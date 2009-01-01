include $(top_srcdir)/gtest.mk

SUBDIRS = . tests

INCLUDES = \
	$(RIVER_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSUMMER_CFLAGS) \
	$(LOUDMOUTH_CFLAGS) \
	$(WARN_CFLAGS) \
	$(DISABLE_DEPRECATED) \
	-include $(CONFIG_HEADER) \
	-Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter

bin_PROGRAMS = river

river_SOURCES = \
	river.c \
	river-config.c \
	river-config.h
	
river_LDADD = \
    $(RIVER_LIBS) \
	$(GLIB_LIBS) \
	$(LIBSUMMER_LIBS)
