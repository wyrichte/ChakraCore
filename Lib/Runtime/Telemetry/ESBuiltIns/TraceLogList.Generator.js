objArgs = WScript.Arguments;

if( objArgs.length != 2 ) {

	WScript.Echo( "Usage: cscript TraceLogList.Generator.js <inputFileName> <outputFileName>" );

} else {

	main( objArgs( 0 ), objArgs( 1 ) )
}


function main( inputFileName, outputFileName ) {

	var fso = new ActiveXObject( "Scripting.FileSystemObject" );
	var file = fso.OpenTextFile( inputFileName, 1 );
	var text = file.ReadAll();
	file.Close();
	var reg = /\nBUILTIN\((\w+),(\w+),(\w+),(\w+),(\w+)\)/g;
	var entries = [];
	// Extract the names from nBUILTIN macros.
	var s = text.replace( reg, function( match, g1, g2, g3, g4, g5, offset ) {
		entries.push( { 
			ctor: g1,
			loca: g2,
			name: g3,
			kind: g4,
			vers: g5
		} );
	} );

	var outfile = fso.CreateTextFile( outputFileName, true );

	outfile.Write( "// Copyright Microsoft 2015. All rights reserved.\r\n// GENERATED FILE, DO NOT HAND MODIFIY\r\n// Generated with the following command line: cscript TraceLogList.Generator.js " + objArgs( 0 ) + " " + objArgs( 1 ) + "\r\n// This should be regenerated whenever the ESBuiltInsDatabase.inc file changes.\r\n\r\n");

	// Unfortunately the C++ compiler will crash with an out-of-heap-space error if the TraceLogChakra macro is evaluated with too many arguments.
	// So we limit it to between 30 to 45 arguments at a time. Feel free to adjust this down if your computer has less memory.

	// Note This script also groups properties by their parent type, in addition to keeping the entriesPerBlock, so the `entriesPerBlock` limit might not necessarily be reached.

	var entriesPerBlock = 30;
	var countInBlock = 0;
	var blockNumber = 0;
	var blockInType = 0;
	var indent = "        ";
	var lastType = "";

	var isFirst = true;
	for(var i = 0; i < entries.length; i++) {
		
		var e = entries[i];

		var isLastInBlock = i == entries.length - 1 || countInBlock == entriesPerBlock - 1;

		if( countInBlock == entriesPerBlock - 1 || e.ctor != lastType ) {
			
			if( lastType != e.ctor ) {
				blockInType = 0;
				lastType = e.ctor;
			} else {
				blockInType++;
			}

			if( i > 0 ) outfile.Write( "\r\n" + indent + ");\r\n\r\n" );
			outfile.Write( indent + "TraceLogChakra(\"ESBuiltIns_" + lastType + "_" + blockInType + "\" /* #" + blockNumber + " */," );

			countInBlock = 0;
			blockNumber++;

			isFirst = true;
		}

		if( !isFirst ) {
			outfile.Write(",");
		}
		isFirst = false;
		outfile.Write("\r\n");

		outfile.Write( indent + "    TraceLoggingInt32( Get( ESBuiltInPropertyId::" );
		outfile.Write( e.ctor );
		outfile.Write( "_" );
		outfile.Write( e.loca );
		outfile.Write( "_" );
		outfile.Write( e.name );

		outfile.Write( " ), \"" );

		outfile.Write( e.ctor );
		outfile.Write( "_" );
		outfile.Write( e.loca );
		outfile.Write( "_" );
		outfile.Write( e.name );

		outfile.Write( "\" )" );

		countInBlock++;
	}

	outfile.Write( "\r\n    );\r\n" )

	outfile.Close();
}
