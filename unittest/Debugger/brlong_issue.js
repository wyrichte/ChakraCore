
var shouldBailout = false;

var GiantPrintArray = [];
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
function InitPolymorphicFunctionArray(args)
{
    PolyFuncArr = [];
    for(var i=0;i<args.length;i++)
    {
        PolyFuncArr.push(args[i])
    }   
}
;

function getRoundValue(n) {
 if(typeof n === 'number') {	
	if(n % 1 == 0) // int number
		return n % 2147483647;
	else // float number
		return n.toFixed(8);
 }
 return n;
};
function test0(){
  function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
  function leaf() { return 100; };
  var obj0 = {};
  var obj1 = {};
  var litObj0 = {prop1: 3.14159265358979};
  var litObj1 = {prop0: 0, prop1: 1};
  var arrObj0 = {};
  var func0 = function(){
    leaf.call(arrObj0 );
  }
  var func1 = function(){
  }
  var ary = new Array(10);
  var i8 = new Int8Array(256);
  var i16 = new Int16Array(256);
  var i32 = new Int32Array(256);
  var ui8 = new Uint8Array(256);
  var ui16 = new Uint16Array(256);
  var ui32 = new Uint32Array(256);
  var f32 = new Float32Array(256);
  var f64 = new Float64Array(256);
  var uic8 = new Uint8ClampedArray(256);
  var cpa8 = WScript.CreateCanvasPixelArray(ui8);;
  var IntArr0 = [];
  var IntArr1 = new Array(568826772,-7159592080080518144);
  var FloatArr0 = new Array();
  var VarArr0 = new Array(obj0,-4,0,-560401447,-90622820,-2.51065477197977E+17,-714110957,-1.5547952326046E+18);
  var a = 1;
  var c = 1;
  var d = 1;
  var e = 1;
  var f = -2065579470.9;
  var g = -831501672;
  var h = -2147483646;
  arrObj0[0] = -2097367002; 
  arrObj0[1] = 4294967297; 
  arrObj0[2] = -1220647354; 
  arrObj0[4] = 517421221; 
  arrObj0[5] = 8.24390841964209E+18; 
  arrObj0[6] = -962985427; 
  arrObj0[7] = -667991771; 
  arrObj0[8] = 1653568226; 
  arrObj0[9] = -106513595; 
  arrObj0[10] = 619634768; 
  arrObj0[11] = -4.66412551795569E+18; 
  arrObj0[12] = 612985686; 
  arrObj0[13] = 879933668; 
  arrObj0[14] = -128; 
  arrObj0[arrObj0.length-1] = 184256289; 
  arrObj0.length = makeArrayLength(-1801170816);
  ary[1] = 249; 
  ary[2] = -286675453; 
  ary[4] = -8; 
  ary[6] = -2.21575170304938E+18; 
  ary[7] = 50; 
  ary[8] = 1; 
  ary[9] = -134979669; 
  ary[10] = 588773971.1; 
  ary[11] = 352034125; 
  ary[12] = -1566046358.9; 
  ary[13] = 897622452.1; 
  ary[14] = 65536; 
  ary[ary.length-1] = 8.73774516349814E+17; 
  ary.length = makeArrayLength(-1068670240.9);
  this.prop0 = -1577673046.9; 
  this.prop1 = 1; 
  obj0.prop0 = -53; 
  obj0.prop1 = 690380562; 
  obj0.length = makeArrayLength(-987189600);
  obj1.prop0 = -2110463188.9; 
  obj1.prop1 = -680713924; 
  obj1.length = makeArrayLength(1.58085146453184E+18);
  arrObj0.prop0 = -822694650; 
  arrObj0.length = makeArrayLength(444184058);
  IntArr0[0] = 1277694105.1; 
  FloatArr0[10] = 1; 
  FloatArr0[FloatArr0.length] = 2.93516188439493E+17; 
  FloatArr0[2] = 1; 
  FloatArr0[3] = 65535; 
  FloatArr0[7] = 7.06024818649616E+18; 
  FloatArr0[FloatArr0.length] = 406826422; 
  FloatArr0[9] = -2147483647; 
  FloatArr0[0] = -1379500857; 
  FloatArr0[8] = 175; 
  FloatArr0[14] = -1015068476.9; 
  FloatArr0[6] = -944039356; 
  FloatArr0[5] = -256; 
  FloatArr0[1] = -2; 
  FloatArr0[FloatArr0.length] = -366251909; 
  function bar0 (argArr0,argArr1,argFunc2){
  }
  InitPolymorphicFunctionArray(new Array(bar0));;
  obj1 = obj0;
  try {
    // CSE bailout because of adding getter.
    
    var v13684 = 1;
    var propName = "_new" + "prop";
    var v13685 = Object.create(obj1);
    var v13686 = { get: function () { return "Getter"; }};
    
    if(shouldBailout) {
    	Object.defineProperty(v13685, propName, v13686);
    }
    
    
    v13684 = v13685[propName];	
    v13684 = v13685[propName] + obj0.prop0;
    GiantPrintArray.push(v13684);
    // Runs JIT only code
    if(runningJITtedCode)
    {
      arrObj0.prop0=1;
    }
    
    ary0 = arguments; /**bp:locals();resume('step_into');locals();resume('step_out');locals();resume('step_out');locals()**/
  } catch(ex) {
    WScript.Echo(ex.message);
    try {
      var __loopvar3 = 0;
      for(var _strvar0 in ui32 ) {
        if(_strvar0.indexOf('method') != -1) continue;
        if(__loopvar3++ > 3) break;
        ui32[_strvar0] = (VarArr0.shift()); 
      }
      var uniqobj1 = {prop0: ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])};
      try {
        d ^=(IntArr0[(1)] * ((IntArr0[(1)] ? IntArr0[(1)] : this.prop1) + IntArr0[(1)]));
        arrObj0.prop1 ^=cpa8.length;
      } catch(ex) {
        WScript.Echo(ex.message);
        GiantPrintArray.push('this.prop1 = ' + (this.prop1|0));
        arrObj0.prop1 = (-1333898275 instanceof ((typeof Array == 'function' ) ? Array : Object)); 
        obj0.prop0 = i8[((IntArr0[(1)] ? (-1333898275 instanceof ((typeof Array == 'function' ) ? Array : Object)) : (-0 instanceof ((typeof Boolean == 'function' ) ? Boolean : Object)))) & 255]; 
        var v = i16[(((258 in 1) & FloatArr0[(18)])) & 255];
        obj1.prop0 &=(typeof(((FloatArr0.unshift(Math.atan2({prop4: (typeof(obj0.length)  == 'string') , prop3: (FloatArr0.push(('method0' in litObj0), (obj0.length++ ), (IntArr1.pop()), (-753162601 && e), (IntArr1.pop()), func0.call(obj0 ), (1750624035 ? -1946597911.9 : 1285462598.1), (new func1()).prop1 , (b instanceof ((typeof Error == 'function' ) ? Error : Object))))
, prop2: (! (((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)]) === FloatArr0[(18)])), prop1: (((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)]) === FloatArr0[(18)]), prop0: VarArr0[(10)]}, VarArr0[((((uniqobj1.prop0 - ((h === uniqobj1.prop0)||(this.prop0 <= obj0.prop1))) >= 0 ? (uniqobj1.prop0 - ((h === uniqobj1.prop0)||(this.prop0 <= obj0.prop1))) : 0)) & 0XF)]), (++ f), IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)], (((typeof(('method0' in litObj0))  != 'object')  * Math.acos((typeof(v)  == 'undefined') )) * (parseInt("-1", 34) - (uniqobj1.prop0 - ((h === uniqobj1.prop0)||(this.prop0 <= obj0.prop1))))), (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])), ((VarArr0.push((func2.call(uniqobj1 ) * (40991270.1 != obj1.prop0) - (IntArr1.push(i16[(1819059538.1) & 255], (obj1.prop0 | g), func1.call(obj1 ), IntArr0[(1)], -1196931649, (typeof(uniqobj1.prop0)  != 'string') , func0.call(obj0 ), (h != obj1.prop0), (typeof(f)  == null) , parseInt("3140596876844637184"), (- uniqobj1.prop0), IntArr0[(9)]))
), (-211 !== ((new Error('abc')) instanceof ((typeof arrObj0.method0 == 'function' ) ? arrObj0.method0 : Object))), (obj0.prop1 > obj1.length), ((~ 126) % 1986986486), (obj1.length %= 189), (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])), -69386592, IntArr0[((((-387821685 instanceof ((typeof Object == 'function' ) ? Object : Object)) >= 0 ? (-387821685 instanceof ((typeof Object == 'function' ) ? Object : Object)) : 0)) & 0XF)], ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)]), func2.call(obj1 ), ((-289988374 * 2147483647 + 35109763) ? (-125 + -3.74907497476885E+18) : IntArr1[(((-8.69787339996343E+18 >= 0 ? -8.69787339996343E+18 : 0)) & 0XF)]), (typeof((FloatArr0.reverse()))  != 'number') , (typeof(obj1.length)  != 'string') , (ui8[(this.prop0) & 255] ? (IntArr1.reverse()) : ((new RangeError()) instanceof ((typeof Error == 'function' ) ? Error : Object))), ((obj1.prop0 == h)&&(arrObj0.length <= obj0.length))))
 ? (obj1.prop0 = ((new RangeError()) instanceof ((typeof EvalError == 'function' ) ? EvalError : Object))) : uic8[(IntArr1[(((__polyobj.polyfunc.call(arrObj0 , IntArr1, VarArr0, leaf) >= 0 ? __polyobj.polyfunc.call(arrObj0 , IntArr1, VarArr0, leaf) : 0)) & 0XF)]) & 255]), (obj0.prop0 *= IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)]), f32.length, ((FloatArr0[((((obj0.prop0 >>= obj1.length) >= 0 ? (obj0.prop0 >>= obj1.length) : 0)) & 0XF)] <= bar0.call(arrObj0 , IntArr1, FloatArr0, leaf)) * (parseInt(-58) + (IntArr1.unshift((VarArr0.shift()), ((h += -4.31242222209918E+18) >= -656352900), (IntArr1[(4)] * a - (e != uniqobj1.prop0)), (++ arrObj0.prop0), ((obj0.length != b) >= (5.59875957048732E+18 * 1818796230 - -3.51676701433531E+18)))))), (i16[(((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) & 255] ? ((-1.48607989988663E+18 == (e ? 634530158 : this.prop1)) !== ((new Object()) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object))) : (h == e)))) ? ((bar0.call(obj0 , 1, 1, leaf) < (~ (FloatArr0.unshift(Math.atan2({prop4: (typeof(obj0.length)  == 'string') , prop3: (FloatArr0.push(('method0' in litObj0), (obj0.length++ ), (IntArr1.pop()), (-753162601 && e), (IntArr1.pop()), func0.call(obj0 ), (1750624035 ? -1946597911.9 : 1285462598.1), (new func1()).prop1 , (b instanceof ((typeof Error == 'function' ) ? Error : Object))))
, prop2: (! (((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)]) === FloatArr0[(18)])), prop1: (((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)]) === FloatArr0[(18)]), prop0: VarArr0[(10)]}, VarArr0[((((uniqobj1.prop0 - ((h === uniqobj1.prop0)||(this.prop0 <= obj0.prop1))) >= 0 ? (uniqobj1.prop0 - ((h === uniqobj1.prop0)||(this.prop0 <= obj0.prop1))) : 0)) & 0XF)]), (++ f), IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)], (((typeof(('method0' in litObj0))  != 'object')  * Math.acos((typeof(v)  == 'undefined') )) * (parseInt("-1", 34) - (uniqobj1.prop0 - ((h === uniqobj1.prop0)||(this.prop0 <= obj0.prop1))))), (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])), ((VarArr0.push((func2.call(uniqobj1 ) * (40991270.1 != obj1.prop0) - (IntArr1.push(i16[(1819059538.1) & 255], (obj1.prop0 | g), func1.call(obj1 ), IntArr0[(1)], -1196931649, (typeof(uniqobj1.prop0)  != 'string') , func0.call(obj0 ), (h != obj1.prop0), (typeof(f)  == null) , parseInt("3140596876844637184"), (- uniqobj1.prop0), IntArr0[(9)]))
), (-211 !== ((new Error('abc')) instanceof ((typeof arrObj0.method0 == 'function' ) ? arrObj0.method0 : Object))), (obj0.prop1 > obj1.length), ((~ 126) % 1986986486), (obj1.length %= 189), (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])), -69386592, IntArr0[((((-387821685 instanceof ((typeof Object == 'function' ) ? Object : Object)) >= 0 ? (-387821685 instanceof ((typeof Object == 'function' ) ? Object : Object)) : 0)) & 0XF)], ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)]), func2.call(obj1 ), ((-289988374 * 2147483647 + 35109763) ? (-125 + -3.74907497476885E+18) : IntArr1[(((-8.69787339996343E+18 >= 0 ? -8.69787339996343E+18 : 0)) & 0XF)]), (typeof((FloatArr0.reverse()))  != 'number') , (typeof(obj1.length)  != 'string') , (ui8[(this.prop0) & 255] ? (IntArr1.reverse()) : ((new RangeError()) instanceof ((typeof Error == 'function' ) ? Error : Object))), ((obj1.prop0 == h)&&(arrObj0.length <= obj0.length))))
 ? (obj1.prop0 = ((new RangeError()) instanceof ((typeof EvalError == 'function' ) ? EvalError : Object))) : uic8[(IntArr1[(((__polyobj.polyfunc.call(arrObj0 , IntArr1, VarArr0, leaf) >= 0 ? __polyobj.polyfunc.call(arrObj0 , IntArr1, VarArr0, leaf) : 0)) & 0XF)]) & 255]), (obj0.prop0 *= IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)]), f32.length, ((FloatArr0[((((obj0.prop0 >>= obj1.length) >= 0 ? (obj0.prop0 >>= obj1.length) : 0)) & 0XF)] <= bar0.call(arrObj0 , IntArr1, FloatArr0, leaf)) * (parseInt(-58) + (IntArr1.unshift((VarArr0.shift()), ((h += -4.31242222209918E+18) >= -656352900), (IntArr1[(4)] * a - (e != uniqobj1.prop0)), (++ arrObj0.prop0), ((obj0.length != b) >= (5.59875957048732E+18 * 1818796230 - -3.51676701433531E+18)))))), (i16[(((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) & 255] ? ((-1.48607989988663E+18 == (e ? 634530158 : this.prop1)) !== ((new Object()) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object))) : (h == e)))))) % (1 <= parseInt("65536"))) : -2147483648))  == 1) ;
      } finally {
        arrObj0.length = makeArrayLength(-4.93614697643141E+18);
        arrObj0.prop0 = (d != obj1.prop1); 
        obj1.prop1 -=(! 1);
        IntArr0[(15)] = obj1.prop1;
        GiantPrintArray.push('a = ' + (a|0));
      }
      litObj4 = {prop0: arrObj0.length, prop1: (f64[3.66152302192198E+18] ? ((/a/ instanceof ((typeof obj0.method0 == 'function' ) ? obj0.method0 : Object)) * (Function('') instanceof ((typeof Array == 'function' ) ? Array : Object)) - (uniqobj1.prop0 > e)) : IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)]), prop2: (! ((arrObj0.prop1 !== obj0.length)&&(d != arrObj0.prop0))), prop3: (obj0.prop1 == a), prop4: (typeof((((arrObj0.prop1 !== obj0.length)&&(d != arrObj0.prop0)), (__polyobj.polyfunc.call(arrObj0 , 1, 1, leaf) * (typeof(FloatArr0[((((obj1.prop1 & -42) >= 0 ? (obj1.prop1 & -42) : 0)) & 0XF)])  != 1)  - (1 * 18695916 - ((obj1.length === -6.38396647177518E+18) * (obj0.prop1 = arrObj0.prop0) - -532896962.9))), ((h >>= arguments[(16)]) <= arguments[((((obj0.length += argArrObj8[(1)]) >= 0 ? (obj0.length += argArrObj8[(1)]) : 0)) & 0XF)])))  == 1) , prop5: 50252329, prop6: (typeof(obj1.prop1)  != 1) , prop7: (+ obj0.prop1)};
      return 1017264704;
      h <<=(argArr10.reverse());
      arrObj0.length= makeArrayLength(obj1.prop0);
    } catch(ex) {
      WScript.Echo(ex.message);
    } finally {
      if((i16[(1) & 255] < IntArr0[(1)])) {
        f = (((/a/ instanceof ((typeof Array == 'function' ) ? Array : Object)) * ((((new Array()) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) === (((ary.reverse()) | (obj0.prop0 > obj0.prop0)) <= ((i8[(arrObj0.length) & 255] == ((arrObj0.length == obj0.length)&&(obj0.prop0 > obj0.prop0))) == ((-87717708 ? 1 : 927803685.1) === (1 * (-8531002 + 1)))))) - (Function('') instanceof ((typeof EvalError == 'function' ) ? EvalError : Object)))) ? obj0.method0.call(obj0 ) : (a += (IntArr0.unshift((~ func1.call(obj0 )))))); /**bp:locals();resume('continue');**/
        obj0.prop0 =(-- g);
      }
      else {
        this.prop1 = (((ui8[(1) & 255] >= (new func0()).prop1 ) instanceof ((typeof Error == 'function' ) ? Error : Object)) * (((arrObj0.prop0 !== a) ? (f == e) : -78944283.9) + (-197 <= f32[(((arrObj0.prop0 <= obj1.prop0)&&(obj1.prop0 >= e))) & 255]))); 
        arrObj0.prop0 %=cpa8[(i16[(1) & 255]) & 255];
        f = (! obj0.prop0); /**bp:locals()**/
        var fPolyProp = function (o) {
          if (o!==undefined) {
            WScript.Echo(o.prop0 + ' ' + o.prop1 + ' ' + o.prop2);
          }
        }
        fPolyProp(litObj0); 
        fPolyProp(litObj1); 

      }
      try {
        f &=1;
      } catch(ex) {
        WScript.Echo(ex.message);
        obj0.length = makeArrayLength((arrObj0.method0.call(arrObj0 ) * ((obj1.prop0 > obj1.prop1)||(obj1.prop1 >= g)) + ((arrObj0.length > arrObj0.length)||(f !== a))));
        var uniqobj2 = {prop0: (FloatArr0.push(75, i8[(f64.length) & 255]))
, prop1: (func0.call(arrObj0 ) * (((-9.47683968434574E+17 ? (ary[(((1 >= 0 ? 1 : 0)) & 0XF)] * obj0.prop0 + func2.call(obj1 )) : ((b >= this.prop0) * (obj0.prop1 + (! (this.prop0 / (obj1.prop0 == 0 ? 1 : obj1.prop0)))))) instanceof ((typeof Array == 'function' ) ? Array : Object)) - (((__polyobj.polyfunc.call(obj0 , 1, 1, leaf) ? 4.24708855931501E+18 : (1 == (obj0.length >= -204))) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) ? ((IntArr0.unshift(f, ((__polyobj.polyfunc.call(obj0 , 1, 1, leaf) ? 4.24708855931501E+18 : (1 == (obj0.length >= -204))) ? (arrObj0.prop0 == this.prop0) : ui32[(-117) & 255]), (obj0.prop0++ ), ((new EvalError()) instanceof ((typeof Boolean == 'function' ) ? Boolean : Object)), -715436558, d)) === i32[((typeof(ary[(((1 >= 0 ? 1 : 0)) & 0XF)])  == 1) ) & 255]) : IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)]))), prop2: (obj0.prop1 += (c++ )), prop3: FloatArr0[(1)], prop4: func0.call(obj0 ), prop5: -19};
        1 = 1;
        uniqobj2.prop4 = ((arrObj0.prop0 != uniqobj2.prop2)&&(this.prop0 != arrObj0.prop1)); /**bp:locals();resume('continue');locals();resume('step_out');locals();resume('step_over');locals()**/
        obj1.prop1 = f64[((((new EvalError()) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) <= (arrObj0[(1)], (e-- ), ui32[(763919823) & 255], arrObj0[(1)], (typeof(1)  == 'boolean') , parseInt("-1333333333333333", 4)))) & 255]; /**bp:locals();**/
        litObj4 = {prop0: (((-- obj1.prop0) - ((new EvalError()) instanceof ((typeof Boolean == 'function' ) ? Boolean : Object))) * (f32[(bar0.call(obj1 , 1, 1, leaf)) & 255] + (/a/ instanceof ((typeof __polyobj.polyfunc == 'function' ) ? __polyobj.polyfunc : Object)))), prop1: ((d > a) < (++ obj1.prop1)), prop2: FloatArr0[((((((new EvalError()) instanceof ((typeof Boolean == 'function' ) ? Boolean : Object)) <= FloatArr0[(1)]) >= 0 ? (((new EvalError()) instanceof ((typeof Boolean == 'function' ) ? Boolean : Object)) <= FloatArr0[(1)]) : 0)) & 0XF)]};
      } finally {
        obj0 = 1;
        obj0.prop0 = (-510938893 ? (func0() * (IntArr0[((((ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) >= 0 ? (ui32[((Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1))) & 255] <= ((obj0.length >= this.prop0), (d !== obj0.length), (Math.cos(((-105 * (1611640001 - -16)) * (Math.round(1) + (-2.32230046617281E+18, -2147483649, this.prop0, 1, obj0.prop0, -7.08236723936938E+18)))) !== (arrObj0.prop1 != this.prop1)), ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (arrObj0[(1)] % ((new EvalError()) instanceof ((typeof Array == 'function' ) ? Array : Object))), IntArr0[(1)])) : 0)) & 0XF)] > f64.length)) : (obj0.prop0++ )); /**bp:locals();**/
      }
    }
    uic8[_strvar0] = (1 * ((typeof(1)  == 1)  + (this.prop0 = func1.call(arrObj0 )))); 
    e = (typeof(arrObj0.length)  != 1) ; 
    arrObj0 = arrObj0;
  } finally {
  }
  WScript.Echo('arrObj0[13] = ' + (arrObj0[13]|0));
  WScript.Echo('arrObj0[14] = ' + (arrObj0[14]|0));
  WScript.Echo('arrObj0[arrObj0.length-1] = ' + (arrObj0[arrObj0.length-1]|0));
  WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
  WScript.Echo('ary[0] = ' + (ary[0]|0));
  WScript.Echo('ary[1] = ' + (ary[1]|0));
  WScript.Echo('ary[2] = ' + (ary[2]|0));
  WScript.Echo('ary[3] = ' + (ary[3]|0));
  WScript.Echo('ary[4] = ' + (ary[4]|0));
  WScript.Echo('ary[5] = ' + (ary[5]|0));
  WScript.Echo('ary[6] = ' + (ary[6]|0));
  WScript.Echo('ary[7] = ' + (ary[7]|0));
  WScript.Echo('ary[8] = ' + (ary[8]|0));
  WScript.Echo('ary[9] = ' + (ary[9]|0));
  WScript.Echo('ary[10] = ' + (ary[10]|0));
  WScript.Echo('ary[11] = ' + (ary[11]|0));
  WScript.Echo('ary[12] = ' + (ary[12]|0));
  WScript.Echo('ary[13] = ' + (ary[13]|0));
  WScript.Echo('ary[14] = ' + (ary[14]|0));
  WScript.Echo('ary[ary.length-1] = ' + (ary[ary.length-1]|0));
  WScript.Echo('ary.length = ' + (ary.length|0));
  WScript.Echo('sumOfary = ' + getRoundValue(ary.slice(0, 23).reduce(function(prev, curr) {{ return prev + curr; }},0)));
