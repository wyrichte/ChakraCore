WScript.Echo("Before"); /**bp:resume('step_over');stack();resume('step_over');stack();resume('step_over');stack();resume('step_over');stack();resume('step_over');stack();resume('step_over');stack();resume('step_over');stack()**/
for (i in [1,2]) {
                WScript.Echo(i);
}
WScript.Echo("After");

