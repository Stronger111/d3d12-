@ECHO off
REM Build Everything

ECHO "Building everything..."

PUSHD engine
CALL build.bat
POPD

IF %ERRORLEVEL% NEQ 0 (echo error:%ERRORLEVEL% && exit)

PUSHD testbed
CALL build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo error:%ERRORLEVEL% && exit)

ECHO "All assemblies built successfully."