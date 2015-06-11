var pcxReader = function() {
    var rleBit = 192;
    var colorTableSize = 256;
    var paletteOffset = -769;
    var colorTablePixels = [];
    var colorTable = [];
    var xSize = 0;
    var ySize = 0;
    
    var exception = { }
    exception.NotSupported = 0;

    var header = { }

    var throwException = function(errorCode) {
        switch(errorCode) {
            case exception.NotSupported:
                throw {
                    name: 'NotSupported',
                    message: 'Only 256 color, version 5, pcx files are currently supported'
                }
            break;
        }        
    }

    var loadFile = function(url) {
        var reader = binaryReader();
        
        var count = 0;
        var processByte = 0;
        var colorByte = 0;
        var paletteIndicator = 0;

        reader.loadFile(url);

        var loadHeader = function() {
            header.manufacturer = reader.readNumber();
            header.version = reader.readNumber();
            header.encoding = reader.readNumber();
            header.bitsPerPixel = reader.readNumber();

            header.xmin = reader.readNumber(2);
            header.ymin = reader.readNumber(2);
            header.xmax = reader.readNumber(2);
            header.ymax = reader.readNumber(2);
            header.hdpi = reader.readNumber(2);
            header.vdpi = reader.readNumber(2);

            // Skip over the reserved header bytes
            reader.seek(49, seekOrigin.current);            

            header.nPlans = reader.readNumber();
            header.bytesPerLine = reader.readNumber(2);
        }();
        
        // After reading the header, skip past it
        reader.seek(128, seekOrigin.begin);        

        if ((header.version === 5) && (header.bitsPerPixel === 8) && 
            (header.encoding === 1) && (header.nPlans === 1)) {

            xSize = header.xmax - header.xmin + 1;
            ySize = header.ymax - header.ymin + 1;
            count = 0;

            while (count < xSize * ySize) {
                // When the process byte is less then 192 then it is an index into the color table.
                // If it greater then 192, then there are (processByte - 192) number of entries
                // in the color table of the *next* byte (colorByte)
                processByte = reader.readNumber();

                if ((processByte & rleBit) === rleBit) {
                    processByte &= 63;
                    colorByte = reader.readNumber();

                    for (var index = 0; index < processByte; index++) {
                        colorTablePixels[count] = colorByte;
                        count++;
                    }
                }
                else {
                    colorTablePixels[count] = processByte;
                    count++;
                }
            }

            // Now read the color table (seek to the correct location in the file)
            reader.seek(paletteOffset, seekOrigin.end);            

            // This should be 12, but sometimes it's not - so we just ignore it
            paletteIndicator = reader.readNumber();

            for (var index = 0; index < colorTableSize; index++) {
                colorTable[index] = {
                    red: reader.readNumber(),
                    green: reader.readNumber(),
                    blue: reader.readNumber()
                };                
            }
        }
        else {
            throwException(exception.NotSupported);
        }
    }

    var drawToCanvas = function(canvas) {
        var context = canvas.getContext('2d');       
        var height = Math.min(ySize, canvas.height);
        var width = Math.min(xSize, canvas.width);
        var outputData = context.createImageData(width, height);

        for (var yIndex = 0; yIndex < height; yIndex++) {
            for(var xIndex = 0; xIndex < width; xIndex++) {
                var outputOffset = yIndex * (width * 4) + (xIndex * 4);
                var pcxOffset = yIndex * xSize + xIndex;
                var pixel = colorTable[colorTablePixels[pcxOffset]];

                outputData.data[outputOffset] = pixel.red;
                outputData.data[outputOffset + 1] = pixel.green;
                outputData.data[outputOffset + 2] = pixel.blue;
                outputData.data[outputOffset + 3] = 255;
            }
        }

        context.putImageData(outputData, 0, 0);
    }

    return {
        loadFile: loadFile,        
        drawToCanvas: drawToCanvas
    }
}