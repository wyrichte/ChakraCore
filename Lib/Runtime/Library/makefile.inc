$(OBJ_PATH)\$(O)\EngineInterfaceObject.obj: $(OBJ_PATH)\$(O)\IntlVerify.hdr

!IF "$(_BUILDARCH)" == "amd64"
$(OBJ_PATH)\$(O)\IntlVerify.hdr: ..\InJavaScript\Intl.js.bc.64b.h ..\InJavaScript\Intl.js
    ..\InJavascript\WarnGenByteCode.cmd ..\InJavaScript\Intl.js $(OBJ_PATH)\$(O)\IntlVerify.hdr ..\InJavaScript\Intl.js.bc.64b.h

!ELSE
$(OBJ_PATH)\$(O)\IntlVerify.hdr: ..\InJavaScript\Intl.js.bc.32b.h ..\InJavaScript\Intl.js
    ..\InJavascript\WarnGenByteCode.cmd ..\InJavaScript\Intl.js $(OBJ_PATH)\$(O)\IntlVerify.hdr ..\InJavaScript\Intl.js.bc.32b.h

!ENDIF
