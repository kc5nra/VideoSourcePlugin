
pushd %_CWD% 
set _CWD=%CD% 
popd 

for /f "delims=" %%a in ('"%GIT_BIN_DIR%\git" describe') do @set VSP_VERSION_BARE=%%a

set VSP_VERSION="VSP_VERSION=L"%VSP_VERSION_BARE%-x64""
msbuild /t:Rebuild /p:DefineConstants=VLC21 /property:Configuration=Release;Platform=x64

rd Release21x64 /s /q
mkdir Release21x64
mkdir Release21x64\VideoSourcePlugin

copy Plugin\x64\Release\VideoSourcePlugin.dll Release21x64\VideoSourcePlugin
copy Wrapper\x64\Release\VideoSourcePluginWrapper.dll Release21x64\

xcopy vlc20x64\* Release21x64\VideoSourcePlugin /s

cd Release21x64
7z a VSP21-%VSP_VERSION_BARE%-x64.7z .
move VSP21-%VSP_VERSION_BARE%-x64.7z ..
cd ..

set VSP_VERSION="VSP_VERSION=L"%VSP_VERSION_BARE%-x86""
msbuild /t:Rebuild /p:DefineConstants=VLC21 /property:Configuration=Release;Platform=win32

rd Release21x86 /s /q
mkdir Release21x86
mkdir Release21x86\VideoSourcePlugin

copy Plugin\win32\Release\VideoSourcePlugin.dll Release21x86\VideoSourcePlugin
copy Wrapper\win32\Release\VideoSourcePluginWrapper.dll Release21x86\

xcopy vlc20x86\* Release21x86\VideoSourcePlugin /s

cd Release21x86
7z a VSP21-%VSP_VERSION_BARE%-x86.7z .
move VSP21-%VSP_VERSION_BARE%-x86.7z ..
cd ..
