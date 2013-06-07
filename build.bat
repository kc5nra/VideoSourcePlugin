
pushd %_CWD% 
set _CWD=%CD% 
popd 

for /f "delims=" %%a in ('"%GIT_BIN_DIR%\git" describe') do @set VSP_VERSION_BARE=%%a

set VSP_VERSION="VSP_VERSION=L"%VSP_VERSION_BARE%-x64""
msbuild /t:Rebuild /property:Configuration=Release;Platform=x64

rd Release20x64 /s /q
mkdir Release20x64
mkdir Release20x64\VideoSourcePlugin

copy Plugin\x64\Release\VideoSourcePlugin.dll Release20x64\VideoSourcePlugin
copy Wrapper\x64\Release\VideoSourcePluginWrapper.dll Release20x64\

xcopy vlc20x64\* Release20x64\VideoSourcePlugin /s

cd Release20x64
7z a VSP-%VSP_VERSION_BARE%-x64.7z .
move VSP-%VSP_VERSION_BARE%-x64.7z ..
cd ..

set VSP_VERSION="VSP_VERSION=L"%VSP_VERSION_BARE%-x86""
msbuild /t:Rebuild /property:Configuration=Release;Platform=win32

rd Release20x86 /s /q
mkdir Release20x86
mkdir Release20x86\VideoSourcePlugin

copy Plugin\win32\Release\VideoSourcePlugin.dll Release20x86\VideoSourcePlugin
copy Wrapper\win32\Release\VideoSourcePluginWrapper.dll Release20x86\

xcopy vlc20x86\* Release20x86\VideoSourcePlugin /s

cd Release20x86
7z a VSP-%VSP_VERSION_BARE%-x86.7z .
move VSP-%VSP_VERSION_BARE%-x86.7z ..
cd ..
