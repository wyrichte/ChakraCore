function write(v) { WScript.Echo(v); }

function foo() {}

var all = [  "0","-1","+Infinity","-0","-Infinity","\'test\'"
          ];

var biops1 = [    
    ["*", "mul" ], ["/", "div"], ["%", "mod"]];                 // 11.5 Multiplicative operators    
   
var biops2= [ ["+", "add" ], ["-", "sub"]]                             // 11.6 Addtitive operators
var biops3 = [  ["<<","lsh" ], [">>","rsh"], [">>>", "rshu"]]          // 11.7 Bitwise shift operators

var biops4= [    ["<", "lt"  ], [">", "gt" ], ["<=", "le"],   [">=", "ge" ]] // 11.8 Relational operators
var biops5= [   ["==","eq"  ], ["!=","ne" ], ["===", "seq"], ["!==","sne"]] // 11.9 Equality operators
var biops6=    [["&", "and" ], ["^", "xor"], ["|", "or"]]                  // 11.10 Binary bitwise operators
var biops7=   [  ["&&","land"], ["||","lor"]    ];                             // 11.11 Binary logical operators   

var biops=[biops1,biops2 ,biops3,biops4,biops5,biops6,biops7];//

var fileSystemObject = new ActiveXObject("Scripting.FileSystemObject");
var WShell = new ActiveXObject("WScript.Shell");

var fp ;

// Generate test files
function GenerateCombinatorialTests() {
	for (var opn=0; opn < biops.length; opn++) {
		//for(var op=0;op<biops[opn].length;op++){
			var filename="noname";
			if(opn===0) filename="Multiplicative";
			if(opn===1) filename="Additive";
			if(opn===2) filename="Shift";
			if(opn===3) filename="Relational";
			if(opn===4) filename="Equality";
			if(opn===5) filename="Bitwise";
			if(opn===6) filename="Logical";
			fp = fileSystemObject.CreateTextFile(filename+".js", true);
			fp.WriteLine("//These are automatically generated Test Cases");
			fp.WriteLine("//Do Not Modify the Test Cases");
			fp.WriteLine();
			fp.WriteLine();
			fp.WriteLine();
			fp.WriteLine();
			fp.WriteLine('function write(v,str) { WScript.Echo(v + "							"+str); }');
			fp.WriteLine("");
			//fp.WriteLine("function foo() {}");
			fp.WriteLine("");
			
			var outerarr=biops[opn];
			for(var len=0;len<outerarr.length;len++){
				var curop=0;
				while(curop<biops.length){
					var innerarr=biops[curop];
					for (var op1=0; op1 < biops[curop].length; op1++){
						for (var i=0; i<all.length; ++i) {
							for (var j=0; j<all.length; ++j) {
						
								//fp.WriteLine(""+opn+""+curop+""+op1);
								fp.WriteLine("write(" +all[i] + " " + outerarr[len][0] + " " + "("+all[j] + "  " +biops[curop][op1][0]+ " " + all[i]+ ")" + "," +"\""+ all[i] + "" + outerarr[len][0] +"" + "("+all[j] + "" +biops[curop][op1][0]+ "" + all[i]+")\""+");");            
							}
						}
				
				
					}
					curop++;
				}
			}
			fp.close();
		//}
	}
}
    
// Generate baseline files
function Generate_CreateBaseLine() {
	fp = fileSystemObject.CreateTextFile("createBaseLine.bat", true);
	for (var op=0; op < biops.length; op++) {
		var filename="noname";
		if(op===0) filename="Multiplicative";
		if(op===1) filename="Additive";
		if(op===2) filename="Shift";
		if(op===3) filename="Relational";
		if(op===4) filename="Equality";
		if(op===5) filename="Bitwise";
		if(op===6) filename="Logical";
		fp.WriteLine("cscript.exe //nologo " +filename+ ".js > " + filename + ".baseline");		
	}
	fp.close();
}

// Generate rlexe.xml files
function Generate_RLexe() {
	fp = fileSystemObject.CreateTextFile("rlexe.xml", true);

	fp.WriteLine('<?xml version="1.0"?>');
	fp.WriteLine('<regress-exe>');
	fp.WriteLine('');

	for (var op=0; op < biops.length; op++) {
		var filename="noname";
		if(op===0) filename="Multiplicative";
		if(op===1) filename="Additive";
		if(op===2) filename="Shift";
		if(op===3) filename="Relational";
		if(op===4) filename="Equality";
		if(op===5) filename="Bitwise";
		if(op===6) filename="Logical";
		fp.WriteLine('<test>');
		fp.WriteLine('    <default>');
		fp.WriteLine('        <files>' +filename + '.js</files>');
		fp.WriteLine('        <baseline>' +filename+ '.baseline</baseline>');
		fp.WriteLine('    </default>');
		fp.WriteLine('</test>');
		fp.WriteLine('');		
	}
	
	fp.WriteLine('</regress-exe>');
	fp.close();
}

GenerateCombinatorialTests();
Generate_CreateBaseLine();
Generate_RLexe();
