@echo off
echo - SET DEV MACHINE for Projection -
echo.

IF "%_NTTREE%" EQU "" (
  echo _NTTREE not defined - error
  exit /b 1
)

SET WIN_JSHOST_METADATA_BASE_PATH=%_NTTREE%\projection\winrt
echo WIN_JSHOST_METADATA_BASE_PATH = %WIN_JSHOST_METADATA_BASE_PATH%
echo.
