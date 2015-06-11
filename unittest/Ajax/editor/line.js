var LineIndentSpaceCount=4;

function Line(text,tokens,div) {
    this.text=text;
    this.tokens=tokens;
    this.div=div;
    this.indentLevel=0;
}

Line.prototype={
    split:function(pos) {
	var remainderText=this.text.substring(pos,this.text.length);
	this.text=this.text.substring(0,pos);
	this.tokens=LexLine(this.text);
	return remainderText;
    },
    join:function(str) {
	this.text=this.text+str;
	this.tokens=LexLine(this.text);
    },
    insertString:function(pos,str) {
	this.text=this.text.substring(0,pos)+str+this.text.substring(pos,this.text.length);
	this.tokens=LexLine(this.text);
    },
    display:function(cursorPos) {
	if (this.div!=null) {
	    this.div.innerHTML=encodeLine(this,cursorPos);
	}
    },
    setIndentLevel:function (newIndentLevel) {
	// adjust leading whitespace
	var leadingWhitespaceCount=newIndentLevel*LineIndentSpaceCount;
	for (var i=0;i<this.text.length;i++) {
	    if (LexIsWhitespace(this.text.charCodeAt(i))) {
		leadingWhitespaceCount--;
	    }
	    else break;
	}
	if (leadingWhitespaceCount>0) {
	    for (var j=0;j<leadingWhitespaceCount;j++) {
		this.text=" "+this.text;
	    }
	}
	else if (leadingWhitespaceCount<0) {
	    this.text=this.text.substring(-leadingWhitespaceCount,this.text.length);
	}
	this.tokens=LexLine(this.text);
	this.indentLevel=newIndentLevel;
    },
    allWhitespaceTo:function(pos) {
	for (var i=0;i<pos;i++) {
	    if (!LexIsWhitespace(this.text.charCodeAt(i)))
		return false;
	}
	return true;
    },
    hasInitialClose:function() {
	for (var i=0;i<this.text.length;i++) {
	    var code=this.text.charCodeAt(i);
	    if (!LexIsWhitespace(code)) {
		if (LexAdjustIndent(code,0)<0) {
		    return true;
		}
		else return false;
	    }
	}
	return false;
    },
    deletePrev:function(pos) {
	this.text=this.text.substring(0,pos-1)+this.text.substring(pos,this.text.length);
	this.tokens=LexLine(this.text);	
    },
    deleteNext:function(pos) {
	this.text=this.text.substring(0,pos)+this.text.substring(pos+1,this.text.length);
	this.tokens=LexLine(this.text);	
    },
    indentLevelChange:function() {
	var indentAmt=0;
	for (var i=0;i<this.tokens.length;i++) {
	    if (this.tokens[i].className=="plain") {
		var tokenText=this.tokens[i].text;
		for (var j=0;j<tokenText.length;j++) {
		    var code=tokenText.charCodeAt(j);
		    indentAmt=LexAdjustIndent(code,indentAmt);
		}
	    }
	}
	return indentAmt;
    }
};

         
function encodeLine(l,cursorPosition) {
    var stringBuilder="";
    var col=0;
    for (var i=0;i<l.tokens.length;i++) {
	var token=l.tokens[i];
        var endCol=col+token.text.length;
	if (token.text=="rainbow") {
	    stringBuilder+='<span class="bowRed">r</span><span class="bowOrange">a</span><span class="bowYellow">i</span><span class="bowGreen">n</span><span class="bowBlue">b</span><span class="bowIndigo">o</span><span class="bowViolet">w</span>';
	}
	else if (token.text=="henry") {
	    stringBuilder+='<span class="bowRed">h</span><span class="bowOrange">e</span><span class="bowYellow">n</span><span class="bowGreen">r</span><span class="bowBlue">y</span>';	    
	}
	else {
	    var encodedText;
	    stringBuilder+='<span class="';
	    stringBuilder+=token.className;
	    stringBuilder+='">';
	    if ((cursorPosition>=col)&&(cursorPosition<endCol)) {
		var pos=cursorPosition-col;
		var pretext=token.text.substring(0,pos);
		var posttext=token.text.substring(pos+1,token.text.length);
		encodedText=pretext;
		if ((token.className!="var")&&(token.className!="keyword")) {
		    encodedText=encodedText.replace(/ /g,"&nbsp;");
		}
		stringBuilder+=encodedText;
		stringBuilder+='<span id="curpos" class="cursorOn">';
		encodedText=token.text.substring(pos,pos+1);
		if ((token.className!="var")&&(token.className!="keyword")) {
		    encodedText=encodedText.replace(/ /g,"&nbsp;");
		}
		stringBuilder+=encodedText;
		stringBuilder+='</span>';
		encodedText=posttext;
		if ((token.className!="var")&&(token.className!="keyword")) {
		    encodedText=encodedText.replace(/ /g,"&nbsp;");
		}
		stringBuilder+=encodedText;
	    }
	    else {
		encodedText=token.text;
		if ((token.className!="var")&&(token.className!="keyword")) {
		    encodedText=encodedText.replace(/ /g,"&nbsp;");
		}
		stringBuilder+=encodedText;
	    }
	    stringBuilder+='</span>';
	}
       col=endCol;
    }
    if (l.text.length==cursorPosition) {
	stringBuilder+='<span id="curpos" class="cursorOn">&nbsp;</span>';
    }
    else if (l.tokens.length==0) {
	stringBuilder+='<span>&nbsp;</span>';	
    }
    return stringBuilder;
}
