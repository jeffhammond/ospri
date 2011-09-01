#
# Copyright (C) 2010 by Argonne National Laboratory.
#     See COPYRIGHT in top-level directory.
#

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_builddir)/src/include

lib_LTLIBRARIES = libosp.la
libosp_la_SOURCES =
libosp_la_LDFLAGS = -version-info $(libosp_abi_version)

include_HEADERS = $(top_builddir)/src/include/osp.h

include src/util/Makefile.mk
#include src/apis/ospri/Makefile.mk

if osp_device_dcmfd
include src/devices/dcmfd/Makefile.mk
endif

#if osp_device_pthreadd
#include src/ospd/pthreadd/Makefile.mk
#endif

#if osp_device_mpid
#include src/ospd/mpid/Makefile.mk
#endif

#if osp_device_geminid
#include src/ospd/geminid/Makefile.mk
#endif

#if osp_device_lapid
#include src/ospd/lapid/Makefile.mk
#endif

#if osp_device_ibd
#include src/ospd/ibd/Makefile.mk
#endif

docs:
	if test "@DOXYGEN@" != "" ; then @DOXYGEN@ osp-doxygen.cfg ; fi
