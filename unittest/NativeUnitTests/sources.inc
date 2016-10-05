TARGETLIBS=     \
    $(INTERNAL_SDK_LIB_PATH)\onecoreuapuuid.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(SDK_LIB_PATH)\oleaut32.lib \
    $(SDK_LIB_PATH)\onecore_downlevel.lib \
     \
    $(SDK_LIB_PATH)\onecore_downlevel.lib \
     \
    $(SDK_LIB_PATH)\onecore_downlevel.lib \
     \
    $(OBJECT_JSCRIPT_DIR)\private\bin\ChakraDiag\lib\$(O)\ChakraDiagLib.lib \
!if !$(FREEBUILD)
     \
!endif