WScript.Echo('subset_of_ary = ' + ary.slice(0, 11).map(getRoundValue));;
  WScript.Echo('sumOfIntArr0 = ' + getRoundValue(IntArr0.slice(0, 23).reduce(function(prev, curr) {{ return prev + curr; }},0)));
WScript.Echo('subset_of_IntArr0 = ' + IntArr0.slice(0, 11).map(getRoundValue));;
  WScript.Echo('sumOfIntArr1 = ' + getRoundValue(IntArr1.slice(0, 23).reduce(function(prev, curr) {{ return prev + curr; }},0)));
WScript.Echo('subset_of_IntArr1 = ' + IntArr1.slice(0, 11).map(getRoundValue));;
  WScript.Echo('sumOfFloatArr0 = ' + getRoundValue(FloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return prev + curr; }},0)));
WScript.Echo('subset_of_FloatArr0 = ' + FloatArr0.slice(0, 11).map(getRoundValue));;
  WScript.Echo('sumOfVarArr0 = ' + getRoundValue(VarArr0.slice(0, 23).reduce(function(prev, curr) {{ return prev + curr; }},0)));
WScript.Echo('subset_of_VarArr0 = ' + VarArr0.slice(0, 11).map(getRoundValue));;
};

// generate profile
test0(); 
// Run Simple JIT
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 


