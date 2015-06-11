var LexKeywords = [
    "abstract","break","case","catch","class","const",
    "continue","debugger","default","delete","do","else",
    "enum","export","extends","final","finally","for", 
    "function","frances","goto","if","implements","import","in", 
    "instanceof","interface","native","new","package", 
    "private","protected","public","return","static", 
    "super","switch","synchronized","this","throw", 
    "throws","transient","try","typeof","var","void",
    "volatile","while","with"
];

var LexEOL = (-1);

var LexCodeEOF = 0x00;
var LexCodeNWL = 0x0A;
var LexCodeRET = 0x0D;
var LexCodeBSL = '\\'.charCodeAt(0);
var LexCodeSHP = '#'.charCodeAt(0);
var LexCodeBNG = '!'.charCodeAt(0);
var LexCodeQUO = '"'.charCodeAt(0);
var LexCodeAPO = '\''.charCodeAt(0);
var LexCodePCT = '%'.charCodeAt(0);
var LexCodeAMP = '&'.charCodeAt(0);
var LexCodeLPR = '('.charCodeAt(0);
var LexCodeRPR = ')'.charCodeAt(0);
var LexCodePLS = '+'.charCodeAt(0);
var LexCodeMIN = '-'.charCodeAt(0);
var LexCodeMUL = '*'.charCodeAt(0);
var LexCodeSLH = '/'.charCodeAt(0);
var LexCodeXOR = '^'.charCodeAt(0);
var LexCodeCMA = ','.charCodeAt(0);
var LexCodeDOT = '.'.charCodeAt(0);
var LexCodeLT =  '<'.charCodeAt(0);
var LexCodeEQ =  '='.charCodeAt(0);
var LexCodeGT =  '>'.charCodeAt(0);
var LexCodeQUE = '?'.charCodeAt(0);
var LexCodeLBR = '['.charCodeAt(0);
var LexCodeRBR = ']'.charCodeAt(0);
var LexCodeUSC = '_'.charCodeAt(0);
var LexCodeLC  = '{'.charCodeAt(0);
var LexCodeRC  = '}'.charCodeAt(0);
var LexCodeBAR = '|'.charCodeAt(0);
var LexCodeTIL = '~'.charCodeAt(0);
var LexCodeCOL = ':'.charCodeAt(0);
var LexCodeSMC = ';'.charCodeAt(0);
var LexCodeSpace = 32;

var LexKeywordTable;

function LexInitialize() {
    LexKeywordTable=new StringHashTable(StringHashMediumSize);
    for (var i=0;i<LexKeywords.length;i++) {
	LexKeywordTable.add(LexKeywords[i],LexKeywords[i]);
    }
}

// TODO: unicode and '$'

function LexIsIdentifierStartChar(code) {
    return (((code>=65)&&(code<=90))||
	    ((code>=97)&&(code<=122)));
	
}

function LexMatchingOpen(code) {
    if (code==LexCodeRBR)
	return LexCodeLBR;
    else if (code==LexCodeRC)
	return LexCodeLC;
    else if (code==LexCodeRPR)
	return LexCodeLPR;
    else return 0;
}

function LexAdjustIndent(code,indentAmt) {
    if ((code==LexCodeLBR)||(code==LexCodeLC)||(code==LexCodeLPR)) {
	return indentAmt+1;
    }
    else if ((code==LexCodeRBR)||(code==LexCodeRC)||(code==LexCodeRPR)) {
	return indentAmt-1;
    }
    else return indentAmt;
}

function LexIsIdentifierChar(code) {
    return (((code>=65)&&(code<=90))||
	    ((code>=97)&&(code<=122)));
	
}

function LexIsPlainChar(code) {
    return !((LexIsIdentifierChar(code)||(code==LexCodeQUO)||
	      (code==LexCodeAPO)||(code==LexEOL)||(code==LexCodeSLH)));
}

function LexBuildIdentifierOrKeyword(pos,len,lineText) {
    var startPos=pos;
    do {
	var ch=LexEOL;
	if (pos<len)
	    ch=lineText.charCodeAt(pos);
    } while (LexIsIdentifierChar(ch));
}

function LexReadChar(pos,len,lineText) {
    if (pos<len)
	return lineText.charCodeAt(pos);
    else return LexEOL;
}

function LexToken(className,tokenText) {
    this.className=className;
    this.text=tokenText;
}

LexToken.prototype = {
    print: function() { alert(this.className+":"+this.text); }
};

function LexMakeToken(className,tokenText) {
    if ((tokenText=="horse")||(tokenText=="colt")||(tokenText=="stallion")||(tokenText=="filly")||
        (tokenText=="mare")) {
	className="horseStyle";
    }
    if ((tokenText=="cat")||(tokenText=="kitten")||(tokenText=="Charlie")||(tokenText=="kitty")||
        (tokenText=="feline")) {
	className="catStyle";
    }
    if (tokenText=="var") {
	className="varStyle";
    }
    var tok=new LexToken(className,tokenText);
    //tok.print();
    return tok;
}

function LexIsWhitespace(code) {
    return code==32;
}

function LexLine(lineText) {
    var pos=0;
    var startPos=0;
    var len=lineText.length;
    var tokens=new Array();

    while (pos<len) {
	var ch=LexReadChar(pos,len,lineText);
	if (LexIsIdentifierStartChar(ch)) {
	    // identifier or keyword
	    startPos=pos;
	    do {
   		pos++;
		ch=LexReadChar(pos,len,lineText);
	    } while (LexIsIdentifierChar(ch));
	    var idText=lineText.substring(startPos,pos);
	    pos--; 
	    if (LexKeywordTable.lookup(idText)) {
		tokens[tokens.length]=LexMakeToken("keyword",idText);
	    }
	    else tokens[tokens.length]=LexMakeToken("var",idText);
        }
	else if (ch==LexCodeSpace) {
	    startPos=pos;
	    do {
   		pos++;
		ch=LexReadChar(pos,len,lineText);
	    } while (ch==LexCodeSpace);
            tokens[tokens.length]=LexMakeToken("whitespace",lineText.substring(startPos,pos));
	    pos--; 
	}
	else if ((ch==LexCodeSLH)&&((pos+1)<len)&&(lineText.charCodeAt(pos+1)==LexCodeSLH)) {
            tokens[tokens.length]=LexMakeToken("comment",lineText.substring(pos,len));
	    return tokens;
	}
	else if ((ch==LexCodeAPO)||(ch==LexCodeQUO)) {
		startPos=pos;
		var endCode=ch;
		do {
   		    pos++;
		    ch=LexReadChar(pos,len,lineText);
		} while ((ch!=endCode)&&(ch!=LexEOL));
		if (ch==LexEOL) {
                    tokens[tokens.length]=LexMakeToken("stringlit",lineText.substring(startPos,pos));
		    return tokens;
     		}
		else tokens[tokens.length]=LexMakeToken("stringlit",lineText.substring(startPos,pos+1));
	}
	else {
	    // read plain token
	    startPos=pos;
	    do {
   		pos++;
		ch=LexReadChar(pos,len,lineText);
	    } while (LexIsPlainChar(ch));
            tokens[tokens.length]=LexMakeToken("plain",lineText.substring(startPos,pos));
	    pos--;
	}
	pos++;
    }
    return tokens;
}    