#
# Copyright (C) 2010 by Argonne National Laboratory.
#     See COPYRIGHT in top-level directory.
#

libosp_la_SOURCES += $(top_srcdir)/src/ospd/mpid/mpid_param.c \
    $(top_srcdir)/src/ospd/mpid/mpid_initialize.c \
	$(top_srcdir)/src/ospd/mpid/mpid_finalize.c \
	$(top_srcdir)/src/ospd/mpid/mpid_malloc.c \
	$(top_srcdir)/src/ospd/mpid/mpid_free.c \
	$(top_srcdir)/src/ospd/mpid/mpid_flush.c \
	$(top_srcdir)/src/ospd/mpid/mpid_flush_all.c \
	$(top_srcdir)/src/ospd/mpid/mpid_put.c \
	$(top_srcdir)/src/ospd/mpid/mpid_get.c \
	$(top_srcdir)/src/ospd/mpid/mpid_collectives.c \
	$(top_srcdir)/src/ospd/mpid/mpid_misc.c 
