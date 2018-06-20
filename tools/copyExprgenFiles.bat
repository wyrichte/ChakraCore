set destination=%1
set debuggerTestPercentage=%2
set hybridDebuggerTestPercentage=%3

IF "%destination%" == ""  (
	set destination=Exprgen
)
robocopy /e \\chakrafs\fs\Tools\vso\Exprgen %destination%
pushd %destination%\Configs
cscript /nologo %~dp0\JsUtilities\UpdateExprgenFlag.js DebuggerTestPercentage %debuggerTestPercentage%
cscript /nologo %~dp0\JsUtilities\UpdateExprgenFlag.js HybridDebuggerTestPercentage %hybridDebuggerTestPercentage%
popd
