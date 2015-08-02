//FileName: \\bptweb02\users\runtimetoolsbugs\casanalyzed\jsfunfuzz\th2_edge_stage_dev3\150730-1800\x86\nebularun_x86_fre\4b2635822e22467d9323c0a15a2d9cbb\ddltd1\bug_8_60f5ea0e-be67-4b19-babc-02a53ad201b2.js
//Baseline switches: -NoNative -NoDeferParse
//Switches: -MaxinterpretCount:2 -MaxSimpleJITRunCount:2 -forceSerialized -force:ObjTypeSpec -force:Atom -force:PolymorphicInlineCache -force:Rejit -force:Cse -bgjit- -loopinterpretcount:3
//MachineName: BPT56501
//Build: 150730-1800
//Branch: th2_edge_stage_dev3
//Binary: \\bptserver1\DailyBuild\th2_edge_stage_dev3\10509.0.150730-1800\X86fre
//Arch: X86
//reduced switches: -loopinterpretcount:3 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:2 -forceserialized
function test3() {
  try {
  } catch (c) {
    Object()() = b;
  }
}
print("pass");

// === Output ===
// command: D:\BinariesCache\th2_edge_stage_dev3\checkins\1577889\x86chk\JsHost.exe -loopinterpretcount:3 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:2 -forceserialized step183.js
// exitcode: C0000420
// stdout:
// 
// stderr:
// ASSERTION 1344: (d:\enlist\th2\inetcore\jscript\core\lib\runtime\language\bytecodeserializer.cpp, line 1013) Unknown OpLayout
//  Failure: (false)
// FATAL ERROR: jshost.exe failed due to exception code c0000420
// 
