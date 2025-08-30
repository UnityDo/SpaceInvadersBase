@echo off
rem build_spritetest.bat
rem Compila el SpriteTest usando g++ (MinGW) y las librerías SDL3/SDL3_image.

setlocal enableextensions enabledelayedexpansion

set GPP=g++
set OUT_DIR=%~dp0

rem Rutas basadas en la ubicación del script. Ajusta si tu estructura difiere.
rem Normalize base path to absolute (script folder -> two levels up)
set BASE=%~dp0..\..
pushd "%BASE%" >nul 2>&1
if errorlevel 1 (
	echo Failed to resolve base path: %BASE%
	goto :EOF
)
set BASE=%CD%
popd >nul 2>&1

rem Place the final executable in the project root (same folder as SpaceInvaders.exe)
set OUT_DIR=%BASE%\

rem Source files to compile for the SpriteTest PoC (include Renderer.cpp because SpriteTest uses Renderer)
set SRCS="%BASE%\Core\SpriteSheet.cpp" "%BASE%\Core\Renderer.cpp" "%~dp0SpriteTest.cpp"

set SDL3_INC=%BASE%\libs\SDL3-3.2.18\x86_64-w64-mingw32\include
set SDL3_LIB=%BASE%\libs\SDL3-3.2.18\x86_64-w64-mingw32\lib

set SDLIMG_INC=%BASE%\libs\SDL3_image-3.2.4\x86_64-w64-mingw32\include
set SDLIMG_LIB=%BASE%\libs\SDL3_image-3.2.4\x86_64-w64-mingw32\lib
rem Si tu instalación está en otro sitio, exporta SDL3_ROOT y/o SDL3_IMAGE_ROOT antes de llamar al script
if defined SDL3_ROOT (
	set SDL3_INC=%SDL3_ROOT%\include
	set SDL3_LIB=%SDL3_ROOT%\lib
)
if defined SDL3_IMAGE_ROOT (
	set SDLIMG_INC=%SDL3_IMAGE_ROOT%\include
	set SDLIMG_LIB=%SDL3_IMAGE_ROOT%\lib
)

echo Using g++: %GPP%
echo Output folder: %OUT_DIR%
echo SDL3 include: %SDL3_INC%
echo SDL3_image include: %SDLIMG_INC%

if not exist "%SDL3_INC%" (
	echo ERROR: SDL3 include folder not found: "%SDL3_INC%"
	echo Please set SDL3_ROOT or fix the repo libs location.
	goto :EOF
)
if not exist "%SDLIMG_INC%" (
	echo ERROR: SDL3_image include folder not found: "%SDLIMG_INC%"
	echo Please set SDL3_IMAGE_ROOT or fix the repo libs location.
	goto :EOF
)

%GPP% --version >nul 2>&1
if errorlevel 1 (
	echo g++ not found on PATH. Please install MinGW and make sure g++ is on PATH.
	goto :EOF
)

rem Añadir subcarpetas SDL3/ y SDL3_image/ donde están los headers en este repo
rem Add both the base include and the SDL3 subfolder so includes like
rem <SDL.h> or <SDL3/SDL.h> both work depending on code.

rem Use same include/lib layout as main build.bat so paths are consistent
set INCLUDES=-ICore -Ifonts -I%SDL3_INC% -Ilibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\include -I%SDLIMG_INC% -ICore\libs -ICore\libs\nlohmann
set LIBS=-L%SDL3_LIB% -Llibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\lib -L%SDLIMG_LIB% -lSDL3 -lSDL3_image -lgdi32

echo Compiling SpriteTest...
echo Compiling SpriteTest...
echo g++ -std=c++17 -O2 %INCLUDES% %SRCS% %LIBS% -o "%OUT_DIR%SpriteTest.exe"
%GPP% -std=c++17 -O2 %INCLUDES% %SRCS% %LIBS% -o "%OUT_DIR%SpriteTest.exe"

if errorlevel 1 (
	echo Compilation failed.
	goto :EOF
)

echo Compilation succeeded.

rem Copy required runtime DLLs to the project root (same behavior as build.bat)
set "SDL3_BIN=%BASE%\libs\SDL3-3.2.18\x86_64-w64-mingw32\bin"
set "SDL3IMG_BIN=%BASE%\libs\SDL3_image-3.2.4\x86_64-w64-mingw32\bin"
if exist "%SDL3_BIN%\SDL3.dll" (
	copy /Y "%SDL3_BIN%\SDL3.dll" "%OUT_DIR%" >nul && echo Copied SDL3.dll to %OUT_DIR%
) else if exist "%SDL3_LIB%\SDL3.dll" (
	copy /Y "%SDL3_LIB%\SDL3.dll" "%OUT_DIR%" >nul && echo Copied SDL3.dll to %OUT_DIR%
) else (
	echo Warning: SDL3.dll not found in expected bin/lib paths.
)

if exist "%SDL3IMG_BIN%\SDL3_image.dll" (
	copy /Y "%SDL3IMG_BIN%\SDL3_image.dll" "%OUT_DIR%" >nul && echo Copied SDL3_image.dll to %OUT_DIR%
) else if exist "%SDLIMG_LIB%\SDL3_image.dll" (
	copy /Y "%SDLIMG_LIB%\SDL3_image.dll" "%OUT_DIR%" >nul && echo Copied SDL3_image.dll to %OUT_DIR%
) else (
	echo Warning: SDL3_image.dll not found in expected bin/lib paths.
)

echo Done. SpriteTest.exe written to: "%OUT_DIR%SpriteTest.exe"
echo Run it from the project root (double-click or from cmd): "%OUT_DIR%SpriteTest.exe" [path_to_sheet] [scale]

endlocal
