
$(OBJ_PATH)\$(O)\dbgcontroller.js.encoded: dbgcontroller.js

$(OBJ_PATH)\$(O)\dbgcontroller.js.encoded:
	encode.cmd perl dbgcontroller.js "$(OBJ_PATH)\$(O)\dbgcontroller.js.encoded"