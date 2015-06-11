objArgs = WScript.Arguments;

if( objArgs.length != 4 ) {

	WScript.Echo( "Usage: cscript ESBuiltInsTypeNames.trie.Generator.js <inputFileName> <outputFileName> <stringParameterName> <stringLengthParameterName>" );

} else {

	main( objArgs( 0 ), objArgs( 1 ), objArgs( 2 ), objArgs( 3 ) )
}

function emitToken( token, d, indent ) {

	r = "";
	indent += "    ";
	r += indent + "return ESBuiltInTypeNameId::" + token.tk + ";\r\n";

	return r;
}

function noMoreBranches( token ) {
	for( var c = token; c.length; c = c[0] ) {
		if( c.length > 1 ) return false;
		if( c.length == 1 && c.tk ) return false;
	}
	return true;
}

function emit( token, d, indent, lengthParam ) {
	var r = "";
	if( d < 0 ) throw "d must be gte 0.";

	if( noMoreBranches( token ) ) {

		var count = d;
		for( var c = token; c.length; c = c[0] ) {
			count++;
		}

		r += indent + "if (" + lengthParam + " == " + ( count );
		for( var c = token; c.length; c = c[0] ) {
			r += " && ";
			r += "p[" + d++ + "] == '" + c[0].char + "'";
		}

		r += "){\r\n";
		r += emitToken( c, d, indent );
		r += indent + "}\r\n";

	} else if( token.length >= 1 ) {

		// Get the count of single-child nodes until the next branch.
		var count = 0;
		for( var c = token; c.length; c = c[0] ) {
			if( c.length > 1 ) break;
			if( c.length == 1 && c.tk ) break;
			count++;
		}

		if( count > 0 ) {

			r += indent + "if( " + lengthParam + " >= " + ( count + d ) + "";

			var i = 0;//, d2 = d;
			for( var c = token; c.length && i < count - 1; c = c[0] ) {
				r += " && ";
				r += "p[" + d++ + "] == '" + c[0].char + "'";
				//				r += "p[" + d2++ + "] == '" + c[0].char + "'";
				i++;
			}
			token = c;

			r += " ) {\r\n";

			indent += "    ";
		}

		r += indent + "switch( p[" + d + "] ) {\r\n";
		indent += "    ";

		for( var i = 0; i < token.length; i++ ) {

			var tk = token[i];

			r += indent + "case '" + tk.char + "':\r\n";

			if( tk.tk && tk.length ) {

				r += indent + "    if (" + lengthParam + " == " + ( d + 1 ) + ") {\r\n" + emitToken( tk, d + 1, indent + "    " ) + indent + "    }\r\n";
			}

			r += emit( tk, d + 1, indent + "    ", lengthParam );

			r += indent + "    break;\r\n";
		}

		if( count > 0 ) {
			indent = indent.substring( 4 );
			r += indent + "}\r\n";
		}

		indent = indent.substring( 4 );

		r += indent + "}\r\n";

	}
	else {
		r += indent + "if (p[" + d + "] == '" + token[0].char + "') {\r\n";
		r += emit( token[0], d + 1, indent + "    ", lengthParam );
		r += indent + "}\r\n";
	}
	return r;
}

function isNumeric(obj) {
	return obj - parseFloat(obj) >= 0;
}

function main( inputFileName, outputFileName, stringParameterName, stringLengthParameterName ) {

	var fso = new ActiveXObject( "Scripting.FileSystemObject" );
	var file = fso.OpenTextFile( inputFileName, 1 );
	var text = file.ReadAll();
	file.Close();
	var reg = /\nTYPENAME\((\d),(\w+)\s*\)/g;
	var t = [];
	// Extract the typenames from TYPENAME macros.
	var s = text.replace( reg, function( match, p1, p2, offset ) {
		t.push( { tk: p2, word: p2 } );
	} );

	var tokens = [];
	var counter = 0;

	for( var i = 0; i < t.length; i++ ) {
		var token = t[i];
		var current = tokens;
		for( var j = 0; j < token.word.length; j++ ) {
			
			var theChar = token.word.substring( j, j + 1 );
			var theKey  = theChar;

			if( isNumeric( theChar) ) {
				theKey = "_" + theChar; // don't use numbers as keys directly.
			}

			var n = current[ theKey ];
			if( n )
				current = n;
			else {
				var nt = [];
				nt.char = theChar;
				current[ theKey ] = nt;
				current.push( nt );
				current = nt;
			}
			counter++;
		}
		current.tk = token.tk;
	}


	var indent = "    ";
	var r = "// Copyright Microsoft 2015. All rights reserved.\r\n// GENERATED FILE, DO NOT HAND MODIFIY\r\n// Generated with the following command line: cscript ESBuiltInsTypeNames.trie.Generator.js " + objArgs( 0 ) + " " + objArgs( 1 ) + " " + objArgs( 2 ) + " " + objArgs( 3 ) + "\r\n// This should be regenerated whenever the type-name list changes.\r\n\r\n";

	r += indent + "const wchar_t* p = " + stringParameterName + ";\r\n";
	r += indent + "switch( p[0] ) {\r\n";
	indent += "    ";

	// Generate the trie.
	for( var i = 0; i < tokens.length; i++ ) {
		var tk = tokens[i];
		r += indent + "case '" + tk.char + "':\r\n";
		var simple = tk.length == 1 && noMoreBranches( tk );
		r += emit( tk, 1, indent + "    ", stringLengthParameterName );
		r += indent + "    return ESBuiltInTypeNameId::_None;\r\n";
	}
	r += indent + "}\r\n";
	indent = indent.substring( 4 );
	r += indent + "return ESBuiltInTypeNameId::_None;\r\n";

	var outfile = fso.CreateTextFile( outputFileName, true );
	outfile.Write( r );
	outfile.Close();
}
