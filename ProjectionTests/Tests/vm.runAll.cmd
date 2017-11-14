@echo off
call .\vm.setup.cmd

call .\RunAllTests.cmd -snapTests -binaryRoot C:\ProjectionTests\JsHost
