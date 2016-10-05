/*
  Let/const Indirect eval
*/

let bar = 1;

function Run(){
    let bar = 100;
    let foo = 200; 
    foo; /**bp:locals(1);stack();**/
    WScript.Echo(bar==100);
    WScript.Echo(foo==200);
}
eval('Run()');