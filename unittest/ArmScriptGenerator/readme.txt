ArmScriptGenerator

    This program grovels through the scripts that define the jscript unit tests (rldirs.txt & ...\rlexe.xml)
and generates a batch file that can be executed in platfom builder to run the corresponding tests on the ARM.

To generate the batch file (from the jscript/unittest directory):
    ArmScriptGenerator rldirs.txt >armUnittests.bat

