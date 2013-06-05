
pushd %_CWD% 
set _CWD=%CD% 
popd 

for /f "delims=" %%a in ('"%GIT_BIN_DIR%\git" describe') do @set VSP_VERSION_BARE=%%a

set VSP_VERSION="VSP_VERSION=L"%VSP_VERSION_BARE%-x64""
msbuild /t:Rebuild /property:Configuration=Release;Platform=x64


rd Release64 /s /q
mkdir Release64
mkdir Release64\VideoSourcePlugin

copy Plugin\x64\Release\VideoSourcePlugin.dll Release64\VideoSourcePlugin
copy Wrapper\x64\Release\VideoSourcePluginWrapper.dll Release64\

xcopy vlc64\* Release64\VideoSourcePlugin /s

cd Release64
zip -r VSP-%VSP_VERSION_BARE%-x64.zip .
move VSP-%VSP_VERSION_BARE%-x64.zip ..
cd ..
