@echo off

setlocal

echo powershell -Command "& '%~dpn0'" -ExecutionPolicy bypass %*
powershell -Command "& '%~dpn0'" -ExecutionPolicy bypass %*

endlocal
