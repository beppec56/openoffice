#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************

PRJ=.

PRJNAME=serf
TARGET=so_serf

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :	serf_version.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_SERF)"=="YES"

all:
	@echo "Using system serf.  Does not have to be built."

.ELSE

# Assemble the full version number from the parts defined in serf_version.mk
LIBSERFVERSION=$(SERF_MAJOR).$(SERF_MINOR).$(SERF_MICRO)

TARFILE_NAME=$(PRJNAME)-$(LIBSERFVERSION)
# This is the SHA1 checksum, not MD5 but tg_ext.mk does not now about this and,
# thankfully, does not care.
TARFILE_MD5=1d45425ca324336ce2f4ae7d7b4cfbc5567c5446

PATCH_FILES=

.IF "$(OS)"=="WNT"

CONFIGURE_DIR=
CONFIGURE_ACTION=
CONFIGURE_FLAGS=

BUILD_DIR=
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -f ../../../../win/Makefile -j$(EXTMAXPROCESS)

.ELIF "$(GUI)" == "OS2"

@all:
	@echo "using system serf. nothing do do."

.ELSE

## CDEFS hold the define used by scons system to build

.IF "$(OS)"=="MACOSX" || "$(OS)"=="FREEBSD" || "$(OS)"=="LINUX"
# Do not link against expat.  It is not necessary (apr-util is already linked against it)
# and does not work (we use a different expat library schema.)
PATCH_FILES+=$(TARFILE_NAME).libs.patch
.ENDIF

.IF "$(OS)"=="LINUX"
# Add -ldl as last library so that the linker has no trouble resolving dependencies.
#PATCH_FILES+=$(TARFILE_NAME).ldl.patch
.ENDIF

# Export ENABLE_SERF_LOGGING=YES to enable serf logging
.IF "$(ENABLE_SERF_LOGGING)" == "YES"
PATCH_FILES+=$(TARFILE_NAME).logging.patch
CDEFS+=-DENABLE_SERF_VERBOSE -DSERF_VERBOSE
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=
CONFIGURE_FLAGS=

.IF "$(OS)"=="LINUX"
.IF "$(SYSTEM_OPENSSL)"=="YES"
#scons will retrive the right openssl installation
CONFOSSL=
.ELSE
OPENSSLINCDIR=external
CONFOSSL=OPENSSL=$(SOLARINCDIR)$/$(OPENSSLINCDIR)
.ENDIF
.ENDIF

# On Linux/Mac we need the content of CDEFS in CFLAGS so that the ssl headers are searched for
.IF "$(OS)"=="MACOSX" || "$(OS)"=="LINUX"
# in a directory that corresponds to the directory that is searched for the ssl library.
CONFIGURE_SCONS='CFLAGS=$(CDEFS)'
.ENDIF

# what can be fed to scons (serf 1.3.8 version)
# $ scons --help
# scons: Reading SConscript files ...
# scons: done reading SConscript files.
# 
# PREFIX: Directory to install under ( /path/to/PREFIX )
#     default: /usr/local
#     actual: /usr/local
# 
# LIBDIR: Directory to install architecture dependent libraries under ( /path/to/LIBDIR )
#     default: $PREFIX/lib
#     actual: /usr/local/lib
# 
# APR: Path to apr-1-config, or to APR's install area ( /path/to/APR )
#     default: /usr
#     actual: /usr
# 
# APU: Path to apu-1-config, or to APR's install area ( /path/to/APU )
#     default: /usr
#     actual: /usr
# 
# OPENSSL: Path to OpenSSL's install area ( /path/to/OPENSSL )
#     default: /usr
#     actual: /usr
# 
# ZLIB: Path to zlib's install area ( /path/to/ZLIB )
#     default: /usr
#     actual: /usr
# 
# GSSAPI: Path to GSSAPI's install area ( /path/to/GSSAPI )
#     default: None
#     actual: None
# 
# DEBUG: Enable debugging info and strict compile warnings (yes|no)
#     default: False
#     actual: False
# 
# APR_STATIC: Enable using a static compiled APR (yes|no)
#     default: False
#     actual: False
# 
# CC: Command name or path of the C compiler
#     default: None
#     actual: gcc
# 
# CFLAGS: Extra flags for the C compiler (space-separated)
#     default: None
#     actual: 
# 
# LIBS: Extra libraries passed to the linker, e.g. "-l<library1> -l<library2>" (space separated)
#     default: None
#     actual: None
# 
# LINKFLAGS: Extra flags for the linker (space-separated)
#     default: None
#     actual: 
# 
# CPPFLAGS: Extra flags for the C preprocessor (space separated)
#     default: None
#     actual: None

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=scons $(CONFIGURE_SCONS) APR=$(SOLARVERSION)/$(INPATH) APU=$(SOLARVERSION)/$(INPATH) $(CONFOSSL)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)

.IF "$(OS)"=="MACOSX"
# Serf names its library only with the major number.
# We are using minor and micro as well.  Fix that here
# by creating a copy with the right name.
SERF_ORIGINAL_LIB=libserf-$(SERF_MAJOR).0.0.0.dylib
SERF_FIXED_LIB=libserf-$(LIBSERFVERSION).0.dylib
INSTALL_ACTION=if [ -f "$(SERF_ORIGINAL_LIB)" -a ! -f "$(SERF_FIXED_LIB)" ]; then cp $(SERF_ORIGINAL_LIB) $(SERF_FIXED_LIB); fi	 
.ENDIF

OUT2INC+=serf*.h
OUT2INC_SUBDIR=serf

.IF "$(OS)"=="MACOSX"
OUT2LIB+=$(SERF_FIXED_LIB)
OUT2LIB+=libserf-1.*dylib
.ELSE
OUT2LIB=libserf-1.so*
.ENDIF

.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
#prepare target
.INCLUDE : target.mk
#build target
.INCLUDE : tg_ext.mk

.ENDIF # .IF "$(OS)"=="WNT"
