#
# Copyright (C) 2010 by Argonne National Laboratory.
#     See COPYRIGHT in top-level directory.
#

libosp_la_SOURCES += $(top_srcdir)/src/apis/ospri/osp_initialize.c \
                     $(top_srcdir)/src/apis/ospri/osp_finalize.c \
                     $(top_srcdir)/src/apis/ospri/osp_malloc.c \
                     $(top_srcdir)/src/apis/ospri/osp_free.c \
                     $(top_srcdir)/src/apis/ospri/osp_flush.c \
                     $(top_srcdir)/src/apis/ospri/osp_flush_group.c \
                     $(top_srcdir)/src/apis/ospri/osp_put.c \
                     $(top_srcdir)/src/apis/ospri/osp_puts.c \
                     $(top_srcdir)/src/apis/ospri/osp_putv.c \
                     $(top_srcdir)/src/apis/ospri/osp_get.c \
                     $(top_srcdir)/src/apis/ospri/osp_gets.c \
                     $(top_srcdir)/src/apis/ospri/osp_getv.c \
                     $(top_srcdir)/src/apis/ospri/osp_putacc.c \
                     $(top_srcdir)/src/apis/ospri/osp_putaccs.c \
                     $(top_srcdir)/src/apis/ospri/osp_putaccv.c \
                     $(top_srcdir)/src/apis/ospri/osp_putmodv.c \
                     $(top_srcdir)/src/apis/ospri/osp_rmw.c \
                     $(top_srcdir)/src/apis/ospri/osp_wait.c \
                     $(top_srcdir)/src/apis/ospri/osp_collectives.c \
                     $(top_srcdir)/src/apis/ospri/osp_misc.c \
                     $(top_srcdir)/src/apis/ospri/osp_counter.c \
                     $(top_srcdir)/src/apis/ospri/osp_mutex.c \
                     $(top_srcdir)/src/apis/ospri/osp_handle.c
