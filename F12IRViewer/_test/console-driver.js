// ========== DRIVER ==========

Output.WriteLine(" --- console app driver --- ");

/*
var input = '\
  function foo(a,b) {\n\
    var str = "hello world";\n\
    var pi = 3.14;\n\
    var ans = 42;\n\
    return a+b+2;\n\
  }\n\
';
*/

// var input = '\
// \
// function foo (seed) {\n\
    // Robert Jenkins\' 32 bit integer hash function.\n\
    // var x = 3.14;\n\
    // var y = x;\n\
    // var z = x+y;\n\
    // seed = ((seed + 0x7ed55d16) + (seed << 12))  & 0xffffffff;\n\
    // seed = ((seed ^ 0xc761c23c) ^ (seed >>> 19)) & 0xffffffff;\n\
    // seed = ((seed + 0x165667b1) + (seed << 5))   & 0xffffffff;\n\
    // seed = ((seed + 0xd3a2646c) ^ (seed << 9))   & 0xffffffff;\n\
    // seed = ((seed + 0xfd7046c5) + (seed << 3))   & 0xffffffff;\n\
    // seed = ((seed ^ 0xb55a4f09) ^ (seed >>> 16)) & 0xffffffff;\n\
    // return (seed & 0xfffffff) / 0x10000000 + z;\n\
// }\
// \
// ';

var input = 'function foo(aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa) { return aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa; }';

var dumpir = parseIR(input);

// ========== OUTPUT ==========

Output.WriteLine("");
Output.WriteLine("*****************************");
Output.WriteLine("    IRBuilder phase");
Output.WriteLine("*****************************");
ViewIR(dumpir.IRBuilder, dumpir.metadata);

Output.WriteLine("");
Output.WriteLine("*****************************");
Output.WriteLine("    GlobOpt phase");
Output.WriteLine("*****************************");
ViewIR(dumpir.GlobOpt, dumpir.metadata);

Output.WriteLine("");
Output.WriteLine("*****************************");
Output.WriteLine("    Lowerer phase");
Output.WriteLine("*****************************");
ViewIR(dumpir.Lowerer, dumpir.metadata);

Output.WriteLine("");
Output.WriteLine("*****************************");
Output.WriteLine("    RegAlloc phase");
Output.WriteLine("*****************************");
ViewIR(dumpir.RegAlloc, dumpir.metadata);

Output.WriteLine("");
Output.WriteLine("*****************************");
Output.WriteLine("    Encoder phase");
Output.WriteLine("*****************************");
ViewIR(dumpir.Encoder, dumpir.metadata);

// ---

var jsonstr = JSON.stringify(dumpir);
Output.WriteLine(jsonstr);
