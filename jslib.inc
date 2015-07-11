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

JS_LIBS_COMMON = \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\codex\$(O)\Chakra.Common.Codex.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\common\$(JS_LIBTYPE)\$(O)\chakra.common.common.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\core\$(JS_LIBTYPE)\$(O)\chakra.common.core.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\memory\$(JS_LIBTYPE)\$(O)\chakra.common.memory.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\util\$(JS_LIBTYPE)\$(O)\chakra.common.util.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\exceptions\$(JS_LIBTYPE)\$(O)\chakra.common.exceptions.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\common\datastructures\$(JS_LIBTYPE)\$(O)\chakra.common.datastructures.lib \
    $(JS_LIBS_MEMPROTECTHEAP)
    

JS_LIBS_PARSER_INC = \
    $(OBJECT_JSCRIPT_DIR)\core\lib\parser\$(JS_LIBTYPE_LS)\$(O)

JS_LIBS_PARSER = \
    $(OBJECT_JSCRIPT_DIR)\core\lib\parser\$(JS_LIBTYPE_LS)\$(O)\chakra.parser.lib 

JS_LIBS_RUNTIME = \
    $(OBJECT_JSCRIPT_DIR)\core\lib\runtime\language\$(JS_LIBTYPE)\$(O)\chakra.runtime.language.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\runtime\types\$(JS_LIBTYPE)\$(O)\chakra.runtime.types.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\runtime\library\$(JS_LIBTYPE)\$(O)\chakra.runtime.library.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\runtime\bytecode\$(JS_LIBTYPE_LS)\$(O)\chakra.runtime.bytecode.lib \
    $(OBJECT_JSCRIPT_DIR)\core\lib\runtime\Math\$(JS_LIBTYPE)\$(O)\chakra.runtime.Math.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\Telemetry\$(JS_LIBTYPE)\$(O)\chakra.Telemetry.lib \

JS_LIBS_WINRT = \
    $(OBJECT_JSCRIPT_DIR)\private\lib\winrt\$(JS_LIBTYPE)\$(O)\chakra.winrt.lib \

JS_LIBS_BACKEND = \
    $(OBJECT_JSCRIPT_DIR)\core\lib\backend\$(JS_LIBTYPE)\$(O)\chakra.backend.lib \

JS_LIBS_JSRT = \
    $(OBJECT_JSCRIPT_DIR)\core\lib\jsrt\$(JS_LIBTYPE)\$(O)\chakra.jsrt.lib \

JS_LINK_LIBS = \
    $(JS_LIBS_COMMON) \
    $(JS_LIBS_MEMPROTECTHEAP) \
    $(JS_LIBS_PARSER) \
    $(JS_LIBS_RUNTIME) \
    $(JS_LIBS_JSRT) \
!ifndef EXCLUDE_BACKEND
    $(JS_LIBS_BACKEND) \
!endif
