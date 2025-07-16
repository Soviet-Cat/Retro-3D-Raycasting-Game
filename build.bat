@echo off
setlocal

if "%1"=="" (
    set ALLOC_CONSOLE=OFF
) else (
    set ALLOC_CONSOLE=%1
)

if "%2"=="" (
    set BUILD_TYPE=Release
) else (
    set BUILD_TYPE=%2
)

echo Configuring with ALLOC_CONSOLE=%ALLOC_CONSOLE% BUILD_TYPE=%BUILD_TYPE%...
call cmake -S . -B build -G Ninja -D ALLOC_CONSOLE=%ALLOC_CONSOLE% -D CMAKE_BUILD_TYPE=%BUILD_TYPE%
echo Finished configuring...
echo Building...
call cmake --build build --config Debug
echo Finished building...