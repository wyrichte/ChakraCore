for %%i in ( client css js libraries ) do (
    @rem echo copy /y D:\enlistments\inetcore\jscript\F12IRViewer\%%i\*.* D:\VSClient_1\src\bpt\diagnostics\irviewer\%%i\*.*
    xcopy /s /e /y D:\enlistments\inetcore\jscript\F12IRViewer\%%i\*.* D:\VSClient_1\src\bpt\diagnostics\irviewer\%%i\*.*
)

for %%i in ( comm.js data.js header.js irviewer.html remote.js ) do (
    xcopy /y D:\enlistments\inetcore\jscript\F12IRViewer\%%i D:\VSClient_1\src\bpt\diagnostics\irviewer\%%i
)
