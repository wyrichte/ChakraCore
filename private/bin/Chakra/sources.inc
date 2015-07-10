!include $(PROJECT_ROOT)\platform.inc
!include $(JSCRIPT_ROOT)\jslib.inc

MIDL_TARGET=NT60
USE_NOTHROW_NEW=1
!if "$(JS_LANGUAGE_SERVICE)" != "1"
BUILD_FOR_CORESYSTEM=1
!endif
USE_DEFAULT_WIN32_LIBS=0
USE_UMBRELLA_LIBS=1
USE_64BIT_LINKER=1
LINK_TIME_CODE_GENERATION=1

!if !$(ARM64)
LINKER_FLAGS=$(LINKER_FLAGS) -d2:-pspeed1
!endif

USER_INCLUDES=$(JS_LIBS_PARSER_INC);$(OBJ_PATH)\..\..\..\dll\retail\$(O)

C_DEFINES =$(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE

USE_IDLBASED_DLLDATA=1

#4510: default constructor could not be generated
#4610: can never be instantiated - user defined constructor required
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4510 /wd4610

# generates SAL annotations for our interface
MIDL_OPTIMIZATION=$(MIDL_OPTIMIZATION) -sal_local

!if "$(JS_LANGUAGE_SERVICE)" != "1"
DLLDEF_OBJECTS=$(DLLDEF_OBJECTS) \
    $(JS_LIBS_MEMPROTECTHEAP)
!endif

MUI_DRV=1

INCLUDES= \
    $(INCLUDES) \
    $(COREINCLUDES); \
    $(SDK_LIB_PATH); \
    $(SDK_INC_PATH); \
    $(SDK_INC_PATH)\atl30; \
    $(COM_INC_PATH); \
    $(COM_INC_PATH)\winrt; \
    $(PROJECT_ROOT)\lib\inc; \
    $(PROJECT_OBJ_ROOT)\manifests\inbox\$(O); \
    $(COM_INC_PATH)\midlrt; \
    $(PROJECT_ROOT)\jscript\lib\runtime\types; \
    $(PROJECT_OBJ_ROOT)\jscript\manifests\$(O); \
    ..\; \
!if "$(_BUILDARCH)" == "arm"
    ..\arm\; \
!endif
!if "$(_BUILDARCH)" == "arm64"
    ..\arm64\; \
!endif

# These are used only when projection is enabled.
PROJECTION_SOURCES = \
        ..\AbiDelegates.cpp \
        ..\AbiEvents.cpp \
        ..\ArrayAsCollection.cpp \
        ..\ArrayAsIterable.cpp \
        ..\ArrayAsIterator.cpp \
        ..\ArrayAsVector.cpp \
        ..\ArrayProjection.cpp \
        ..\ArrayProjectionEnumerator.cpp \
        ..\ExternalWeakReferenceImpl.cpp \
        ..\ExternalWeakReferenceSourceImpl.cpp \
        ..\FinalizableTypedArrayContents.cpp \
        ..\IBufferProjection.cpp \
        ..\InspectableObjectTypeOperations.cpp \
        ..\ObjectAsIPropertyValue.cpp \
        ..\ObjectAsIReference.cpp \
        ..\ProjectionContext.cpp \
        ..\ProjectionFastPath.cpp \
        ..\ProjectionMemoryInformation.cpp \
        ..\ProjectionMethodInvoker.cpp \
        ..\ProjectionMarshaler.cpp \
        ..\projectionTypeOperations.cpp \
        ..\ProjectionWriter.cpp \
        ..\PropertySet.cpp \
        ..\PropertySetEnumerator.cpp \
        ..\namespaceprojection.cpp \
        ..\NamespaceProjectionEnumerator.cpp \
        ..\UnknownImpl.cpp \
        ..\VariableArgMethodHelper.cpp \
        ..\VectorArray.cpp \
        ..\VectorArrayEnumerator.cpp \
        ..\ProjectionObjectInstance.cpp \
        ..\ProjectionAsyncDebug.cpp \

EDIT_AND_CONTINUE_SOURCES = \
        ..\EditTest.cpp \
        ..\ScriptEdit.cpp \
        ..\SemanticChange.cpp \

# NON_LANGUAGE_SERVICE_SOURCES is appended in release/test sources
NON_LANGUAGE_SERVICE_SOURCES = \
        ..\jsrt.cpp \
        ..\JsrtShared.cpp \
        ..\JsrtContext.cpp \
        ..\JsrtDelegateWrapper.cpp \
        ..\JsrtExternalArrayBuffer.cpp \
        ..\JsrtRuntime.cpp \
        ..\JsrtThreadService.cpp \

SOURCES= \
        ..\activeScriptError.cpp \
        ..\ActiveScriptProfilerHeapEnum.cpp \
        ..\binaryfeaturecontroloverrides.cpp \
        ..\breakpointProbe.cpp \
        ..\caller.cpp \
        ..\CJavascriptOperations.cpp \
        ..\classfac.cpp \
        ..\codectx.cpp \
        ..\customenumerator.cpp \
        ..\CustomExternalType.cpp \
        ..\dbgfmt.cpp \
        ..\dbgprop.cpp \
        ..\DebugObject.cpp \
        ..\DefaultScriptOperations.cpp \
        ..\DiagnosticsScriptObject.cpp \
        ..\DispatchHelper.cpp \
        ..\DispMemberProxy.cpp \
        ..\DispMemberProxyType.cpp \
        ..\DOMProperties.cpp \
        ..\dllfunc.cpp \
        ..\EnumVariantEnumerator.cpp \
        ..\ErrorTypeHelper.cpp \
        ..\externalobject.cpp \
        ..\eventsink.cpp \
        ..\guids.c \
        ..\HostDispatch.cpp \
        ..\HostDispatchEnumerator.cpp \
        ..\HostDispatchType.cpp \
        ..\HostObject.cpp \
        ..\HostObjectType.cpp \
        ..\HostVariant.cpp \
        ..\JavascriptDispatch.cpp \
        ..\JavascriptExternalOperators.cpp \
        ..\JavascriptPixelArrayDispatch.cpp \
        ..\javascriptThreadService.cpp \
        ..\jscriptinfo.idl \
        ..\jsfac.cpp \
        ..\JsrtCustomEnumerator.cpp \
        ..\JsrtExternalObject.cpp \
        ..\MemspectMemoryTracking.cpp \
        ..\NamedEventHandler.cpp \
        ..\NamedItemList.cpp \
        ..\ProfileDataObject.cpp \
        ..\proxystub.c \
        ..\QueryContinuePoller.cpp \
        ..\refCountedHostVariant.cpp \
        ..\SCADeserialization.cpp \
        ..\SCAPropBag.cpp \
        ..\SCASerialization.cpp \
        ..\scpnode.cpp \
        ..\scptext.cpp \
        ..\ScriptDAC.cpp \
        ..\DiagHook.cpp \
        ..\ScriptEnginebase.cpp \
        ..\ScriptEngine.cpp \
        ..\ScriptSite.cpp \
        ..\scrpting.cpp \
        ..\siteserv.cpp \
        ..\ScriptDebugDocument.cpp \
        ..\StreamHelper.cpp \
        ..\StreamReader.cpp \
        ..\StreamWriter.cpp \
        ..\TestHooks.cpp \
        ..\ThreadServiceWrapperBase.cpp \
        ..\typeinfobuilder.cpp \
        ..\DynamicSourceHolder.cpp \
        $(PROJECTION_SOURCES) \
        $(EDIT_AND_CONTINUE_SOURCES) \

AMD64_SOURCES=\
    $(AMD64_SOURCES) \
    ..\amd64\projectioncall.asm \
    ..\amd64\UnknownImplHelper.asm \

ARM_SOURCES = \
    $(ARM_SOURCES) \
    ..\arm\projectioncall.asm \
    ..\arm\UnknownImplHelper.asm \
    ..\arm\CallingConvention.cpp \

ARM64_SOURCES = \
    $(ARM64_SOURCES) \
    ..\arm64\projectioncall.asm \
    ..\arm64\UnknownImplHelper.asm \
    ..\arm64\CallingConvention.cpp \


# Dependent Libraries
#   iedevtools_host_iel1.lib:
#       LoadLatestPDM
#       ShouldUseLocalPDM
#
#   ieshare_common_iel1.lib:
#       IsOs_Mobilecore
#
#   Chakra.Common.Codex.lib:
#       utf8 support
#
#   IESettings*, iertutilp.lib
#       SettingStore
#
#   CommonUtilityLibrary.lib:
#       _DoNotUseDirectly_CULprintf
#
#   PathCchApis.lib: (to avoid loading api-ms-win-core-path-l1-1-0.dll)
#       PathCchCombine  (used in iedevtools_host_iel1.lib)
#       PatthCchAppend  (used in iedevtools_host_iel1.lib)

LINKLIBS= \
    $(INETCORE_LIB_PATH)\iertutilp.lib \
    $(INETCORE_LIB_PATH)\IESettingsApi.lib \
    $(INETCORE_LIB_PATH)\IESettingsId.lib \
    $(JS_LIBS_WINRT) \
    $(JS_LINK_LIBS) \
    $(MINCORE_PRIV_SDK_LIB_PATH)\pathcchapis.lib    \
    $(OBJECT_INETCORE_DIR)\jscript\core\lib\common\codex\$(O)\Chakra.Common.Codex.lib \
    $(OBJECT_INETCORE_DIR)\lib\devtb\dtbhost\$(O)\iedevtools_host_iel1.lib \
    $(OBJECT_INETCORE_DIR)\lib\ScriptProjectionHost\iel3_edge\$(O)\ScriptProjectionHost_Edge_iel3.lib \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib \

TARGETLIBS= \
    $(BASE_LIB_PATH)\AppModel\Common\AppModel.Common.lib \
    $(MINCORE_PRIV_SDK_LIB_PATH)\1.3\ext-ms-win-com-ole32-l1.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-rometadata-dispenser-l1.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\mincore_private.lib \
    $(MINCORE_SDK_LIB_PATH)\mincore_fw.lib    \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-ro-typeresolution-l1.lib \
    $(ONECORE_SDK_LIB_VPATH)\onecore.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(SDK_LIB_PATH)\ole32.lib \
    $(SDK_LIB_PATH)\Uuid.lib \
!if (!$(FREEBUILD))
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-ntuser-dialogbox-l1.lib \
    $(SDK_LIB_PATH)\dbghelp.lib \
!endif
!IF (!$(FREEBUILD) || defined(FRETEST) || defined(NTTESTENV)) && "$(TARGET_DIRECTORY)" != "arm" && "$(TARGET_DIRECTORY)" != "arm64"
    ..\..\..\tools\external\lib\$(TARGET_DIRECTORY)\jitprofiling.lib \
!ENDIF


# Delay load
DELAYLOAD = \
    oleaut32.dll; \
    ole32.dll; \
    cryptsp.dll; \
    crypt32.dll; \
!if "$(JS_LANGUAGE_SERVICE)" != "1"
    api-ms-win-core-winrt-l1; \
    api-ms-win-core-com-l1-1-0.dll; \
    api-ms-win-ro-typeresolution-l1; \
    ext-ms-win-rometadata-dispenser-l1-1-0.dll; \
!IF (!$(FREEBUILD))
    ext-ms-win-ntuser-dialogbox-l1-1-0.dll; \
!ENDIF
!endif

ALLOW_NT_STYLE_DLOAD_ERROR_HANDLER=1
DLOAD_ERROR_HANDLER=kernelbase.dll
# End delay load

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1
