::
:: This script launches a HTA with environment variable parameters
::
::      Usage: launchhta.cmd  HTA_PATH  [NAME VALUE] [NAME VALUE]...
::
@echo off
setlocal

set THIS_HTA=%1
shift

:NextArgument
if not "%1" == "" (
    set %1=%2
    shift
    shift
    goto NextArgument
)

start %THIS_HTA%
