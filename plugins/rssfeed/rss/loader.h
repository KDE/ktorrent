/*
 * loader.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBRSS_LOADER_H
#define LIBRSS_LOADER_H

#include "global.h"

class KURL;

#include <qobject.h>

namespace KIO
{
	class Job;
}
class KProcess;

namespace RSS
{
	class Document;

	/**
	 * Abstract baseclass for all data retriever classes. Subclass this to add
	 * a new retrieval algorithm which can then be plugged into the RSS loader.
	 * @see Loader, FileRetriever, OutputRetriever
	 */
	class DataRetriever : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * Default constructor.
			 */
			DataRetriever();

			/**
			 * Destructor.
			 */
			virtual ~DataRetriever();

			/**
			 * Retrieve data from the given URL. This method is supposed to get
			 * reimplemented by subclasses. It will be called by the Loader
			 * class in case it needs to retrieve the data.
			 * @see Loader::loadFrom()
			 */
			virtual void retrieveData(const KURL &url) = 0;

			/**
			 * @return An error code which might give a more precise information
			 * about what went wrong in case the 'success' flag returned with
			 * the dataRetrieved() signal was 'false'. Note that the meaning of
			 * the returned integer depends on the actual data retriever.
			 */
			virtual int errorCode() const = 0;

			virtual void abort() = 0;
		signals:
			/**
			 * Emit this signal to tell the Loader class that the retrieval
			 * process was finished.
			 * @param data Should contain the retrieved data and will get
			 * parsed by the Loader class.
			 * @param success Indicates whether there were any problems during
			 * the retrieval process. Pass 'true' to indicate that everything
			 * went seamlessy, 'false' to tell the Loader that something went
			 * wrong and that the data parameter might contain no or invalid
			 * data.
			 */
			void dataRetrieved(const QByteArray &data, bool success);

		private:
			DataRetriever(const DataRetriever &other);
			DataRetriever &operator=(const DataRetriever &other);
	};

	/**
	 * Implements a file retriever, to be used with Loader::loadFrom().
	 * @see DataRetriever, Loader::loadFrom()
	 */
	class FileRetriever : public DataRetriever
	{
		Q_OBJECT
		public:
			/**
			 * Default constructor.
			 */
			FileRetriever();

			/**
			 * Destructor.
			 */
			virtual ~FileRetriever();

			/**
			 * Downloads the file referenced by the given URL and passes it's
			 * contents on to the Loader.
			 * @param url An URL referencing a file which is assumed to
			 * reference valid XML.
			 * @see Loader::loadFrom()
			 */
			virtual void retrieveData(const KURL &url);

			/**
			 * @return The error code for the last process of retrieving data.
			 * The returned numbers correspond directly to the error codes
			 * <a href="http://developer.kde.org/documentation/library/cvs-api/classref/kio/KIO.html#Error">as
			 * defined by KIO</a>.
			 */
			virtual int errorCode() const;

			virtual void abort();

            static void setUseCache(bool enabled);
            
		signals:
			/**
			 * Signals a permanent redirection. The redirection itself is
			 * handled internally, so you don't need to call Loader::loadFrom()
			 * with the new URL. This signal is useful in case you want to
			 * notify the user, or adjust a database entry.
			 * @see Loader::loadFrom()
			 */
			void permanentRedirection(const KURL &url);

        protected slots:
            void slotTimeout();
                
		private slots:
			void slotData(KIO::Job *job, const QByteArray &data);
			void slotResult(KIO::Job *job);
			void slotPermanentRedirection(KIO::Job *job, const KURL &fromUrl,
			                                             const KURL &toUrl);

		private:
            static bool m_useCache;
            
			FileRetriever(const FileRetriever &other);
			FileRetriever &operator=(const FileRetriever &other);

			struct Private;
			Private *d;
	};

	/**
	 * Implements a data retriever which executes a program and stores returned
	 * by the program on stdout. To be used with Loader::loadFrom().
	 * @see DataRetriever, Loader::loadFrom()
	 */
	class OutputRetriever : public DataRetriever
	{
		Q_OBJECT
		public:
			/**
			 * Default constructor.
			 */
			OutputRetriever();

			/**
			 * Destructor.
			 */
			virtual ~OutputRetriever();

			/**
			 * Executes the program referenced by the given URL and retrieves
			 * the data which the program prints to stdout.
			 * @param url An URL which is supposed to reference an executable
			 * file.
			 * @see Loader::loadFrom()
			 */
			virtual void retrieveData(const KURL &url);
			
			/**
			 * @return The error code for the last process of retrieving data.
			 * 0 is returned in case there was no error, otherwise an error
			 * code which depends on the particular program which was run is
			 * returned.
			 */
			virtual int errorCode() const;

			virtual void abort() {}
			
		private slots:
			void slotOutput(KProcess *process, char *data, int length);
			void slotExited(KProcess *process);

		private:
			OutputRetriever(const OutputRetriever &other);
			OutputRetriever &operator=(const OutputRetriever &other);

			struct Private;
			Private *d;
	};

	/**
	 * This class is the preferred way of loading RSS files. Usage is very
	 * straightforward:
	 *
	 * \code
	 * Loader *loader = Loader::create();
	 * connect(loader, SIGNAL(loadingComplete(Loader *, Document, Status)),
	 *         this, SLOT(slotLoadingComplete(Loader *, Document, Status)));
	 * loader->loadFrom("http://www.blah.org/foobar.rdf", new FileRetriever);
	 * \endcode
	 *
	 * This creates a Loader object, connects it's loadingComplete() signal to
	 * your custom slot and then makes it load the file
	 * 'http://www.blah.org/foobar.rdf' using the FileRetriever. You could've
	 * done something like this as well:
	 *
	 * \code
	 * // create the Loader, connect it's signal...
	 * loader->loadFrom("/home/myself/some-script.py", new OutputRetriever);
	 * \endcode
	 *
	 * That'd make the Loader use another algorithm for retrieving the RSS data;
	 * 'OutputRetriever' will make it execute the script
	 * '/home/myself/some-script.py' and assume whatever that script prints to
	 * stdout is RSS markup. This is e.g. handy for conversion scripts, which
	 * download a HTML file and convert it's contents into RSS markup.
	 *
	 * No matter what kind of retrieval algorithm you employ, your
	 * 'slotLoadingComplete' method might look like this:
	 *
	 * \code
	 * void MyClass::slotLoadingComplete(Loader *loader, Document doc, Status status)
	 * {
	 *     // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
	 *     // You don't need to do that anyway since Loader instances delete themselves.
	 *
	 *     if (status != RSS::Success)
	 *         return;
	 *
	 *     QString title = doc.title();
	 *     // do whatever you want with the information.
	 * }
	 * \endcode
	 *
	 * \note You have to create a copy of the passed Document instance in
	 * case you want/need to use it after the slot attached to the
	 * loadingComplete signal goes out of scope. This is e.g. the case if you
	 * intend to call getPixmap() on Document::image()!
	 */
	class Loader : public QObject
	{
		Q_OBJECT
		friend class someClassWhichDoesNotExist;
		public:
			/**
			 * Constructs a Loader instance. This is pretty much what the
			 * default constructor would do, except that it ensures that all
			 * Loader instances have been allocated on the heap (this is
			 * required so that Loader's can delete themselves safely after they
			 * emitted the loadingComplete() signal.).
			 * @return A pointer to a new Loader instance.
			 */
			static Loader *create();

			/**
			 * Convenience method. Does the same as the above method except that
			 * it also does the job of connecting the loadingComplete() signal
			 * to the given slot for you.
			 * @param object A QObject which features the specified slot
			 * @param slot Which slot to connect to.
			 */
			static Loader *create(QObject *object, const char *slot);
			
			/**
			 * Loads the RSS file referenced by the given URL using the
			 * specified retrieval algorithm. Make sure that you connected
			 * to the loadingComplete() signal before calling this method so
			 * that you're guaranteed to get notified when the loading finished.
			 * \note A Loader object cannot load from multiple URLs simultaneously;
			 * consequently, subsequent calls to loadFrom will be discarded
			 * silently, only the first loadFrom request will be executed.
			 * @param url An URL referencing the input file.
			 * @param retriever A subclass of DataRetriever which implements a
			 * specialized retrieval behaviour. Note that the ownership of the
			 * retriever is transferred to the Loader, i.e. the Loader will
			 * delete it when it doesn't need it anymore.
			 * @see DataRetriever, Loader::loadingComplete()
			 */
			void loadFrom(const KURL &url, DataRetriever *retriever);

			/**
			 * Retrieves the error code of the last loading process (if any),
			 * as reported by the employed data retrever.
			 */
			int errorCode() const;
            
            const KURL &discoveredFeedURL() const;

			void abort();
			
		signals:
			/**
			 * This signal gets emitted when the loading process triggered by
			 * calling loadFrom() finished.
			 * @param loader A pointer pointing to the loader object which
			 * emitted this signal; this is handy in case you connect multiple
			 * loaders to a single slot.
			 * @param doc In case status is Success, this parameter holds the
			 * parsed RSS file. In case it's RetrieveError, you should query
			 * loader->errorCode() for the actual error code.
			 * Note that you have to create a copy of the passed Document
			 * instance in case you want/need to use it after the slot attached
			 * to the loadingComplete signal goes out of scope. This is e.g.
			 * the case if you intend to call getPixmap() on Document::image()!
			 * @param status A status byte telling whether there were any problems
			 * while retrieving or parsing the data.
			 * @see Document, Status
			 */
			void loadingComplete(Loader *loader, Document doc, Status status);

		private slots:
			void slotRetrieverDone(const QByteArray &data, bool success);

		private:
			Loader();
			Loader(const Loader &other);
			Loader &operator=(const Loader &other);
			~Loader();
            void discoverFeeds(const QByteArray &data);
            
			struct Private;
			Private *d;
	};
}

#endif // LIBRSS_LOADER_H
// vim: noet:ts=4
