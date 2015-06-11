
$(OBJ_PATH)\$(O)\jsGlass.js.encoded: jsGlass.js

$(OBJ_PATH)\$(O)\jsGlass.js.encoded:
	encode.cmd perl jsGlass.js "$(OBJ_PATH)\$(O)\jsGlass.js.encoded"