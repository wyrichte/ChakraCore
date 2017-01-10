@echo off

setlocal

echo powershell -ExecutionPolicy bypass -Command "& '%~dpn0'" %*
powershell -ExecutionPolicy bypass -Command "& '%~dpn0'" %*

endlocal