// Baseline output:
// Skipping first 82 lines of output...
// ary[8] = 1
// ary[9] = -134979669
// ary[10] = 588773971
// ary[11] = 352034125
// ary[12] = -1566046358
// ary[13] = 897622452
// ary[14] = 215074048
// ary[ary.length-1] = 0
// ary.length = 100
// sumOfary = -1410129736
// subset_of_ary = ,249,-286675453,,-8,,-1882815966,50,1,-134979669,588773971.10000000
// sumOfIntArr0 = 1277694105.10000000
// subset_of_IntArr0 = 1277694105.10000000
// sumOfIntArr1 = -1924568609
// subset_of_IntArr1 = 568826772,-345911842
// sumOfFloatArr0 = 964618104
// subset_of_FloatArr0 = -1379500857,-2,1,65535,,-256,-944039356,1646863524,175,0,1
// sumOfVarArr0 = 0[object Object]-40-560401447-90622820-251065477197977000-714110957-1554795232604600000
// subset_of_VarArr0 = [object Object],-4,0,-560401447,-90622820,-783794961,-714110957,-1829090061
// 
// 
// Test output:
// Skipping first 53 lines of output...
// ary[10] = 588773971
// ary[11] = 352034125
// ary[12] = -1566046358
// ary[13] = 897622452
// ary[14] = 215074048
// ary[ary.length-1] = 0
// ary.length = 100
// sumOfary = -1410129736
// subset_of_ary = ,249,-286675453,,-8,,-1882815966,50,1,-134979669,588773971.10000000
// sumOfIntArr0 = 1968074667.10000000
// subset_of_IntArr0 = 1277694105.10000000,,,,,,,,,,
// sumOfIntArr1 = -1924568609
// subset_of_IntArr1 = 568826772,-345911842
// sumOfFloatArr0 = 964618104
// subset_of_FloatArr0 = -1379500857,-2,1,65535,,-256,-944039356,1646863524,175,0,1
// sumOfVarArr0 = -1270135167
// subset_of_VarArr0 = -90622820,-783794961,-714110957,-1829090061
// ASSERTION 5960: (inetcore\jscript\lib\runtime\language\diagobjectmodel.cpp, line 1338) pResolvedObject->obj
//  Failure: (pResolvedObject->obj)
