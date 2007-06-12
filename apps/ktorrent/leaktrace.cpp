#ifdef KT_LEAKTRACE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <util/constants.h>

/*
 * prime number for the address lookup hash table.
 * if you have _really_ many memory allocations, use a
 * higher value, like 343051 for instance.
 */
#define SOME_PRIME 35323
#define ADDR_HASH(addr) ((unsigned long) addr % SOME_PRIME)

using namespace bt;

struct MemAlloc
{
	void* ptr;
	size_t size;
	void* alloc_addr; // addr of function which did the allocation
	MemAlloc* left;
	MemAlloc* right;
};

struct MemAllocTree
{
	Uint32 num_buckets;
	Uint32 count;
	Uint64 bytes;
	pthread_mutex_t mutex;
	MemAlloc* buckets[SOME_PRIME];
};

static MemAllocTree mtree = {0,0,0,PTHREAD_MUTEX_INITIALIZER};
static bool print_status_done = false;

static void InsertIntoTree(MemAlloc* cnode,MemAlloc* target)
{
	if (target->ptr < cnode->ptr)
	{
		if (!cnode->left)
			cnode->left = target;
		else
			InsertIntoTree(cnode->left,target);
	}
	else
	{
		if (!cnode->right)
			cnode->right = target;
		else
			InsertIntoTree(cnode->right,target);
	}
}

static void PrintTree(MemAlloc* ma)
{
	if (!ma)
		return;
	
	printf("\t0x%p 0x%p %i bytes\n",ma->alloc_addr,ma->ptr,ma->size);
	PrintTree(ma->left);
	PrintTree(ma->right);
}

static void PrintTreeToFile(FILE* fptr,MemAlloc* ma)
{
	if (!ma)
		return;
	
	fprintf(fptr, "L %10p   %9ld  # %p\n",ma->alloc_addr,ma->size,ma->ptr);
	PrintTreeToFile(fptr,ma->left);
	PrintTreeToFile(fptr,ma->right);
}

static void WriteLeakReport()
{
	FILE* report = fopen("leak.out","wt");
	if (!report)
	{
		printf("Cannot write report !!!!!!!!\n");
		return;
	}
		
	
	for (Uint32 b = 0;b < SOME_PRIME;b++)
		PrintTreeToFile(report,mtree.buckets[b]);
	fclose(report);
}

static void PrintStatus()
{
	if (mtree.count == 0)
	{
		printf("No memory leaks detected !!!\n");
	}
	else
	{
		printf("LeakTrace results : \n");
		for (Uint32 b = 0;b < SOME_PRIME;b++)
			PrintTree(mtree.buckets[b]);
		printf("====================\n");
		printf("Total : %i leaks, %li bytes leaked\n",mtree.count,mtree.bytes);
	}
	WriteLeakReport();
	print_status_done = true;
}



static void RegisterAlloc(void* ptr,Uint32 size)
{
	MemAlloc* ma = (MemAlloc*)malloc(sizeof(MemAlloc));
	ma->left = ma->right = 0;
	ma->size = size;
	ma->alloc_addr = __builtin_return_address(1);
	ma->ptr = ptr;
	if (mtree.num_buckets == 0)
	{
		// init buckets
		for (Uint32 b = 0;b < SOME_PRIME;b++)
			mtree.buckets[b] = 0;
		mtree.num_buckets = SOME_PRIME;
		atexit(PrintStatus);
	}
	
	// hash the address
	Uint32 b = ADDR_HASH(ptr);
	if (!mtree.buckets[b])
	{
		mtree.count++;
		mtree.bytes += size;
		mtree.buckets[b] = ma;
	}
	else
	{
		// walk the tree and insert it
		InsertIntoTree(mtree.buckets[b],ma);
		mtree.count++;
		mtree.bytes += size;
	}
}

static void DeregisterAlloc(void* ptr)
{
	if (print_status_done)
		printf("PrintStatus already happened !!!!!!!!!!\n");
	Uint32 b = ADDR_HASH(ptr);
	
	MemAlloc* p = mtree.buckets[b];
	MemAlloc* prev = 0;
	while (p && p->ptr != ptr)
	{
		prev = p;
		if (ptr < p->ptr)
			p = p->left;
		else if (ptr > p->ptr)
			p = p->right;
	}
	
	if (!p)
		return;
	
	if (!prev)
	{
		// it's the root
		mtree.count--;
		mtree.bytes -= p->size;
		free(p);
		mtree.buckets[b] = 0;
	}
	else
	{
		// first update some additional info
		mtree.count--;
		mtree.bytes -= p->size;
				
		if (!p->left && !p->right)
		{
			// no children so just free p
			if (prev->left == p)
			{
				free(prev->left);
				prev->left = 0;
			}
			else
			{
				free(prev->right);
				prev->right = 0;
			}
		}
		else if (p->left && !p->right)
		{
			// one child of p is zero, so just attach 
			// the child of p to prev
			if (prev->left == p)
				prev->left = p->left;
			else
				prev->right = p->left;
			free(p);
		}
		else if (!p->left && p->right)
		{
			// one child of p is zero, so just attach 
			// the child of p to prev
			if (prev->left == p)
				prev->left = p->right;
			else
				prev->right = p->right;
			free(p);
		}
		else
		{
			// both children exist
			if (prev->left == p)
			{
				// attach the left child of p
				prev->left = p->left;
				InsertIntoTree(prev,p->right);
			}
			else
			{
				// attach the right child of p
				prev->right = p->right;
				InsertIntoTree(prev,p->left);
			}
			
			free(p);
		}
	}
		
}


void* operator new(size_t size) 
{
	void* ptr = malloc(size);
	if (!ptr)
	{
		printf("PANIC : memory allocation failed !\n");
		exit(-1);
	}
	
	
	pthread_mutex_lock(&mtree.mutex);
	RegisterAlloc(ptr,size);
	pthread_mutex_unlock(&mtree.mutex);
	return ptr;
}


void* operator new[] (size_t size) 
{
	void* ptr = malloc(size);
	if (!ptr)
	{
		printf("PANIC : memory allocation failed !\n");
		exit(-1);
	}
	
	pthread_mutex_lock(&mtree.mutex);
	RegisterAlloc(ptr,size);
	pthread_mutex_unlock(&mtree.mutex);
	return ptr;
}


void operator delete (void *ptr) 
{
	if (!ptr)
		return;
	
	
	pthread_mutex_lock(&mtree.mutex);
	DeregisterAlloc(ptr);
	pthread_mutex_unlock(&mtree.mutex);
	free(ptr);
}


void operator delete[] (void *ptr) 
{
	if (!ptr)
		return;
	
	pthread_mutex_lock(&mtree.mutex);
	DeregisterAlloc(ptr);
	pthread_mutex_unlock(&mtree.mutex);
	free(ptr);
}

#endif // KT_LEAKTRACE
