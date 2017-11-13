!include $(PROJECT_ROOT)\platform.inc
!include $(JSCRIPT_ROOT)\jslib.inc

USE_DEFAULT_WIN32_LIBS = 0
USE_UMBRELLA_LIBS = 1
USE_IDLBASED_DLLDATA = 1

# This is supposed to exclude lots of .h files from windows.h, which we most likely don�t need anyway, during build.
# See https://osgwiki.com/wiki/NOT_LEAN_AND_MEAN for details.
WIN32_LEAN_AND_MEAN=1

# Check out https://osgwiki.com/wiki/Dev_14_Migration for more details about -Zc:implicitNoexcept-
USER_C_FLAGS=$(USER_C_FLAGS) -Zc:implicitNoexcept-

INCLUDES= \
   $(INCLUDES) \
   $(COREINCLUDES); \
   $(SDK_LIB_PATH); \
   $(SDK_INC_PATH); \
   $(SDK_INC_PATH)\atlmfc; \
   $(ONECORESDKTOOLS_INC_PATH); \
   $(SDKTOOLS_INC_PATH); \
   $(COM_INC_PATH); \
   $(COM_INC_PATH)\winrt; \
   $(OBJECT_INETCORE_DIR)\published\internal\inc\$(O); \
   $(PROJECT_OBJ_ROOT)\manifests\inbox\$(O); \
   $(COM_INC_PATH)\midlrt; \
   ..\; \
   $(PROJECT_ROOT)\jscript\private\bin\Chakra; \
   $(PROJECT_ROOT)\jscript\core\lib\common; \
   $(PROJECT_ROOT)\jscript\core\lib\common\core; \
   $(PROJECT_ROOT)\jscript\core\lib\common\memory; \
   $(PROJECT_ROOT)\jscript\core\lib\common\DataStructures; \
   $(PROJECT_ROOT)\jscript\core\lib\runtime; \
   $(PROJECT_ROOT)\jscript\core\lib\runtime\debug; \
   $(PROJECT_ROOT)\jscript\core\lib\runtime\library; \
   $(PROJECT_ROOT)\jscript\core\lib\runtime\language; \
   $(PROJECT_ROOT)\jscript\core\lib\runtime\types; \
   $(PROJECT_ROOT)\jscript\core\lib\runtime\bytecode; \
   $(PROJECT_ROOT)\jscript\core\lib\backend; \

USER_INCLUDES=$(OBJ_PATH)\..\..\..\dll\retail\$(O)

C_DEFINES = $(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE -D_WINDLL

# generates SAL annotations for our interface
MIDL_OPTIMIZATION=$(MIDL_OPTIMIZATION) -sal_local

MUI_DRV=1

#4189: local variable is initialized but not referenced
#4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4189 /wd4290
