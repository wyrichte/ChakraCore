@echo off
setlocal

::
:: If non-admin actually works for you, set _CHAKRA_SUBMIT_NO_ELEVATE = 1 to avoid elevation
::

if "%_CHAKRA_SUBMIT_NO_ELEVATE%" == "" (
    ::
    :: Run hta elevated with needed environment variables
    ::
    ::  SDXROOT         Used as path for other files
    ::  _BuildBranch    Used to access correct snap queue
    ::  _NTUSER         Used to identify correct snap job
    ::
    runelevated.exe "%~dp0launchhta.cmd" "%~dp0submit.hta" "SDXROOT" "%SDXROOT%" "_BuildBranch" "%_BuildBranch%" "_NTUSER" "%_NTUSER%"
) else (
    call "%~dp0submit.hta"
)