@echo off
setlocal
set _CL_=/DRLEXE_XML_DATASOURCE
pushd %~dp0
build -cz
popd

set _root = %_JSCRIPT_ROOT%
IF "%_root%" == "" (
    set _root=%_NTROOT%\inetcore\jscript
)
pushd %_root%
sd edit ...\Taef_*.xml
te %_NTTREE%\jscript\taef\ChakraNativeTaefTests.dll /p:SkipExec=1 /p:GenerateXml=1 > nul
sd revert -a ...\Taef_*.xml
popd

set _CL_=
pushd %~dp0
build -cz -dir %~dp0;%_root%\unittest\TaefDataSource;%_root%\projectionTests\Tests\TaefDataSource
popd


