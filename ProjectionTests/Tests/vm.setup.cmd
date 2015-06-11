@echo off
echo - SET VM MACHINE for Projection -
echo.

IF "%_NTTREE%" NEQ "" (
  echo _NTTREE defined - error
  exit /b 1
)

SET WIN_JSHOST_METADATA_BASE_PATH=C:\ProjectionTests\JsHost
echo WIN_JSHOST_METADATA_BASE_PATH = %WIN_JSHOST_METADATA_BASE_PATH%
echo.
