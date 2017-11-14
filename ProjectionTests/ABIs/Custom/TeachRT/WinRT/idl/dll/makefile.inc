$(OBJ_PATH)\$O\ComposedMetadata: $(OBJ_PATH)\$O\fabrikam.h ..\Fabrikam.idl makefile.inc
        set BUILDMSG=Making $(@F)
        $(RAZZLETOOLPATH)\x86\mdmerge.exe -metadata_dir $(PROJECT_SDK_METADATA_PATH) -i $(OBJ_PATH)\$O -o $(OBJ_PATH)\$O\Output -partial -v
        set BUILDMSG=Made $(@F)
        echo Done. >> $(OBJ_PATH)\$O\ComposedMetadata