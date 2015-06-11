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
function test0(){
  var obj0 = {};
  var obj1 = {};
  var arrObj0 = {};
  var func2 = function(argMath6){
    a = 1; /**bp:stack();**/
  }
  // apply1.ecs
  var v12335 = {
    v12337: function() {
      return function bar() {
        func2.call(obj1 , 1);
        this.method0.apply(this, arguments);
      }
    }
  };
  var v12338 = arrObj0;
  
  v12338.prototype = {
      method0 : function(){
          
          return 1;
      },
      method1 : function(){
          return 1;
      },
      v12339 : function() {
          return 1;
      },
      v12340 : function() {
          
          return 1;
      },
      v12341 : function() {
          return 1;
      },
      v12342 : function() {
          
          return 1;
      }
      };
  v12338.v12339 = v12335.v12337();
  
  v12338.v12339.prototype = {
      v12343 : (-- obj0.length),
      method0 : function(v12343) {
          this.v12343 = v12343;
          this.method0 = function(){
              
              return 1;
          }
          this.method1 = function(){
              return 1;
          }
     }
  };
  
  
  
  v12338.v12340 = v12335.v12337();
  
  v12338.v12340.prototype = {
      v12344 : 1,
      v12345 : 1,
      method0 : function(v12344,v12345) {
          this.v12344 = v12344;
          this.v12345 = v12345;
      },
      method1 : function(){
          
          return 1;
      }
  };
  
  
  v12338.v12341 = v12335.v12337();
  
  v12338.v12341.prototype = {
      v12346 : obj1,
      method0 : function(v12347) {
          this.v12346 = new v12338.v12339(v12347);
          return (a++ );
      }
  };
  
  
  
  v12338.v12342 = v12335.v12337();
  
  v12338.v12342.prototype = {
      v12348 : obj0,
      v12349 : arrObj0,
      method0 : function(v12336,v12350,v12351) {
          this.v12348 = new v12338.v12339(v12336);
          this.v12349 = new v12338.v12339(v12350,v12351);
          return 1;
      }
  };
  
  var v12352 = new v12338.v12339(1);
  var v12353 = new v12338.v12339(1,1);
  
  var v12354 = new v12338.v12339(1,1,1);
  GiantPrintArray.push(v12354.v12343);
  
  
  
  var v12355 = new v12338.v12340(1);
  var v12356 = new v12338.v12340(1,1);
  
  var v12357 = new v12338.v12340(1,1,1);
  GiantPrintArray.push(v12357.v12345);
  
  var v12358 = new v12338.v12341(1);
  var v12359 = new v12338.v12341(1,1);
  var v12360 = new v12338.v12341(1,1,1);
  GiantPrintArray.push(v12360.v12346);
  
  var v12361 = new v12338.v12342(1);
  var v12362 = new v12338.v12342(1,(++ obj0.length));
  
  var v12363 = new v12338.v12342(1,1,1);
  GiantPrintArray.push(v12363.v12348);
  GiantPrintArray.push(v12363.v12349);
};

// generate profile
test0(); 
// Run Simple JIT
test0(); 
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 

WScript.Echo('pass');