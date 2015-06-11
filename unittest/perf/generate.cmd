@echo off

set _ROOT=%~dp0

for /r %%i in (*.tst) do call :GenerateTest %%i

set _ROOT=

goto :EOF

:GenerateTest
    pushd %~dp1
    set _TEST=%~n1
    echo Generating test for "%_TEST%"
    type "%_TEST%.tst" > "%_TEST%.js"
    type "%_ROOT%\template.js" >> "%_TEST%.js"
    echo ^<pre^>^<script type="text/javascript" src="%_TEST%.js"^>^</script^>^</pre^> >"%_TEST%.htm"
    set _TEST=
    popd
    goto :EOF
