JSTHUNK_CFLAGS=$(EX_CXX_COMPILER_FLAGS)

# Remove /Gy to avoid warning about /Gy- overriding it.

JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:-Gy= )
JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:/Gy= )

# Remove /O2 it adds a lot of size between functions.
# /O1 does better, applied below.

JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:/O2= )
JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:-O2= )

# Remove /Od to avoid warning.

JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:/Odi= )
JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:-Odi= )
JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:/Od= )
JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS:-Od= )

# Something still implies /Gy so put /Gy- in, at/near the end.

JSTHUNK_CFLAGS=$(JSTHUNK_CFLAGS) /FC /Fo$@ /Gy- /O1

$(OBJ_PATH)\$O\jsthunk_thunks.obj: jsthunk_thunks.cpp
    $(CXX_COMPILER_NAME) @<<$@.rsp jsthunk_thunks.cpp
$(JSTHUNK_CFLAGS: =
)
<<Keep
