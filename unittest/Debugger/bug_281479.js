//Configuration: full.xml
//Testcase Number: 35329
//Bailout Testing: ON
//Switches:   -maxinterpretcount:1  -debuglaunch -targeted -dbgbaseline 
//Baseline Switches: -nonative 
//Branch:  fbl_ie_script_dev
//Build: 130322-2023
//Arch: X86
//MachineName: MKOLT2
//InstructionSet: SSE4.1
var shouldBailout = false;

var reuseObjects = false;
var PolymorphicFuncObjArr = [];
var PolyFuncArr = [];
function GetPolymorphicFunction()
{ 
	if(PolyFuncArr.length > 1 )
	{
		var myFunc = PolyFuncArr.shift();
		PolyFuncArr.push(myFunc);
		return myFunc;
	}
	else
	{
		return PolyFuncArr[0];
	}
}
function GetObjectwithPolymorphicFunction(){
	if (reuseObjects)
	{
		if(PolymorphicFuncObjArr.length > 1 )
		{
			var myFunc = PolymorphicFuncObjArr.shift();
			PolymorphicFuncObjArr.push(myFunc);
			return myFunc
		}
		else
		{
			return PolymorphicFuncObjArr[0];
		}		
	}	
	else
	{
		var obj = {};
		obj.polyfunc = GetPolymorphicFunction();
		PolymorphicFuncObjArr.push(obj)
		return obj
	}	
};
function InitPolymorphicFunctionArray()
{
    for(var i=0;i<arguments.length;i++)
    {
        PolyFuncArr.push(arguments[i])
    }   
}
;
function test0(){
  function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
  function leaf() { return 100; };
  var obj0 = {};
  var obj1 = {};
  var arrObj0 = {};
  var func0 = function(){
    //Snippet:getter7.ecs  prototype bailout: new function 
    function __getterfoo7(obj1) {
        WScript.Echo(obj1.__getterprop7);
    }
    var __getterCtor7 = function () { }
    var __getterproto7 = {};
    __getterCtor7.prototype = __getterproto7;
    var __getterinst7 = new __getterCtor7();
    var __getterdesc7 = { get: function () { WScript.Echo("getter"); 
                         return _Value }, set: function (x) 
                         { _Value = x; },configurable:true }
    Object.defineProperty(__getterproto7, "__getterprop7", __getterdesc7);
    
    __getterproto7.__getterprop7 = 100;
    
    __getterfoo7(__getterinst7);
    1
    if (shouldBailout)
        Object.defineProperty(__getterproto7, "__getterprop7", { get:function () { WScript.Echo("instance getter7"); return _Value }, set:function (x) { _Value = x; },configurable:true });
    
    __getterfoo7(__getterinst7);
    
  }
  var func1 = function(argArr0,argObj1,argFunc2,argObj3){
    argObj1.prop0 = 1; /**bp:locals();stack()**/
  }
  var func2 = function(argObj4,argFunc5){
  }
  obj0.method0 = func1; 
  arrObj0.method0 = func1; 
  var ary = new Array(10);
  var i8 = new Int8Array(256);
  var i16 = new Int16Array(256);
  var ui8 = new Uint8Array(256);
  var ui32 = new Uint32Array(256);
  var f32 = new Float32Array(256);
  var f64 = new Float64Array(256);
  var cpa8 = WScript.CreateCanvasPixelArray(ui8);;
  var floatary = 1;
  var intfloatary = 1;
  var intary = [4,66,767,-100,0,1213,34,42,55,-123,567,77,-234,88,11,-66];
  var c = 1;
  var e = 1;
  var f = -2;
  var g = 1;
  var h = 1;
  this.prop1 = 1; 
  obj0.prop1 = 1; 
  arrObj0[0] = 1; 
  arrObj0[1] = 1; 
  arrObj0[5] = 1; 
  arrObj0[9] = 1; 
  arrObj0[12] = 1; 
  arrObj0[13] = 1; 
  arrObj0[arrObj0.length-1] = 1; 
  ary[4] = 1; 
  ary[13] = 1; 
  ary[ary.length-1] = 2; 
  function bar0 (){
    arguments[(((obj0.method0.call(obj1 , ary, arrObj0, leaf, obj0) >= 0 ? obj0.method0.call(obj1 , ary, arrObj0, leaf, obj0) : 0)) & 0XF)] = 1;
    arrObj0.prop0 =(intary.unshift(obj0.prop1, this.prop1, arrObj0.prop1, obj1.prop1, arrObj0.prop0, e, this.prop0, g, this.prop1, f, c, h));
    1;
  }
  InitPolymorphicFunctionArray(bar0);;
  var __polyobj = GetObjectwithPolymorphicFunction();;
  bar0(); /**bp:locals();stack();resume('step_over');**/
  var __loopvar1 = 0;
  LABEL0: 
  while(((1 * __polyobj.polyfunc())) && __loopvar1 < 3) {
    __loopvar1++;
    WScript.Echo("f = " + (f|0));
1    //Snippets:stfldtype.ecs
    obj0.v343421 = this.prop0;
    obj0.v343422 = arrObj0.prop0;
    obj0.v343423 = arrObj0.prop0;
    
    WScript.Echo(obj0.v343423);
    WScript.Echo(obj0.v343421);
    WScript.Echo(obj0.v343422);
  }
  a = 1; /**bp:locals();stack();resume('step_out');**/
  WScript.Echo("obj1.prop1 = " + (obj1.prop1|0));
  WScript.Echo("obj1.length = " + (obj1.length|0));
  WScript.Echo("arrObj0.length = " + (arrObj0.length|0));
  WScript.Echo("arrObj0[0] = " + (arrObj0[0]|0));
  WScript.Echo("arrObj0[6] = " + (arrObj0[6]|0));
  WScript.Echo("arrObj0[11] = " + (arrObj0[11]|0));
  WScript.Echo("ary[13] = " + (ary[13]|0));
};

// generate profile
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 

// run code with bailouts enabled
shouldBailout = true;
test0(); 


// This test is to avoid dead store in PreOpt peep in globopt, in scenario like this:
// Bytecode:
//        0010  LdHeapArguments_OneByte R6  R2  R2 
//        0016  Ld_A_OneByte R9  R6 
// IR Builder:
//        s6.var          =  LdHeapArguments  s2.var, s2.var          #0010 
//        s9.var          =  Ld_A           s6.var                    #0016 
// Globopt:
//        s9.var          =  LdHeapArguments  s2.var!, s2.var         #0010 
