@echo off

rem Arg 1 - path to perl.exe
rem Arg 2 - path to source file to encode
rem Arg 3 - path to destination encoded file

rem perl doesn't seem to like multiple -e switches in one set of arguments
rem \x22 is the double-quote character; perl doesn't seem to like the double-quote in command line arguments

call %1 -p -e "s/\/\/ File .*//g" < %2 > "%~3.temp1"
call %1 -p -e "s/\$([A-Z]*)[0-9]+:[ \t]/\$\1/g" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/\$([A-Z]*)[0-9]+#[ \t]*/\$\1/g" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/\$([A-Z]*)[0-9]+/\$\1/g" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/ebp-[0-9]+/ebp/" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/esp\+[0-9]+/esp/" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/tv[0-9]+/tv/" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/\[sp\+[0-9]+/\[sp/" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/NEAR PTR //" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/COMM NEAR.*$//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/ORG.*$//" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/EXTRN.*$//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/COMM FAR.*$//" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/;[ \t]*Line.*//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/^; [0-9]+ +:/; :/" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/\/\/[0-9]+.*$//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/ASSUME.*$/ASS/" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/TITLE.*//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/^\tnpad.*$//" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/^; Block.*$//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/^\/\/ Block.*$//" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/R-Addr:.*$//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/(token:.*)/token/" < "%~3.temp2" > "%~3.temp1"
call %1 -p -e "s/^; Listing generated.*//" < "%~3.temp1" > "%~3.temp2"
call %1 -p -e "s/^\/\/ Listing generated.*//" < "%~3.temp2" > %3

del /f /q "%~3.temp1"
del /f /q "%~3.temp2"
