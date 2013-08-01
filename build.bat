@echo off

pushd %_CWD% 
set _CWD=%CD% 
popd 

for /f "delims=" %%a in ('"%GIT_BIN_DIR%\git" describe') do @set VSP_VERSION_BARE=%%a

if "%1"=="" goto failure


:loop
if "%1"=="" goto end

set VLC=%1
set VLC_VERSION="%VLC%"
set VSP=%VSP_VERSION_BARE%-%VLC%
set VSP_VERSION="VSP_VERSION=L"%VSP%-x64""
msbuild /t:Rebuild /p:VLC2X=true /property:Configuration=Release;Platform=x64

rd Release-%VLC%-x64 /s /q
mkdir Release-%VLC%-x64
mkdir Release-%VLC%-x64\VideoSourcePlugin

copy Plugin\x64\Release\VideoSourcePlugin.dll Release-%VLC%-x64\VideoSourcePlugin
copy Wrapper\x64\Release\VideoSourcePluginWrapper.dll Release-%VLC%-x64\

xcopy %VLC%-x64\* Release-%VLC%-x64\VideoSourcePlugin /s

cd Release-%VLC%-x64
7z a VSP-%VSP%-x64.7z .
move VSP-%VSP%-x64.7z ..
cd ..

set VSP_VERSION="VSP_VERSION=L"%VSP%-x86""
msbuild /t:Rebuild /p:VLC2X=true /property:Configuration=Release;Platform=win32

rd Release-%VLC%-x86 /s /q
mkdir Release-%VLC%-x86
mkdir Release-%VLC%-x86\VideoSourcePlugin

copy Plugin\win32\Release\VideoSourcePlugin.dll Release-%VLC%-x86\VideoSourcePlugin
copy Wrapper\win32\Release\VideoSourcePluginWrapper.dll Release-%VLC%-x86\

xcopy %VLC%-x86\* Release-%VLC%-x86\VideoSourcePlugin /s

cd Release-%VLC%-x86
7z a VSP-%VSP%-x86.7z .
move VSP-%VSP%-x86.7z ..
cd ..

shift
goto loop

:failure
echo.
echo usage^: build ^<vlc_version^>
echo 	^<vlc_version^> vlc version to build against [VLC20, VLC2X]
echo.
:end