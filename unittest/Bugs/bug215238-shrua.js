//Configuration: inline.xml
//Testcase Number: 3710
//Bailout Testing: ON
//Switches:   -maxinterpretcount:1 -maxsimplejitruncount:1  -MinSwitchJumpTableSize:3 -bgjit- -loopinterpretcount:1 -force:fieldhoist -force:rejit -force:ScriptFunctionWithInlineCache -force:fixdataprops
//Baseline Switches: -nonative 
//Arch: AMD64
//Flavor: chk
//Branch:  fbl_ie_stage_dev3
//Build: 140620-2030
//FullBuild: 9773.0.140620
//MachineName: VSP35390
//InstructionSet: 
function test0() {
    function makeArrayLength(x) {
        if (!(x < 1)) {
            return Math.floor();
        }
    }
    var arrObj0 = {};
    var func0 = function () {
        arrObj0[0] = 156;
        return arrObj0[0];
    };
    arrObj0[12] = -167;
    protoObj1 = Object();
    length = makeArrayLength(~(arrObj0[(func0.call(protoObj1, 1, 1, 1) >= 0 ? func0.call(protoObj1, 1, 1, 1) : 0) & 15] >>> Object.prototype.prop2));
    WScript.Echo(length);
}
test0();
test0();
test0();

// === Baseline Output ===
// command: JsHost.exe         step.js
// exitcode: 0
// stdout: NaN
// NaN
// NaN
// 
// stderr: 
// 
// // === Actual Output ===
// command: JsHost.exe    -maxinterpretcount:1 -maxsimplejitruncount:1 -MinSwitchJumpTableSize:3 -bgjit- -loopinterpretcount:1 -force:fieldhoist -force:rejit -force:ScriptFunctionWithInlineCache -force:fixdataprops      step.js
// exitcode: 0
// stdout: NaN
// NaN
// undefined
// 
// stderr: 
// 
// 