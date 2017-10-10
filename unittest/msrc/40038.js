// x86_debug
// ch.exe test.js -mic:1 -MaxSimpleJITRunCount:3 -force:FieldCopyProp
function foo() { }
Object.preventExtensions(this);

function test5(){
    foo([b] = "");
};
test5();
test5();
test5();
test5();
test5();
test5();
test5();


print('pass');