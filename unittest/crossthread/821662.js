var scriptVersion = ScriptEngineMajorVersion() + (ScriptEngineMinorVersion()/10);
var a = ScriptEngine();
var b = ScriptEngineBuildVersion();
var globValue = null;

function _Array()
{
    arrayObj0 = new Array();
    arrayObj1 = new Array(10);
    arrayObj2 = new Array(1, 2, 3);

    arrayObj0.reverse();
    arrayObj1.push(1, 2);
    arrayObj1.concat(arrayObj2);
    arrayObj2.join();
    arrayObj2.pop();
    arrayObj2.push();
    arrayObj2.shift();
    arrayObj2.slice(0, 1);
    arrayObj2.sort();
    arrayObj2.unshift(1, 2);
    arrayObj2.valueOf();
    arrayObj2.toLocaleString();
    arrayObj2.toString();

    arrayObj3 = [11233331212333,0.0000000010012344123331222,3e124];
    arrayObj0.splice(0, 1);

    function foo() { return true; }

    if (scriptVersion >= 9.0)   //IE9 or higher
    {
        arrayObj4 = [];
        arrayObj4[0] = 1;
        Array.isArray(arrayObj0);
        arrayObj0.indexOf(1);
        arrayObj0.lastIndexOf(1);
        arrayObj0.every(foo);
        arrayObj0.some(foo);
        arrayObj0.forEach(foo);
        arrayObj0.map(foo);
        arrayObj0.filter(foo);
        arrayObj2.reduce(function(previousValue, currentValue, index, array){ return previousValue + currentValue; });
        arrayObj2.reduceRight(function(previousValue, currentValue, index, array){ return previousValue + currentValue; });
    }
}

function _Boolean()
{
    bool1 = new Boolean();
    bool2 = new Boolean(true);
    bool1.toString();
    bool2.valueOf();
}

function _Date()
{
    dateobj = Date();
    dateObj = new Date()

    dateObj.getMinutes();
    dateObj.getTime();
    dateObj.toUTCString();
    dateObj.getDate(); 
    dateObj.getDay();
    dateObj.getFullYear();
    dateObj.getHours(); 
    dateObj.getMilliseconds();
    dateObj.getMonth();
    dateObj.getSeconds();
    dateObj.getTimezoneOffset();
    dateObj.getUTCDate();
    dateObj.getUTCDay();
    dateObj.getUTCFullYear();
    dateObj.getUTCHours();
    dateObj.getUTCMilliseconds();
    dateObj.getUTCMinutes();
    dateObj.getUTCMonth();
    dateObj.getUTCSeconds();
    dateObj.getVarDate();
    dateObj.getYear();
    dateObj.setDate(223312412331122344);
    dateObj.setFullYear(1970)
    dateObj.setHours(2)
    dateObj.setMilliseconds(2);
    dateObj.setMinutes(2);
    dateObj.setSeconds(2);
    dateObj.setMonth(2);
    dateObj.setTime(2);
    dateObj.setUTCDate(2);
    dateObj.setUTCFullYear(1970)
    dateObj.setUTCHours(2)
    dateObj.setUTCMilliseconds(2);
    dateObj.setUTCMinutes(2);
    dateObj.setUTCSeconds(2);
    dateObj.setUTCMonth(2);
    dateObj.setYear(1970);
    dateObj.toDateString();
    dateObj.toGMTString();
    dateObj.toLocaleDateString();
    dateObj.toLocaleTimeString();
    dateObj.toTimeString();
    dateObj.valueOf();

    var d1 = Date.UTC(2000, 1, 12, 11, 20, 14, 1000);

    var datestring = "November 1, 1997 10:15 AM";
    var d2 = Date.parse(datestring);

    dateObj.toString();
    dateObj.toLocaleString();

    if (scriptVersion >= 9.0)   //IE9 or higher
    {
        Date.now();
        dateObj.toISOString();
        dateObj.toJSON();
    }
}

