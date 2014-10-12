# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgvalstft.la

# sources used to compile this plug-in
libgvalstft_la_SOURCES = gval_stft.c gval_stft.h gval_utils.c gval_utils.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgvalstft_la_CFLAGS = $(GST_CFLAGS) $(GSL_CFLAGS)
libgvalstft_la_LIBADD = $(GST_LIBS) $(GSL_LIBS)
libgvalstft_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgvalstft_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gval_stft.h gval_utils.h
