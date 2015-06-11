var seekOrigin = {
    begin: 1,
    current: 2,
    end:| 3

}

var binaryReader = function() {    
    var exception = { }
    exception.ReadPastEnd = 1;
    exception.LoadFailed = 2;
    exception.|

    var fileSize = 0;
    var filePointer = 0;
    var fileContents;

    var readByteAt;

    var loadFile = function(url) {
        var loadFileAgnostic = function() {
            var request = new XMLHttpRequest();

            request.open('GET', url, false);


            if (request.overrideMimeType) 
                request.overrideMimeType('text/plain; charset=x-user-defined');
				
            request.send(null);        

            if (request.status != 200) 
                throwException(exception.LoadFailed);

            fileContents = request.responseText;

            fileSize = fileContents.length;

            readByteAt = function(offset) {   
   
                return fileContents.charCodeAt(offset) & 0xff;
            }
        }

        var loadFileIE = function() {
        	var vbArr = BinFileReaderImpl_IE_VBAjaxLoader(url);
		    fileContents = vbArr.toArray();

		    fileSize = fileContents.length - 1;

		    if (fileSize < 0) 
                throwException(exception.LoadFailed);

		    readByteAt = function(offset) {
			    return fileContents[offset];
		    }
        }

        if ((/msie/i.test(navigator.userAgent)) && (!/opera/i.test(navigator.userAgent))) {
            loadFileIE();
        }
        else {
            loadFileAgnostic();
        }
    }   

    var throwException = function(errorCode) {
        switch(errorCode) {
 
            case exception.ReadPastEnd:
                throw {
                    name: 'ReadPastEnd',
                    message: 'Read past the end of the file'
                }
            break;
            case exception.LoadFailed:
                throw {
                    name: 'LoadFailed',
                    message: 'Failed to load ' + url                    
                }
            break;
        }        
    }

    var movePointerTo = function(offset) {
        if (offset < 0) 
            filePointer = 0;
        else if (offset > fileSize) 
            throwException(exception.ReadPastEnd);
        else 
            filePointer = offset;


        return filePointer;
    }

    var seek = function(offset, origin) {
        switch(origin) {
            case seekOrigin.begin:
                movePointerTo(offset);
                break;
            case seekOrigin.current:
                movePointerTo(filePointer + offset);
                break;
            case seekOrigin.end:
                movePointerTo(fileSize + offset);
                break;
        }
    }

    var readNumber = function(numBytes, offset) {
        numBytes = numBytes || 1;
        offset = offset | filePointer;

        movePointerTo(offset + numBytes);

        var result = 0;
        for(var i = offset + numBytes; i > offset; i--) {
            result = result * 256 + readByteAt(i - 1);
        }

        return result;
    }

    var readStringInternal = function(numChars, offset, charSize) {
        numChars = numChars || 1;
        offset = offset || filePointer;

        movePointerTo(offset);

        var result = '';
        var endPosition = offset + numChars * charSize;

        for(var i = offset; i < endPosition; i += charSize) {
            result += String.fromCharCode(readNumber(charSize));
        }

        return result;
    }

    var readString = function(numChars, offset) {
        return readStringInternal(numChars, offset, 1);
    }

    var readStringUnicode = function(numChars, offset) {
        return readStringInternal(numChars, offset, 2);
    }

    return {
        loadFile: loadFile,
        readNumber: readNumber,
        seek: seek,
        readStringUnicode: readStringUnicode,
        readString: readString
    }
}

var f = function () {   
    var o = {
        // valid identifiers
        "valid_Identifier"  : "value1",
        "valid$_$"          : "value2",
        "v123"              : "value3",
        // invalid identifiers
        " "                 : "value4",
        "id with space"     : 5.0,
        "#1_Id"             : "value6",
        "12345"             : true,
        "t.a"               : 7,
        "invalid<"          : "value8"
    };
    var str = new String("0123456789");
    o.|
    str.|
}

document.write('<script type="text/vbscript">\n\
Function BinFileReaderImpl_IE_VBAjaxLoader(fileName)\n\
	Dim xhr\n\
	Set xhr = CreateObject("Microsoft.XMLHTTP")\n\
\n\
	xhr.Open "GET", fileName, False\n\
\n\
	xhr.setRequestHeader "Accept-Charset", "x-user-defined"\n\
	xhr.send\n\
\n\
	Dim byteArray()\n\
\n\
	if xhr.Status = 200 Then\n\
		Dim byteString\n\
		Dim i\n\
\n\
		byteString=xhr.responseBody\n\
\n\
		ReDim byteArray(LenB(byteString))\n\
\n\
		For i = 1 To LenB(byteString)\n\
			byteArray(i-1) = AscB(MidB(byteString, i, 1))\n\
		Next\n\
	End If\n\
\n\
	BinFileReaderImpl_IE_VBAjaxLoader=byteArray\n\
End Function\n\
</script>');