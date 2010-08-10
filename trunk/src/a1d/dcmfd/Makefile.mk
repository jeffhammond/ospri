#
# Copyright (C) 2010 by Argonne National Laboratory.
#     See COPYRIGHT in top-level directory.
#

liba1_la_SOURCES += $(top_srcdir)/src/a1d/dcmfd/dcmfd_param.c \
        $(top_srcdir)/src/a1d/dcmfd/dcmfd_initialize.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_finalize.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_malloc.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_free.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_flush.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_flush_all.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_put.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_puts.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_get.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_gets.c \
        $(top_srcdir)/src/a1d/dcmfd/dcmfd_putacc.c \
        $(top_srcdir)/src/a1d/dcmfd/dcmfd_putaccs.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_collectives.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_misc.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_util.c \
	$(top_srcdir)/src/a1d/dcmfd/dcmfd_requestpool.c 
