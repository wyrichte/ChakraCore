$(TARGETNAME).cab :
    echo $(BUILD_PASS)
!if "$(BUILD_PASS)" == "PASS2"
   echo BUILDMSG: Building ProjectionTests cab file via BuildTestsCab.cmd $(OBJ_PATH)\$O $(JSCRIPT_ROOT) $(TARGETNAME).cab $(PROJECT_SDK_METADATA_PATH)
   BuildTestsCab.cmd $(OBJ_PATH)\$O $(JSCRIPT_ROOT) $(TARGETNAME).cab $(PROJECT_SDK_METADATA_PATH) > $(OBJ_PATH)\$O\BuildTestsCab.log 2>&1
!endif