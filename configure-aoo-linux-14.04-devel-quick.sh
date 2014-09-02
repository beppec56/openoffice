#!/bin/bash

#solo all'inizio o dopo un update:
#autoconf

#--with-epm-url="http://www.epmhome.org/software.php?VERSION=4.2&FILE=epm/3.7/epm-3.7-source.tar.bz2"  \


#./configure --with-epm-url=http://ftp.easysw.com/pub/epm/3.7/epm-3.7-source.tar.gz \
#--with-epm-url=http://epm.sourcearchive.com/downloads/3.7-1/epm_3.7.orig.tar.gz \
#--with-build-version="$(date +"%Y-%m-%d %H:%M:%S (%a, %d %b %Y)") - Rev. $(echo $(git svn info) | sed -e 's/^.*Last Changed Rev: //g' -e 's/ .*//g')" \
#./configure \
#--with-epm-url=http://www.msweet.org/files/project2/epm-3.7-source.tar.gz \
#--with-dmake-url=http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2 \
#--enable-category-b \
#--enable-crashdump \
#--with-package-format="deb" \
#--with-lang='en-US' \
#--enable-opengl  \
#--enable-dbus  \
#--enable-gstreamer \
#--enable-pdfimport \
#--enable-hunspell \
#--enable-hyphen \
#--enable-bundled-dictionaries \
#\
#--with-mingwin=i586-mingw32msvc-c++ \
#\
#--enable-verbose \
#--with-build-version="$(date +"%Y-%m-%d %H:%M:%S (%a, %d %b %Y)") - Rev.$(echo $(git svn info) | sed -e 's/^.*Last Changed Rev: //g' -e 's/ .*//g') $(echo git_$(git log -n1 --format=%h))" \

#            --enable-symbols \
#            --enable-dbgutil \

./configure \
--with-epm-url=http://www.msweet.org/files/project2/epm-3.7-source.tar.gz \
--with-dmake-url=http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2 \
--enable-category-b \
--with-package-format="deb" \
--without-junit \
--without-stlport \
--enable-bundled-dictionaries \
--with-lang='en-US' \
\
--enable-verbose \
--with-build-version="$(date +"%Y-%m-%d %H:%M:%S (%a, %d %b %Y)") - Rev. $(git branch |grep "*" | sed 's/* //g') $(echo git_$(git log -n1 --format=%h))" \
--with-mingwin=i586-mingw32msvc-c++ \


exit


--with-lang='en-US' \
--enable-opengl  \
--enable-dbus  \
--enable-gstreamer \
--enable-pdfimport \
--enable-hunspell \
--enable-hyphen \
--enable-bundled-dictionaries \
\
--with-mingwin=i586-mingw32msvc-c++ \
\
--enable-verbose \
--with-build-version="$(date +"%Y-%m-%d %H:%M:%S (%a, %d %b %Y)") - Rev.$(echo $(git svn info) | sed -e 's/^.*Last Changed Rev: //g' -e 's/ .*//g') $(echo git_$(git log -n1 --format=%h))" \

exit
