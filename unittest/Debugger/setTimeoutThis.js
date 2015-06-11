/**exception(resume_ignore):evaluate("this")**/

// Windows Blue Bugs: Bug 612664: CAS:WebCrawler:  AV: jscript9.dll!Js::GlobalObject::UpdateThisForEval
//
// jshost -targeted -debuglaunch -forceNative
//
// ARM only:
//      setTimeout calls root function with "this" arg. But ARM jit has an optimization to not home "this"
//      for Global function. This causes CallInfo.Count == 1, but "this" isn't on stack, AV when stackwalker
//      tries to grab "this" var from stack.

WScript.SetTimeout("throw 0", 10);

WScript.Echo("pass");
