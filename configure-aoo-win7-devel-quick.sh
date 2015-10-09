# linea di comando di Davide:
#./configure --with-directx-home="C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" --with-ant-home="/cygdrive/c/apache-ant-1.9.4/" --with-dmake-url="http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2" --with-epm-url="http://www.msweet.org/files/project2/epm-3.7-source.tar.gz" --enable-pch --disable-atl --disable-activex --without-junit

./configure --with-directx-home="C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" --with-ant-home="/cygdrive/c/apache-ant-1.9.4/" --with-dmake-url="http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2" --with-epm-url="http://www.msweet.org/files/project2/epm-3.7-source.tar.gz" --enable-pch --with-atl-include-dir="/cygdrive/c/WinDDK/7600.16385.1/inc/atl71" --with-atl-lib-dir="/cygdrive/c/WinDDK/7600.16385.1/lib/ATL/i386" --with-mfc-include-dir="/cygdrive/c/WinDDK/7600.16385.1/inc/mfc42" --with-mfc-lib-dir="/cygdrive/c/WinDDK/7600.16385.1/lib/Mfc/i386" --disable-activex --without-junit --with-mozilla-build="/cygdrive/c/mozilla-build" --enable-category-b --enable-bundled-dictionaries --with-lang="en-US it" --with-build-version="$(date +"%Y-%m-%d %H:%M:%S (%a, %d %b %Y)") - Rev. $(git branch |grep "*" | sed 's/* //g') $(echo git_$(git log -n1 --format=%h))"

#./configure --with-directx-home="C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" --with-ant-home="/cygdrive/c/apache-ant-1.9.4/"  --with-mozilla-build=C\:/mozilla-build/  --with-dmake-url="http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2" --with-epm-url="http://www.msweet.org/files/project2/epm-3.7-source.tar.gz" --enable-pch --disable-atl --disable-activex --without-junit --enable-dbgutil --with-build-version="$(date +"%Y-%m-%d %H:%M:%S (%a, %d %b %Y)") - Rev. $(git branch |grep "*" | sed 's/* //g') $(echo git_$(git log -n1 --format=%h))"
#./configure --with-directx-home="C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" --with-ant-home="/cygdrive/c/apache-ant-1.9.4/" --with-nsis-path=C\:/Program\ Files\ \(x86\)/NSIS/Unicode/ --with-mozilla-build=C\:/mozilla-build/    --with-dmake-url="http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2" --with-epm-url="http://www.msweet.org/files/project2/epm-3.7-source.tar.gz" --enable-pch --disable-atl --disable-activex --without-junit --enable-dbgutil
#./configure --with-frame-home="$SDK_PATH" --with-psdk-home="$SDK_PATH" --with-midl-path="$SDK_PATH" --with-directx-home="C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" --with-ant-home="/cygdrive/c/apache-ant-1.9.4/" --with-dmake-url="http://dmake.apache-extras.org.codespot.com/files/dmake-4.12.tar.bz2" --with-epm-url="http://www.msweet.org/files/project2/epm-3.7-source.tar.gz" --enable-pch --disable-atl --disable-activex --without-junit
