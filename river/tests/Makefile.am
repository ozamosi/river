include $(top_srcdir)/gtest.mk

noinst_PROGRAMS = $(TEST_PROGS)

INCLUDES = \
	$(RIVER_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LOUDMOUTH_CFLAGS) \
	-I$(top_srcdir)/libsummer \
	-include $(CONFIG_HEADER)

AM_LDFLAGS = \
	$(RIVER_LIBS) \
	$(GLIB_LIBS) \
	$(LOUDMOUTH_LIBS)

check-local: test

#TEST_PROGS += \
#	test_config

#test_config_SOURCES = test-config.c