function _Error()
{
    errorObj = new Error();
    errorObj = new Error(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();

    errorObj = new EvalError();
    errorObj = new EvalError(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();

    errorObj = RangeError("err");
    errorObj = new RangeError();
    errorObj = new RangeError(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();

    errorObj = ReferenceError("err");
    errorObj = new ReferenceError();
    errorObj = new ReferenceError(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();

    errorObj = SyntaxError("err");
    errorObj = new SyntaxError();
    errorObj = new SyntaxError(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();

    errorObj = TypeError("err");
    errorObj = new TypeError();
    errorObj = new TypeError(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();

    errorObj = URIError("err");
    errorObj = new URIError();
    errorObj = new URIError(10, "MyError");
    errorObj.number = 5;
    errorObj.description = "aaaaa";
    errorObj.message = "bbb";
    errorObj.name = "ccc";
    errorObj.toString();
}

function _FunctionObject()
{
    var add1 = new Function("x", "y", "return(x+y)");
    add1(2, 3);
    add1.toString();

    function add()
    {
        return this.a + this.b;
    }	

    function myobj(a, b)
    {
        this.a = a;
        this.b = b;
    }

    var myobj1 = new myobj(2, 5);
    add.call(myobj1);
    add.apply(myobj1);

    add1.valueOf();
    add1.toString();
    add1.length = 5564534522.234343;
    add1.apply();
    add1.call();

    if (scriptVersion >= 9.0)   //IE9 or higher
    {
        // Testing Function.prototype.bind(thisArg)
        var obj = {
          name: "test",
          showName: function() { this.name = "aaa"; }
        };
        var foo = obj.showName.bind(obj);
        foo();
    }
}

function _GlobalObject()
{
    var a = isNaN(100);
    var b = parseFloat("1.2abc");
    var c = parseInt("12abc");
    var d = isFinite(10);
    encoded = encodeURIComponent("http://www.microsoft.com");
    decoded = decodeURIComponent("http://www.microsoft.com");
    encoded = encodeURI("http://www.microsoft.com");
    decoded = decodeURI("http://www.microsoft.com");
    eval("var e = 2;");
}

function _Projection()
{
    WScript.InitializeProjection();
    var uri = new Windows.Foundation.Uri("http://rss.netflix.com/Top100RSS");
}

function _JSON()
{
    var obj = new Object();
    obj.val1 = new String("str");
    obj.val2 = new Boolean(true);
    obj.val3 = new Date();
    obj.val4 = new Number(2343234234556634);

    var funct = function(key,value){if(key == "val1") return "String"; else return value;};

    var repFunct = function(key, value){if (key == "val1") return "str"; else return value;};

    obj.toJSON = function(key)
     {
        var replacement = new Object();
        for (var val in this)
        {
            if (typeof (this[val]) === 'string')
                replacement[val] = this[val].toUpperCase();
            else
                replacement[val] = this[val]
        }
        return replacement;
    };

    var str = JSON.stringify(obj,funct,3);
    var obj1 = JSON.parse(str,repFunct);
}

function _Math()
{
    return Math.abs(2) + Math.max(2, 3, 4) + Math.random() + Math.acos(2) + Math.asin(2) +  Math.atan(2) + Math.atan2(2, 3) +
           Math.ceil(2) + Math.cos(2) + Math.exp(2) + Math.floor(2) + Math.log + Math.max(2, 3, 5, 6) + Math.min(2, 3, 5, 6) +
           Math.pow(2 ,3) + Math.round(3.12323423656345453455) + Math.sin(2) + Math.sqrt(3) + Math.tan(3);
}

function _Number()
{
    num = new Number(10);
    num.toFixed();
    num.toExponential();
    num.toPrecision();
    num.toLocaleString();
    var a = num.toString();
    var b = num.valueOf();
}

function _Object()
{
    obj1 = new Object();
    obj1.toLocaleString();
    obj1.toString();
    obj1.valueOf();
    var a = obj1.isPrototypeOf(obj1);
    var b = obj1.hasOwnProperty("bold");

    obj1 = new Object("String");
    obj1.toLocaleString();
    obj1.toString();
    obj1.valueOf();
    var c = obj1.bold();
    var d = String.prototype.hasOwnProperty("split");

    obj1 = new Object(true);
    obj1 = new Object(1);
    obj2 = {x:5, showX: function() { return this.x; }}
    with (obj2) {
        showX();
    }

    if (scriptVersion >= 9.0)   //IE9 or higher
    {
        var obj = {
          name: "test"
        };

        Object.getPrototypeOf(obj);
        Object.getOwnPropertyNames(obj);
        Object.create({}, { p: { value: 42 } });      //or Object.create(Object.prototype);
        Object.defineProperty(obj, "age", { value: 20 });
        Object.seal(obj);
        Object.freeze(obj);
        Object.preventExtensions(obj);
        Object.isSealed(obj);
        Object.isFrozen(obj);
        Object.isExtensible(obj);
        Object.keys(obj);

        try {
            var obj2 = window.document;
            Object.getOwnPropertyDescriptor(obj2, "name");
        } catch(e) {}
    }
}

function _RegExp()
{
    var s = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPp"
    var r = new RegExp("[A-Z]", "g");
    var re = /[A-Z]/g;
    var a = r.test(s);
    r.compile("[a-z]", "g");
    var b = re.exec(s);
    var c = RegExp.$_;
}

function _SetterGetter()
{
    var o = new Object();
    Object.defineProperty(o,"foo",{set:function(){},get:function(){return "foogetter";} });
    var pdesc = Object.getOwnPropertyDescriptor(o,"foo");
    o.foo=10;
    var a = o.foo;

    if (scriptVersion >= 9.0)   //IE9 or higher
    {
        function foo() {
            function Point() {
                this.x=0;
                this.y=0;
            }

            Point.prototype = {
                print:function() { return "x:"+this.x+", y:"+this.y+", z:"+this.z; }
            };

            var _z;
            Object.defineProperty(Point.prototype,"z",{ set:function(v) { _z=v; }, get: function() { return _z; }});
            var pt=new Point();
            pt.z=12;
            pt.print();
        }

        foo();
    }
}

function _String()
{
    var str1 = new String("ABCDEFGHIJKLM");
    var str2 = "NOPQRSTUVWXYZ";

    var s = str1.concat(str2);
    s.anchor("Anchor1");
    s.big();
    s.blink();
    s.charAt(2);
    s.charCodeAt(2);
    s.fixed();
    s.fontcolor("red");
    s.fontsize(-1);
    String.fromCharCode(112, 108, 97, 105, 110);
    s.italics();
    s.lastIndexOf("a");
    s.link("http://www.microsoft.com");
    s.localeCompare("abc");
    s.match("abc");
    s.replace("abc", "abc");
    s.search("abc");
    s.small();
    s.split("");
    s.strike();
    s.sub();
    s.sup();
    s.toLocaleUpperCase();
    s.toLowerCase();
    s.toUpperCase();
    s.toString(2);
    s.valueOf();
    s.substr(0, 2);
    s.slice(0, 2);
    s.bold();
    s.toLocaleLowerCase();
    s.substring(3, 4);
    s.indexOf("AB");

    if (scriptVersion >= 9.0)   //IE9 or higher
    {
        s.trim();
    }
}

function _Condition()
{
    // Conditional Compilation
    //@cc_on
    var a = @_jscript_version;
    @if (@_win32)
        @set @platform32 = 5
    @elif (@_win16)
        @set @platform32 = false;
    @elif (@_mac)
        @set @oswindows = false
    @else
        @set @oswindows = true
    @end
    
    // if/else condition
    function foo() {
        if (5 < 0)
            return false
        else if (5 > 0)
            return true;
        else
            return 0;
    }
    foo();

    // switch
    function bar(a) {
        switch (a)
        {
            case 0:
                return 0;
            case 1:
                return 1;
            default:
                return -1;
        }
    }
    var a = bar(1);
    var b = bar(100);
}

function _TryCatch()
{
    try {
        a = 5;
    } catch (e) {}
    finally {
        b = 10;
    }
    
    try {
        throw "aaaa";
    } catch (e) {
        try {
            throw "bbbb";
        } catch (e) {
            a = 10;
        } finally {
            b = 20;
        }
    } finally {
        b = 30;
    }
}

function _Loop()
{
    var a = 0;
    for (var i = 0; i < 3; i++)
        a += i;
    
    while (a < 10) {
        a++;
    }
    
    do {
        a--;
    } while (a > 5);
    
    var b = {x:5,y:10};
    for (var c in b)
        a += b[c];
    
    var c = false;
    while (true) {
        if (c) break;
        else c = true;
        continue;
        a = 10;
    }
}

function _ActiveX()
{
    var WScriptObj = new ActiveXObject ("WScript.Shell");
    var WshSysEnv = WScriptObj.Environment("SYSTEM");
}

function _ScriptControl()
{
    var sc = new ActiveXObject("ScriptControl");
	sc.language="jscript";

    var m1 = sc.modules.add("Mod1");
	var m2 = sc.modules.add("Mod2");
	sc.addobject("m1",m1.codeobject);
	sc.addobject("m2",m2.codeobject);

	m1.addcode('var x = "mod1";');
	m2.addcode('var x = "mod2";');	

	sc.reset();
	m1 = sc.modules.add("Mod1");
	m2 = sc.modules.add("Mod2");
	sc.addobject("m1",m1.codeobject);
	sc.addobject("m2",m2.codeobject);
	m1.addcode('var x = "mod1";');
	m2.addcode('var x = "mod2";');

	m1.AddCode( "function test(x,y) { return x + y; };" );
	sum = m1.Run("test",1,2);
	sc.AddCode( "function test(x,y) { return x + y; };" );
	sum = sc.Run("test",2,3);
}

function _TypedArray()
{
    var typedArrays = [ new Int8Array(0),
						new Int8Array(1),
						new Int8Array(127),
						new Int8Array(new ArrayBuffer(129), Math.floor(129 / 2), Math.floor(129 / 4)),
						new Int8Array(1023),
						new Int8Array(new ArrayBuffer(1025), Math.floor(1025 / 2), Math.floor(1025 / 4)),
						new Int8Array(Math.pow(2,5)),
						new Int16Array(0),
						new Int16Array(257),
						new Int16Array(new ArrayBuffer(2)),
						new Float32Array(1025),
						new Float32Array(new ArrayBuffer(4),0,0),
						new Float64Array(0),
						new Float64Array(new ArrayBuffer(1025 * 8), Math.floor(1025/2), Math.floor(1025/8)),
						new Float64Array(1)];
    for (var i = 0; i < typedArrays.length; i++)
    {
        globValue = typedArrays[i];
    }
}

function _Inline()
{
    //Eval
    (function(){
      var obj0 = 1;
      var d = (((88109002.1 + obj0.length) === obj0.b) ? 1 : 1);
      eval("");
    })();
    
    //Setters and Getters 
    var obj = {};
    Object.defineProperty(obj,'foo',{get:function(){return value;},set:function(x){value = x}});
    obj.foo = 10;
    function bar(a, b) {
        for (var i = 0; i < 2; i++) {
            globValue = obj.foo;
        }
    }
    bar(11, 99);
    
    //Bailout
    var foo = function (x, y) {
        return x + y;
    }
    var bar = function (x, y) {
        return x - y;
    }
    function baz() {
        for (var i = 0; i < 2; i++) {
            if (i == 0)
                func = foo;
            else
                func = bar;
            globValue = func(22, 42);
        }
    }
    baz();
}

for (var mainCnt=0; mainCnt<2; mainCnt++)
{
    _Array();
    _Boolean();
    _Date();
    _Error();
    _FunctionObject();
    _GlobalObject();
    //_Projection();
    _JSON();
    _Math();
    _Number();
    _Object();
    _RegExp();
    _SetterGetter();
    _String();
    _Condition();
    _TryCatch();
    _Loop();
    _ActiveX();
    _ScriptControl();
    _TypedArray();
    _Inline();
}

WScript.Echo("Pass");