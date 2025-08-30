@echo off
rem build_building.bat - compila BuildingTest.exe (standalone SDL)
setlocal

set BASE=%~dp0..\..
set SRC=%~dp0BuildingTest.cpp
set OUT=BuildingTest.exe

rem Mirror includes/libs used by main build
set INCLUDES=-ICore -Ifonts -Ilibs\SDL3-3.2.18\x86_64-w64-mingw32\include -Ilibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\include -Ilibs\SDL3_image-3.2.4\x86_64-w64-mingw32\include -ICore\libs -ICore\libs\nlohmann
set LIBS=-Llibs\SDL3-3.2.18\x86_64-w64-mingw32\lib -Llibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\lib -Llibs\SDL3_image-3.2.4\x86_64-w64-mingw32\lib -lSDL3 -lSDL3_ttf -lSDL3_image

echo Compiling BuildingTest...
pushd %BASE%
"C:\mingw64\bin\g++.exe" -std=c++17 -O0 -g %INCLUDES% "%SRC%" %LIBS% -o "%OUT%"
if errorlevel 1 (
    echo Compilation failed.
    popd
    endlocal
    exit /b 1
)

echo Build succeeded: %CD%\%OUT%
copy /Y "%OUT%" %~dp0
popd
endlocal
