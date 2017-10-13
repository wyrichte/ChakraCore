# build/link config for release or test or language service binaries

!if $(FREEBUILD) && defined(FRETEST)
JS_LIBTYPE = test
C_DEFINES = $(C_DEFINES) -DENABLE_DEBUG_CONFIG_OPTIONS=1
!else
JS_LIBTYPE = release
!endif

!ifdef JS_LANGUAGE_SERVICE
!if $(FREEBUILD) && defined(FRETEST)
JS_LIBTYPE_LS = ls_test
!else
JS_LIBTYPE_LS = ls_release
!endif
!else
JS_LIBTYPE_LS = $(JS_LIBTYPE)
!endif

JS_LIBS_MEMPROTECTHEAP = \
    $(OBJECT_JSCRIPT_DIR)\private\lib\memprotectheap\$(JS_LIBTYPE)\$(O)\chakra.memprotectheap.lib \

JS_LIBS_RECYCLERHEAP = \
    $(OBJECT_JSCRIPT_DIR)\private\lib\recyclerheap\$(JS_LIBTYPE)\$(O)\chakra.recyclerheap.lib \

JS_LIBS_PLATFORMAGNOSTIC = \
	  $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\PlatformAgnostic\$(JS_LIBTYPE)\$(O)\chakra.runtime.PlatformAgnostic.lib \

JS_LIBS_COMMON = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\codex\$(O)\Chakra.Common.Codex.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\common\$(JS_LIBTYPE)\$(O)\chakra.common.common.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\core\$(JS_LIBTYPE)\$(O)\chakra.common.core.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\memory\$(JS_LIBTYPE)\$(O)\chakra.common.memory.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\util\$(JS_LIBTYPE)\$(O)\chakra.common.util.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\exceptions\$(JS_LIBTYPE)\$(O)\chakra.common.exceptions.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\common\datastructures\$(JS_LIBTYPE)\$(O)\chakra.common.datastructures.lib \
    $(JS_LIBS_MEMPROTECTHEAP) \
    $(JS_LIBS_RECYCLERHEAP) \
    $(JS_LIBS_PLATFORMAGNOSTIC)

JS_LIBS_PARSER_INC = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\parser\$(JS_LIBTYPE_LS)\$(O)

JS_LIBS_PARSER = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\parser\$(JS_LIBTYPE_LS)\$(O)\chakra.parser.lib\
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\WasmReader\$(JS_LIBTYPE_LS)\$(O)\chakra.WasmReader.lib\

JS_LIBS_RUNTIME = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\language\$(JS_LIBTYPE)\$(O)\chakra.runtime.language.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\types\$(JS_LIBTYPE)\$(O)\chakra.runtime.types.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\library\$(JS_LIBTYPE)\$(O)\chakra.runtime.library.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\base\$(JS_LIBTYPE)\$(O)\chakra.runtime.base.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\bytecode\$(JS_LIBTYPE_LS)\$(O)\chakra.runtime.bytecode.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\debug\$(JS_LIBTYPE_LS)\$(O)\chakra.runtime.debug.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\runtime\Math\$(JS_LIBTYPE)\$(O)\chakra.runtime.Math.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\Telemetry\$(JS_LIBTYPE)\$(O)\chakra.Telemetry.lib \

!if (!$(FREEBUILD) || defined(FRETEST)) && ("$(_BUILDARCH)" == "amd64" || "$(_BUILDARCH)" == "x86")
JS_LIBS_RUNTIME = $(JS_LIBS_RUNTIME)\
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\wabt\$(JS_LIBTYPE)\$(O)\wabt.lib \
!endif

JS_LIBS_BACKEND = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\backend\$(JS_LIBTYPE)\$(O)\chakra.backend.lib \

JS_LIBS_JITSERVER = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\JITClient\$(JS_LIBTYPE)\$(O)\chakra.JITClient.lib \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\JITServer\$(JS_LIBTYPE)\$(O)\chakra.JITServer.lib \

JS_LINK_LIBS = \
    $(JS_LIBS_COMMON) \
    $(JS_LIBS_MEMPROTECTHEAP) \
    $(JS_LIBS_RECYCLERHEAP) \
    $(JS_LIBS_PARSER) \
    $(JS_LIBS_RUNTIME) \
    $(JS_LIBS_JITSERVER) \
!ifndef EXCLUDE_BACKEND
    $(JS_LIBS_BACKEND) \
!endif

JS_LIBS_JSRT = \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\jsrt\$(JS_LIBTYPE)\$(O)\chakra.jsrt.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\jsrtchakra\$(JS_LIBTYPE)\$(O)\chakra.jsrtchakra.lib \

JS_LIBS_WINRT = \
    $(OBJECT_JSCRIPT_DIR)\private\lib\winrt\$(JS_LIBTYPE)\$(O)\chakra.winrt.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\projection\$(JS_LIBTYPE)\$(O)\chakra.projection.lib \


JS_LIBS_ENGINE = \
    $(OBJECT_JSCRIPT_DIR)\private\lib\engine\$(JS_LIBTYPE)\$(O)\chakra.engine.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\sca\$(JS_LIBTYPE)\$(O)\chakra.SCA.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\enc\$(JS_LIBTYPE)\$(O)\chakra.EnC.lib \

