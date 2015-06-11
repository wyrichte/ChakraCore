@echo off

rem Encode a file so it can be #included as a WCHAR string literal in C++

rem Arg 1 - path to perl.exe
rem Arg 2 - path to source file to encode
rem Arg 3 - path to destination encoded file

rem perl doesn't seem to like multiple -e switches in one set of arguments
rem \x22 is the double-quote character; perl doesn't seem to like the double-quote in command line arguments

call %1 -p -e "s/\\/\\\\/g" < %2 > "%~3.temp1"
call %1 -p -e "s/\x22/\\\x22/g" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/^/L\x22/g" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/\r?\n/\\r\\n\x22\n/g" < "%~3.temp1" > %3

del /f /q "%~3.temp1"
del /f /q "%~3.temp2"
