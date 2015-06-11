TARGETNAME=chakradiagtest
TARGETTYPE=DYNLINK

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
TEST_CODE=1
!ENDIF

FRETEST=1
!include ..\sources.inc

DLLDEF=$(OBJ_PATH)\$(O)\JScript9diagTest.def
