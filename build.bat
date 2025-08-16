@echo off
REM build.bat - Compila Space Invaders y actualiza versión automáticamente

setlocal enabledelayedexpansion

REM === CONFIGURACIÓN ===
set AUTHOR=UnityDo
set VERSION_FILE=version.txt
set BUILD_INFO=Core\build_info.h
set DATE=%DATE:~0,10%

REM === LEER Y ACTUALIZAR VERSIÓN ===
set /p VERSION=<%VERSION_FILE%
for /f "tokens=1,2,3 delims=." %%a in ("!VERSION!") do (
    set /a PATCH=%%c+1
    set MINOR=%%b
    set MAJOR=%%a
)
set NEW_VERSION=!MAJOR!.!MINOR!.!PATCH!
echo !NEW_VERSION! > %VERSION_FILE%

REM === GENERAR build_info.h ===
echo // Este archivo se genera automáticamente en cada build > %BUILD_INFO%
echo #pragma once >> %BUILD_INFO%
echo #define BUILD_VERSION "!NEW_VERSION!" >> %BUILD_INFO%
echo #define BUILD_DATE "%DATE%" >> %BUILD_INFO%
echo #define BUILD_AUTHOR "%AUTHOR%" >> %BUILD_INFO%

REM === COMPILAR ===
set SRC=Core\main.cpp Core\Game.cpp Core\Player.cpp Core\Enemy.cpp Core\EnemyManager.cpp Core\EnemyFactory.cpp Core\Bullet.cpp Core\Renderer.cpp Core\InputManager.cpp Core\CollisionManager.cpp Core\ParticleSystem.cpp Core\TextRenderer.cpp Core\AudioManager.cpp Core\AudioManagerMiniaudio.cpp
set OUT=SpaceInvaders.exe
set INCLUDES=-ICore -Ifonts -Ilibs\SDL3-3.2.18\x86_64-w64-mingw32\include -Ilibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\include -ICore\libs -ICore\libs\nlohmann
set LIBS=-Llibs\SDL3-3.2.18\x86_64-w64-mingw32\lib -Llibs\SDL3_ttf-devel-3.2.2-mingw\x86_64-w64-mingw32\lib -lSDL3 -lSDL3_ttf

REM Soporta parámetro /fast para compilación de desarrollo (-O0 -g)
set BUILD_FLAGS=-O2
if "%1"=="/fast" set BUILD_FLAGS=-O0 -g

REM Compilar por fichero y medir tiempos (usa PowerShell para temporización y estimador)
REM Creamos carpeta de objetos
if not exist build mkdir build

REM Exportar variables para PowerShell
setlocal enabledelayedexpansion
set OUT_ENV=%OUT%
set INCLUDES_ENV=%INCLUDES%
set LIBS_ENV=%LIBS%

REM Crear un script PowerShell temporal y ejecutarlo para evitar problemas de parsing en cmd
set PS_SCRIPT=tools\space_build.ps1
if not exist "%PS_SCRIPT%" (
        echo ERROR: No se encuentra %PS_SCRIPT%. Asegurate de que exista tools\space_build.ps1
        goto :end
)

REM Exportar variables para el script
set SRC_ENV=%SRC%
set INCLUDES_ENV=%INCLUDES%
set LIBS_ENV=%LIBS%
set OUT_ENV=%OUT%

powershell -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%"

if %ERRORLEVEL%==0 (
        echo Build exitoso. Version: !NEW_VERSION! Fecha: %DATE%
) else (
        echo Error en la compilación. Revisa la salida anterior.
)

endlocal
