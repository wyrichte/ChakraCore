DoBinplaceTestFiles :
    echo $(BUILD_PASS)
!if "$(BUILD_PASS)" == "PASS2"
   echo BUILDMSG: OFFICIAL_BUILD_MACHINE is "$(OFFICIAL_BUILD_MACHINE)", TIMEBUILD is "$(TIMEBUILD)", USERNAME is "$(USERNAME)"
   echo BUILDMSG: copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.js $(OBJ_PATH)\$O
   copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.js $(OBJ_PATH)\$O >nul 2>&1
   echo BUILDMSG: copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.baseline $(OBJ_PATH)\$O
   copy $(JSCRIPT_ROOT)\ProjectionTests\Tests\Functional\*.baseline $(OBJ_PATH)\$O >nul 2>&1
!endif
