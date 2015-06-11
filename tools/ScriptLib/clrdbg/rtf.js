/****************************************************************************/ 
/* Helpers to format Rich Text Format (RTF) files.
RTF is an ascii file with tags injected for formatting.
Tags begin with a '\' and end with a space. eg: \tag

Common tags are:
\par - end of line
\b - begin bold
\b0 - end bold
\\, \{ - escaped chars  
\cf? - begin color, ? is color number
\cf0 - end color (return to color 0?)
*/


// Indexes into ColorTable
var RTF_RED   = 1;
var RTF_GREEN = 2;
var RTF_BLUE  = 3;
var RTF_MAX_COLOR = 4;

/****************************************************************************/ 
/* Write header to start off an rtf file.
*/
function rtfWriteHeader(rtfFile)
{
    rtfFile.WriteLine("{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033");

    // Fontable
    rtfFile.WriteLine("{\\fonttbl");
    //rtfFile.WriteLine("{\\f0\\fswiss\\fcharset0 Arial;}");
    rtfFile.WriteLine("{\\f0\\fmodern\\fprq1\\fcharset0 Courier New;}");
    rtfFile.WriteLine("}");
    
    // Color table
    rtfFile.Write("{\\colortbl ;");
    _rtfWriteColorTableEntry(rtfFile, 255,   0,   0); // red
    _rtfWriteColorTableEntry(rtfFile, 0  , 128,   0); // green
    _rtfWriteColorTableEntry(rtfFile, 0  ,   0, 255); // blue  
    rtfFile.WriteLine("}");
    
    rtfFile.WriteLine("{\\*\\generator Msftedit 5.41.21.2500;}\\viewkind4\\uc1\\pard\\f0\\fs20");
}

function _rtfWriteColorTableEntry(rtfFile, r, g, b)
{
    rtfFile.Write("\\red"+r+"\\green"+g + "\\blue" + b +";");
};

/****************************************************************************/ 
/* Write footer at the end of an RTF file.
*/
function rtfWriteFooter(rtfFile)
{
    rtfFile.WriteLine("}");
}

/****************************************************************************/ 
/* Encodes a literal string into an rtf string. 
Practically, this just means escaping chars.
*/
function rtf(szRtfLiteral)
{    
//    szRtfLiteral = szRtfLiteral + "";

    // Must escape the '\' and '{', '}' chars.
    szRtfLiteral = szRtfLiteral.replace(/\\/g, "\\\\");
    szRtfLiteral = szRtfLiteral.replace(/\r/g, "");
    szRtfLiteral = szRtfLiteral.replace(/\n/g, "\\par\r\n");
    szRtfLiteral = szRtfLiteral.replace(/}/g, "\\}");
    szRtfLiteral = szRtfLiteral.replace(/{/g, "\\{");

    return szRtfLiteral;
}

/****************************************************************************/ 
/* Helper to write a single literal string to an rtf file.
*/
function rtfWriteLine(rtfFile, szLiteral)
{
    rtfFile.Write(rtf(szLiteral));
    rtfFile.WriteLine(rtfEOL());    
}

/****************************************************************************/ 
/* Get the rtf End-Of-Line tag
*/
function rtfEOL()
{
    return "\\par";
}

/****************************************************************************/ 
/* Return the data as an rtf string in bold
*/
function rtfBold(szRtf)
{
    return "\\b " + szRtf + "\\b0 ";
}

/****************************************************************************/ 
/* Return the data as an rtf string in the given color.
colorIdx - index into the color table.
*/
function rtfColor(szRtf, colorIdx)
{
    return "\\cf"+colorIdx + " " + szRtf + "\\cf0";
}
