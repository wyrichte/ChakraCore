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

!ifdef JSGEN
JS_LIBS_WINRT_TYPE = jsgen
!else
JS_LIBS_WINRT_TYPE = release
!endif

JS_LIBS_MEMPROTECTHEAP = \
    $(OBJECT_JSCRIPT_DIR)\lib\memprotectheap\$(JS_LIBTYPE)\$(O)\jscript.memprotectheap.lib \

JS_LIBS_COMMON = \
    $(OBJECT_JSCRIPT_DIR)\lib\common\common\$(JS_LIBTYPE)\$(O)\jscript.common.common.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\core\$(JS_LIBTYPE)\$(O)\jscript.common.core.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\memory\$(JS_LIBTYPE)\$(O)\jscript.common.memory.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\exceptions\$(JS_LIBTYPE)\$(O)\jscript.common.exceptions.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\datastructures\$(JS_LIBTYPE)\$(O)\jscript.common.datastructures.lib \
    $(JS_LIBS_MEMPROTECTHEAP)
    

JS_LIBS_PARSER_INC = \
    $(OBJECT_JSCRIPT_DIR)\lib\parser\$(JS_LIBTYPE_LS)\$(O)

JS_LIBS_PARSER = \
    $(OBJECT_JSCRIPT_DIR)\lib\parser\$(JS_LIBTYPE_LS)\$(O)\jscript.parser.lib 

JS_LIBS_RUNTIME = \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\language\$(JS_LIBTYPE)\$(O)\jscript.runtime.language.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\types\$(JS_LIBTYPE)\$(O)\jscript.runtime.types.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\library\$(JS_LIBTYPE)\$(O)\jscript.runtime.library.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\bytecode\$(JS_LIBTYPE_LS)\$(O)\jscript.runtime.bytecode.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\Math\$(JS_LIBTYPE)\$(O)\jscript.runtime.Math.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\Telemetry\$(JS_LIBTYPE)\$(O)\jscript.runtime.Telemetry.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\Telemetry\ESBuiltIns\$(JS_LIBTYPE)\$(O)\jscript.runtime.Telemetry.ESBuiltIns.lib \

JS_LIBS_WINRT = \
    $(OBJECT_JSCRIPT_DIR)\lib\winrt\$(JS_LIBS_WINRT_TYPE)\$(O)\jscript.winrt.lib \

JS_LIBS_BACKEND = \
    $(OBJECT_JSCRIPT_DIR)\lib\backend\$(JS_LIBTYPE)\$(O)\jscript.backend.lib \

JS_LINK_LIBS = \
    $(JS_LIBS_COMMON) \
    $(JS_LIBS_MEMPROTECTHEAP) \
    $(JS_LIBS_PARSER) \
    $(JS_LIBS_RUNTIME) \
!ifndef EXCLUDE_BACKEND
    $(JS_LIBS_BACKEND) \
!endif
