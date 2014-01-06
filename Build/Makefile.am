###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS                                             =   -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES                                                =   ../Includes/libARDataTransfer/ARDATATRANSFER_Manager.h \
                                                                ../Includes/libARDataTransfer/ARDATATRANSFER_DataDownloader.h \
                                                                ../Includes/libARDataTransfer/ARDATATRANSFER_MediasDownloader.h \           
                                                                ../Includes/libARDataTransfer/ARDATATRANSFER_Error.h \
                                                                ../Includes/libARDataTransfer/ARDataTransfer.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES                                                =   $(HEADER_FILES)\
                                                                ../Sources/ARDATATRANSFER_Manager.c \
                                                                ../Sources/ARDATATRANSFER_Manager.h \
                                                                ../Sources/ARDATATRANSFER_DataDownloader.c \
                                                                ../Sources/ARDATATRANSFER_DataDownloader.h \
                                                                ../Sources/ARDATATRANSFER_MediasDownloader.c \
                                                                ../Sources/ARDATATRANSFER_MediasDownloader.h \
                                                                ../Sources/ARDATATRANSFER_Error.c \                               
                                                                ../Sources/ARDATATRANSFER_MediasQueue.c \
                                                                ../Sources/ARDATATRANSFER_MediasQueue.h

# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES                                             =   libardatatransfer_dbg.la
libardatatransfer_dbg_ladir                                 =   $(includedir)/libARDataTransfer
libardatatransfer_dbg_la_HEADERS                            =   $(HEADER_FILES)
libardatatransfer_dbg_la_SOURCES                            =   $(SOURCE_FILES)
else
lib_LTLIBRARIES                                             =   libardatatransfer.la
libardatatransfer_ladir                                     =   $(includedir)/libARDataTransfer
libardatatransfer_la_HEADERS                                =   $(HEADER_FILES)
libardatatransfer_la_SOURCES                                =   $(SOURCE_FILES)
endif


check_PROGRAMS                                              =   ../TestBench/Linux/autoTest
# bin_PROGRAMS                                              =   ../TestBench/Linux/autoTest

___TestBench_Linux_autoTest_SOURCES                         =   ../TestBench/Linux/autoTest.c \
                                                                ../TestBench/Linux/test_manager.c

if DEBUG_MODE
___TestBench_Linux_autoTest_LDADD                           =   libardatatransfer_dbg.la \
                                                                -larsal_dbg         \
                                                                -larutils_dbg       \
                                                                -lcurl
else
___TestBench_Linux_autoTest_LDADD                           =   libardatatransfer.la     \
                                                                -larsal             \
                                                                -larutils           \
                                                                -lcurl
endif


CLEAN_FILES                                                 =   libardatatransfer.la       \
                                                                libardatatransfer_dbg.la

EXTRA_DIST                                                  =   bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST                                                  +=  doxygen.ac  \
                                                                doxygen.am  \
                                                                $(DX_CONFIG)
endif

