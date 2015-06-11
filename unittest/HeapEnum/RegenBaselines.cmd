jshost -nativetest:NativeUnitTests.dll HeapEnum DoOneEnumWithNewEngine.js > DoOneEnumWithNewEngine.baseline
jshost -nativetest:NativeUnitTests.dll HeapEnum DoMultipleEnumsWithinOneEngine.js > DoMultipleEnumsWithinOneEngine.baseline
jshost -nativetest:NativeUnitTests.dll HeapEnum DoMultipleEnumsAcrossEngines.js > DoMultipleEnumsAcrossEngines.baseline
jshost array.js > array.baseline
jshost arguments.js > arguments.baseline
jshost attributes.js > attributes.baseline
jshost properties.js > properties.baseline
jshost internalProperties.js > internalProperties.baseline
jshost deletedProperties.js > deletedProperties.baseline
jshost string.js > string.baseline
jshost copyonwrite.js > copyonwrite.baseline
jshost closure.js > closure.baseline
jshost closure_binding.js > closure_binding.baseline
jshost closure_nested.js > closure_nested.baseline
jshost closure_nested_eval.js > closure_nested_eval.baseline
jshost hosttypes.js > hosttypes.baseline
jshost -html %sdxroot%\inetcore\jscript\unittest\HeapEnum\SimpleDom.html > SimpleDom.baseline
jshost -html %sdxroot%\inetcore\jscript\unittest\HeapEnum\SimpleFastDom.html > SimpleFastDom.baseline
jshost -html %sdxroot%\inetcore\jscript\unittest\HeapEnum\SimpleParentWithFrame.html > SimpleParentWithFrame.baseline
