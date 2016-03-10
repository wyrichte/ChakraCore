@echo off
set SDXROOT=C:\rs1_dev3
C:\rs1_dev3\tools\perl\bin\perl.exe -I C:\rs1_dev3\tools -I C:\rs1_dev3\tools\perllib C:\ChakraGit\VSOToGitHubPump.pl
exit /b %errorlevel%