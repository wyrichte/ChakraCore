$(OBJ_PATH)\$(O)\published\Winery.winmd:
    mdmerge -metadata_dir $(SDK_METADATA_PATH) -i $(OBJ_PATH)\$(O) -o $(OBJ_PATH)\$(O)\published -n:1 -partial