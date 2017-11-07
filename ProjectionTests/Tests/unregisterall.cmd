@echo off

if "%_TestsDirectory%" EQU "" (set _TestsDirectory=%cd%)

Functional\registerABIs.cmd -unregister
