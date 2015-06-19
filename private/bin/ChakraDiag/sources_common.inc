!include $(PROJECT_ROOT)\platform.inc
!include $(JSCRIPT_ROOT)\jslib.inc

USE_DEFAULT_WIN32_LIBS = 0
USE_UMBRELLA_LIBS = 1
USE_IDLBASED_DLLDATA = 1

# This is supposed to exclude lots of .h files from windows.h, which we most likely don’t need anyway, during build.
# See http://windows/winbuilddocs/Wiki%20Pages/NOT_LEAN_AND_MEAN.aspx for details.
WIN32_LEAN_AND_MEAN=1

INCLUDES= \
   $(INCLUDES) \
   $(COREINCLUDES); \
   $(SDK_LIB_PATH); \
   $(SDK_INC_PATH); \
   $(SDK_INC_PATH)\atlmfc; \
   $(SDKTOOLS_INC_PATH); \
   $(COM_INC_PATH); \
   $(COM_INC_PATH)\winrt; \
   $(OBJECT_INETCORE_DIR)\published\internal\inc\$(O); \
   $(PROJECT_OBJ_ROOT)\manifests\inbox\$(O); \
   $(COM_INC_PATH)\midlrt; \
   ..\; \
   ..\..\jscript; \
   ..\..\..\lib\common; \
   ..\..\..\lib\common\memory; \
   ..\..\..\lib\common\DataStructures; \
   ..\..\..\lib\runtime; \
   ..\..\..\lib\runtime\library; \
   ..\..\..\lib\runtime\language; \
   ..\..\..\lib\runtime\types; \
   ..\..\..\lib\runtime\bytecode; \
   ..\..\..\lib\backend; \

USER_INCLUDES=$(OBJ_PATH)\..\..\..\dll\retail\$(O)

C_DEFINES = $(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE -D__PLACEMENT_NEW_INLINE -DWIN32_DCOM -D_WINDLL

# generates SAL annotations for our interface
MIDL_OPTIMIZATION=$(MIDL_OPTIMIZATION) -sal_local

MUI_DRV=1

#4189: local variable is initialized but not referenced
#4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4189 /wd4290
