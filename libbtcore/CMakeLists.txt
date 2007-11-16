include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libbtcore_SRC 
	util/bitset.cpp
	util/timer.cpp
	util/urlencoder.cpp
	util/sha1hashgen.cpp
	util/sha1hash.cpp
	util/functions.cpp
	util/ptrmap.cpp
	util/array.cpp
	util/error.cpp
	util/file.cpp
	util/fileops.cpp
	util/log.cpp
	util/waitjob.cpp
	util/autorotatelogjob.cpp
	
	bcodec/bdecoder.cpp  
	bcodec/bencoder.cpp  
	bcodec/bnode.cpp  
	bcodec/value.cpp

)

# set(libbtcore_UI )

# kde4_add_ui_files(libbtcore_SRC ${libbtcore_UI})
# kde4_add_kcfg_files(libbtcore_SRC settings.kcfgc)
kde4_add_library(btcore SHARED ${libbtcore_SRC})

# target_link_libraries(btcore ${KDE4_KPARTS_LIBS} -lgmp)
target_link_libraries(btcore ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KPARTS_LIBS} -lsolid -lgmp)
set_target_properties(btcore PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS btcore  DESTINATION ${LIB_INSTALL_DIR} )

