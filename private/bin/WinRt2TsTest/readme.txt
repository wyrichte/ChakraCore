RUNNING UNIT TESTS FOR WINRT2TS.EXE
===========================

Build the ChakraFull project (from project root) :

msbuild build\Chakra.Full.sln /p:Configuration=all-Debug /p:Platform=x86


Run tests for the flavor just built (from project root) :

tools\runwinrt2tstests.cmd -x86debug
