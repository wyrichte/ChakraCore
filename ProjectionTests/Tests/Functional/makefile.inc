DoBinplaceTestFiles :
    echo $(BUILD_PASS)
!if "$(BUILD_PASS)" == "PASS2"
# Check some environment variables to see if we should binplace the test collateral. We only want to do this for official build lab builds.
# Note: In the non-official build, we still have to put some js/baseline in the obj folder for the binplace step to succeed. 
#       We just choose the first arbitrary file.
# OFFICIAL_BUILD_MACHINE should indicate primary/secondary/etc status for the current build machine and is only present in build lab.
# TIMEBUILD is a flag indicating that the build is a time build.
# USERNAME is the alias of the dev. For build lab, this is wincbld.
   echo BUILDMSG: OFFICIAL_BUILD_MACHINE is "$(OFFICIAL_BUILD_MACHINE)", TIMEBUILD is "$(TIMEBUILD)", USERNAME is "$(USERNAME)"
!if "$(OFFICIAL_BUILD_MACHINE)" != "" || "$(TIMEBUILD)" != "" || "$(USERNAME)" == "wincbld"
   echo BUILDMSG: copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.js $(OBJ_PATH)\$O
   copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.js $(OBJ_PATH)\$O >nul 2>&1
   echo BUILDMSG: copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.baseline $(OBJ_PATH)\$O
   copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.baseline $(OBJ_PATH)\$O >nul 2>&1
!else
   echo BUILDMSG: Generating dummy js/baseline.
   echo dummyfile > $(OBJ_PATH)\$O\dummy.js
   echo dummyfile > $(OBJ_PATH)\$O\dummy.baseline
!endif
!endif
