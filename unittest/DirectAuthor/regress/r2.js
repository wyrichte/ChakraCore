var loadFile = function(url) {
    var reader = binaryReader();
        
    var count = 0;
    var processByte = 0;
    var colorByte = 0;
    var paletteIndicator = 0;

    reader.loadFile(url);

    reader.|

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
