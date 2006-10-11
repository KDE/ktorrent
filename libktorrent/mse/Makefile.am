INCLUDES = -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO
libmse_la_LDFLAGS = -lgmp $(all_libraries)
noinst_LTLIBRARIES = libmse.la
noinst_HEADERS = bigint.h rc4encryptor.h streamsocket.h encryptedauthenticate.h \
	encryptedserverauthenticate.h functions.h
libmse_la_SOURCES = bigint.cpp rc4encryptor.cpp streamsocket.cpp \
		encryptedauthenticate.cpp encryptedserverauthenticate.cpp functions.cpp
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
