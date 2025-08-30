@echo off
rem build_simple.bat - Compila SpriteTestSimple usando exactamente la misma configuración que el juego principal

setlocal enableextensions

set GPP=g++
set BASE=%~dp0..\..
set OUT_DIR=%BASE%\

rem Usar exactamente las mismas configuraciones que build.bat
set SRC="%~dp0SpriteTestSimple.cpp" "%BASE%\Core\Renderer.cpp" "%BASE%\Core\SpriteSheet.cpp" "%BASE%\Core\TextRenderer.cpp"
set INCLUDES=-ICore -Ifonts -Ilibs\SDL3-3.2.18\x86_64-w64-mingw32\include -Ilibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\include -Ilibs\SDL3_image-3.2.4\x86_64-w64-mingw32\include -ICore\libs -ICore\libs\nlohmann
set LIBS=-Llibs\SDL3-3.2.18\x86_64-w64-mingw32\lib -Llibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\lib -Llibs\SDL3_image-3.2.4\x86_64-w64-mingw32\lib -lSDL3 -lSDL3_ttf -lSDL3_image

echo Compiling SpriteTestSimple with same config as main game...
pushd "%BASE%"

%GPP% -std=c++17 -O2 %INCLUDES% %SRC% %LIBS% -o "%OUT_DIR%SpriteTestSimple.exe"

if errorlevel 1 (
    echo Compilation failed.
    popd
    goto :EOF
)

echo Compilation successful: SpriteTestSimple.exe
echo Use arrow keys to change sprite, Q/ESC to quit

popd
endlocal
