::
:: ExprGen information can be found at:
::
:: http://devdiv/sites/bpt/team/eze/Eze%20Wiki/ExprGen.aspx
::
IF %PROCESSOR_ARCHITECTURE% == ARM (
	CALL :RunARM
	GOTO :EOF
)
set branch=%1
set debuggerTestPercentage=%2
set hybridDebuggerTestPercentage=%3
set flavour=%_BuildArch%%_BuildType%
md %sdxroot%\inetcore\jscript\tools\ExprGen\%flavour%
pushd %sdxroot%\inetcore\jscript\tools\ExprGen\%flavour%

IF "%branch%" == ""  (
	set branch=%_BuildBranch%
)
:: Copy the most recent ExprGen and config file drop locally.  If the share is unavailable,
:: just rely on the last locally copied drop of ExprGen.
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\x86fre  . ExprGen.exe
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\x86fre  . TraceOpts.dll
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\ExprgenXML  . *.xml
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\ExprgenXML  . Traceopts.dll.config
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\ExprgenXML  . TraceOpts.Stylesheet.css
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\ExprgenXML\Snippets  .\Snippets *.ecs
GOTO :EOF


:RunARM
set branch=%1
md .\ExprGen
pushd .\ExprGen

IF "%branch%" == ""  (
	set branch=%_BuildBranch%
)
:: Copy the most recent ExprGen and config file drop locally.  If the share is unavailable,
:: just rely on the last locally copied drop of ExprGen.
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\woafre  . ExprGen.exe 
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\ExprgenXML  . *.xml
robocopy /W:5 /R:3 /NJS /NJH \\bptserver1\dailybuild\%branch%\Latest\ExprgenXML\Snippets  .\Snippets *.ecs
GOTO :EOF

:EOF
cscript /nologo JsUtilities\UpdateExprgenFlag.js DebuggerTestPercentage %debuggerTestPercentage%
cscript /nologo JsUtilities\UpdateExprgenFlag.js HybridDebuggerTestPercentage %hybridDebuggerTestPercentage%
