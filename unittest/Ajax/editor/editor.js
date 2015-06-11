function Editor(bod) {
    this.bod=bod;
    this.incrSearch=false;
    this.completing=false;
    this.textarea=document.getElementById("textarea");
    this.textarea.onmouseup = EventMouseup;
    this.textarea.onmousedown = EventMousedown;
    this.textarea.onmousemove = EventMousemove;
    this.status=document.getElementById("status");
    this.highlight=document.getElementById("hlt");
    this.mhighlight=document.getElementById("hltmultiline");

    this.mhighlight.style.display="block";
    this.pathlight=document.getElementById("pathlight");

    var testDiv=document.createElement("div");
    testDiv.className="code1";
    testDiv.innerText="M";
    this.textarea.appendChild(testDiv);
    var rect=testDiv.getBoundingClientRect();
    this.textHeight=rect.bottom-rect.top;
    this.textWidth=rect.right-rect.left;
    this.textarea.removeChild(testDiv);

    this.updateRectangles();
    this.cursorColumn=0;
    this.cursorLine=0;
    this.topLine=0;

    this.maxEventTime=0;
    this.totalEventTime=0;
    this.eventCount=0;
    this.lines=ListMakeHead();
}                                                                          
   
Editor.prototype = {
    testSelect:function() {
	this.completing=true;
	var sel=new Select(["feline","bat","hat","rat","mat"],5);
	var bounds=cursorSpan.getBoundingClientRect();
	var textAreaBounds=this.textarea.getBoundingClientRect();
	var textAreaHeight=textAreaBounds.bottom-textAreaBounds.top;
	if (bounds.bottom>(textAreaHeight/2)) {
	    // TODO: use font height
	    sel.div.style.top=bounds.top-(6*this.textHeight);
	}
	else {
	    sel.div.style.top=bounds.bottom;
	}
	sel.div.style.left=bounds.left;
	this.bod.appendChild(sel.div);
    },
    cancelIncrSearch:function() {
	if (this.incrSearch) {
	    this.incrSearch=false;
	    this.incrSearchUp=false;
	    this.incrSearchWrap="none";
	    this.incrSearchPrefix='';
	    this.bod.removeChild(this.incrSearchDiv);
	}
    },
    startIncrSearch:function(up) {
	if (cursorSpan) {
	    if (!this.incrSearch) {
		this.incrSearch=true;
		this.incrSearchDiv=document.createElement("div");
		this.incrSearchDiv.className="isearch";
		this.incrSearchDiv.innerHTML='<span>ISearch:&nbsp;</span>';
		this.incrSearchWrap="none";
		this.incrSearchPrefix='';
		this.bod.appendChild(this.incrSearchDiv);
		this.moveIncrSearch();
	    }
	    else {
		if (up!=this.incrSearchUp) {
		    this.incrSearchWrap="none";
		}
		this.searchFromCursor(this.incrSearchPrefix,true,up);
	    }
	    this.incrSearchUp=up;
	}
    },
    moveIncrSearch:function() {
	var bounds=cursorSpan.getBoundingClientRect();
	var textAreaBounds=this.textarea.getBoundingClientRect();
	var textAreaHeight=textAreaBounds.bottom-textAreaBounds.top;

	if (bounds.bottom>(textAreaHeight/2)) {
	    // TODO: use font height
	    this.incrSearchDiv.style.top=bounds.top-(this.incrSearchDiv.scrollHeight+12);
	}
	else {
	    this.incrSearchDiv.style.top=bounds.bottom;
	}
	this.incrSearchDiv.style.left=bounds.left;
    },
    findCursorLine:function() {
	var entry=this.lines.next;
	var count=0;
	while (!(entry.isHead)) {
	    if (entry==this.cursorLineEntry)
		break;
	    count++;
	    entry=entry.next;
	}
	this.cursorLine=count;
    },
    searchFromCursor:function(str,skipCurrent,up) {
	var entry;
	var col;
	var wrapped=false;
	if (this.incrSearchWrap=="pending") {
	    wrapped=true;
	    if (up) {
		entry=this.lines.prev;
	    }
	    else entry=this.lines.next;
	    col=0;
	} else {
	    entry=this.cursorLineEntry;
	    col=this.cursorColumn;
	}
	if (!skipCurrent) {
	   col-=str.length;
	}
	var cursorDist=0;
	while (!(entry.isHead)) {
	    var lowerText=entry.data.text.toLowerCase();
	    var lowerStr=str.toLowerCase();
	    var index;
	    if (up) {
		if (entry==this.cursorLineEntry) {
		    var col=this.cursorColumn;
		    if (!skipCurrent) {
			col+=str.length;
		    }
		    lowerText=lowerText.substring(0,col);
		}
		index=lowerText.lastIndexOf(lowerStr);
	    }
	    else {
		index=lowerText.indexOf(lowerStr,col);
		col=0;
	    }
	    if (index>=0) {
		break;
	    }
	    cursorDist++;
	    if (up) {
		entry=entry.prev;
	    }
	    else {
		entry=entry.next;
	    }
	}
	if (index>=0) {
	    this.incrSearchDiv.className="isearch";
	    if (entry!=this.cursorLineEntry) {
		this.cursorLineEntry=entry;
		if (wrapped) {
		    this.findCursorLine();
		}
		else {
		    if (up) {
			this.cursorLine-=cursorDist;
		    }
		    else  {
			this.cursorLine+=cursorDist;			
		    }
		}
		this.wholeScroll(true);
	    }
	    this.cursorColumn=index;
	    if (!up) {
		this.cursorColumn+=str.length;
	    }
	    if (this.cursorColumn>this.cursorLineEntry.data.text.length) {
		this.cursorColumn=0;
	    }
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.moveIncrSearch();
	    if (wrapped) {
		this.incrSearchWrap="wrapped found";
	    }
	    this.updateStatus();
	}
	else {
	    this.incrSearchDiv.className="isearcherr";
	    if (this.incrSearchWrap=="none") {
		this.incrSearchWrap="pending";
	    }
	    else this.incrSearchWrap="wrapped";
	    this.updateStatus();
	}
    },
    incrSearchBksp:function() {
	if (this.incrSearchPrefix.length>0) {
	    this.incrSearchPrefix=this.incrSearchPrefix.substring(0,this.incrSearchPrefix.length-1);
	    this.incrSearchDiv.innerText="ISearch: "+this.incrSearchPrefix;
	    this.incrSearchWrap="none";
	    if (!this.incrSearchUp) {
		this.cursorLeft();
	    }
	    this.updateStatus();
	}
    },
    addToIncrSearch:function(str) {
	this.incrSearchPrefix+=str;
	this.incrSearchDiv.innerText="ISearch: "+this.incrSearchPrefix;
	this.searchFromCursor(this.incrSearchPrefix,false,this.incrSearchUp);
    },
    makeDiv:function() {
	var div=document.createElement("div");
	div.className="code1";
	return div;
    },
    getDiv:function(entry) {
	if (entry.data.div==null) {
	    entry.data.div=this.makeDiv();
	    if (entry==this.cursorLineEntry) {
		cursorPos=this.cursorColumn;
	    }
	    else cursorPos=(-1);
	    entry.data.display(cursorPos);
	}
	return entry.data.div;
    },
    resize:function() {
	// TODO: check cursor in range; set top line and bottom line
	this.prevLinesVisibleCount=this.linesVisibleCount;
	this.updateRectangles();
	this.reDisplay();
    },
    loadTemp:function() {
	var div=this.makeDiv();
	this.textarea.appendChild(div);
	this.topLineEntry=this.addLine(" ",div);
	this.cursorLineEntry=this.topLineEntry;

	div=this.makeDiv();
	this.textarea.appendChild(div);
	this.bottomLineEntry=this.addLine(" ",div);
    },
    load:function(filename) {
	var iolib=window.IOLib;
	
        var file=iolib.open(filename,"r");
	if (file!=null) {
	    this.clearMark();
	    var count=0;
	    var l=iolib.readline(file);
	    var prevEntry=null;
            while (l!=null) {
		var entry=this.addLine(l,null);
		if (count<this.linesVisibleCount) {
		    entry.data.div=this.makeDiv();
		    this.textarea.appendChild(entry.data.div);
		    this.bottomLineEntry=entry;
		}
		if (count==0) {
		    entry.data.indentLevel=0;
		    entry.data.display(0);
		    this.topLineEntry=entry;
		    this.cursorLineEntry=this.topLineEntry;
		}
		else {
		    var indentLevel=0;
		    if (prevEntry!=null) {
			indentLevel=this.nextLineIndent(prevEntry);
		    }
		    if (entry.data.hasInitialClose()) {
			indentLevel--;
		    }
		    entry.data.setIndentLevel(indentLevel);
		    entry.data.display(-1);
		}
		count++;
		l=iolib.readline(file);
		prevEntry=entry;
	    }
	    iolib.close(file);
	}
    },
    setMark:function() {
	if (cursorSpan) {
	    this.mark=cursorSpan.getBoundingClientRect();
	    this.markColumn=this.cursorColumn;
	    this.markLine=this.cursorLine;
	    this.markLineEntry=this.cursorLineEntry;
    	}
	else this.clearMark();
    },
    columnOffsetToColumn:function(x) {
	var col=Math.floor(x/this.textWidth);
	return col;
    },
    lineEntryFromOffset : function (y) {
	var distanceFromTop=y;
	var entry=this.topLineEntry;
	var lineNum=this.topLine;
	while (!entry.isHead) {
	    if (distanceFromTop<this.textHeight)
		break;
	    else {
		distanceFromTop-=this.textHeight;
		lineNum++;
	    }
	    entry=entry.next;
	}
	return {lineEntry:entry,lineNum:lineNum};
    },
    setCursorFromCoordinates:function(x,y,clearIfEq) {
	var pair=this.lineEntryFromOffset(y);
	var entry=pair.lineEntry;
	var lineNum=pair.lineNum;
	if (!entry.isHead) {
	    this.cursorLine=lineNum;
            this.cursorLineEntry.data.display(-1);
	    this.cursorLineEntry=entry;
	    this.cursorColumn=this.columnOffsetToColumn(x);
	    entry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	    if (clearIfEq&&this.mark&&(this.markColumn==this.cursorColumn)&&
		(this.markLine==this.cursorLine)) {
		this.clearMark();
	    }
	}
    },
    clearMark:function() {
        this.mark=undefined;
	this.mhighlight.style.display="none";
    },
    updateRectangles:function() {
	var totalHeight=this.bod.clientHeight-this.bod.clientTop;
	var totalLines=Math.floor(totalHeight/this.textHeight);
	this.linesVisibleCount=totalLines-1;
    
	this.textarea.style.top=0;
	this.textarea.style.left=2;
	this.textarea.style.width=this.bod.clientWidth-2;
	this.textarea.style.height=this.linesVisibleCount*this.textHeight;

	this.mhighlight.style.top=0;
	this.mhighlight.style.left=0;
	this.mhighlight.style.width=this.bod.clientWidth;
	this.mhighlight.style.height=this.linesVisibleCount*this.textHeight;
 
	this.status.style.top=this.linesVisibleCount*this.textHeight;
	this.status.style.height=this.textHeight;
	this.status.style.left=0;
	this.status.style.width=this.bod.clientWidth;

    },
    nextLineIndent:function(entry) {
	var nextIndent=entry.data.indentLevel;
	nextIndent+=entry.data.indentLevelChange();	
	if (entry.data.hasInitialClose()) {
	    nextIndent++;
	}
	return nextIndent;
    },
    // assume startLineEntry <= endLineEntry
    indentLineRange:function(startLineEntry,endLineEntry) {
	if (startLineEntry!=endLineEntry) {
	    var indentLevel=this.nextLineIndent(startLineEntry);
	    var entry=startLineEntry.next;
	    do {
		if (entry.data.hasInitialClose()) {
		    indentLevel--;
		}
		entry.data.setIndentLevel(indentLevel);
		if (entry==this.cursorLineEntry) {
		    this.cursorColumn=this.cursorLineEntry.data.indentLevel*LineIndentSpaceCount;
		    cursorPos=this.cursorColumn;
		}
		else cursorPos=(-1);
		entry.data.display(cursorPos);
		if (entry==endLineEntry)
		    return;
		indentLevel=this.nextLineIndent(entry);
		entry=entry.next;
	    } while (!(entry.isHead));
	}
    },
    gotoLineInner:function(l) {
	var delta=l-this.topLine;
	var entry;
	if (delta>0) {
	    for (entry=this.topLineEntry;(!entry.isHead)&&(delta>0);entry=entry.next) {
		delta--;
	    }
	}
	else if (delta<0) {
	    for (entry=this.topLineEntry;(!entry.isHead)&&(delta<0);entry=entry.prev) {
		delta++;
	    }
	}
	else entry=this.topLineEntry;

	if (entry!=this.cursorLineEntry) {
	    this.cursorLineEntry=entry;
	    this.cursorLine=l;
	    this.wholeScroll(true);
	    this.cursorColumn=0;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	}
    },
    gotoLine:function() {
	var l=Math.floor(prompt("GotoLine: ",this.cursorLine));
	var eventTime=(new Date()).getTime();
	this.gotoLineInner(l);
	return eventTime;
    },
    viewportFull:function() {
      return (this.textarea.childNodes.length>=this.linesVisibleCount);
    },
    updateStatus:function() {
	//var avg=this.totalEventTime/this.eventCount;
	//this.status.innerText="Line: "+this.cursorLine+" Max: "+this.maxEventTime+" Avg: "+avg;
	var statusText="Line: "+this.cursorLine;
	if (this.incrSearch) {
	    statusText+=(" Searching for: "+this.incrSearchPrefix);
	    if (this.incrSearchWrap=="pending") {
		statusText+=" (not found)";
	    }
	    else if (this.incrSearchWrap=="wrapped") {
		statusText+=" (not found; wrapped)";		
	    }
	    else if (this.incrSearchWrap=="wrapped found") {
		statusText+=" (wrapped)";				
	    }
	    
	}
	this.status.innerText=statusText;
    },
    addLine:function(lineText,div) {
        var line=new Line(lineText,LexLine(lineText),div);
	var lineEntry=ListAdd(this.lines,line);
	return lineEntry;
    },
    // this is used for resize and initial display
    reDisplay:function() {
	this.clearMark();
	var entry=this.topLineEntry;
	var originalBottomLineEntry=this.bottomLineEntry;
	var cursorPos;
	var count=0;

	if (this.prevLinesVisibleCount==this.linesVisibleCount) {
	    return;
	}
	else if (this.prevLinesVisibleCount<this.linesVisibleCount) {
	    // all the lines will fit
	    var appendDiv=false;
	    while (!(entry.isHead)&&(count<this.linesVisibleCount)) {
		if (entry==this.cursorLineEntry)
		    cursorPos=this.cursorColumn;
		else cursorPos=(-1);
		entry.data.display(cursorPos);
		if (appendDiv)
		    this.textarea.appendChild(this.getDiv(entry));
		this.bottomLineEntry=entry;
		if (entry==originalBottomLineEntry)
		    appendDiv=true;
		count++;
		entry=entry.next;
	    }
	}
	else {
	    // trim some lines; cursor may be reset
	    while ((!(entry.isHead))&&(count<this.linesVisibleCount)) {
		if (entry==this.cursorLineEntry)
		    cursorPos=this.cursorColumn;
		else cursorPos=(-1);
		entry.data.display(cursorPos);
		this.bottomLineEntry=entry;
		count++;
		entry=entry.next;
	    }
	    while (!(entry.isHead)) {
		// these lines don't fit
		if (entry==this.cursorLineEntry) {
		    // move the cursor to the top if it didn't fit
		    this.cursorColumn=0;
		    this.cursorLineEntry=this.topLineEntry;
		    this.topLineEntry.data.display(0);
		    this.cursorLine-=count;
                    this.updateStatus();
		}
		this.textarea.removeChild(entry.data.div);
		entry.data.div=null;
		count++;
		if (entry==originalBottomLineEntry)
		    return;
		entry=entry.next;
	    }
	}
    },
    beginningOfLine:function() {
	this.cursorColumn=0;
	this.cursorLineEntry.data.display(this.cursorColumn);
	cursorSpan = document.getElementById("curpos");
	this.updateHighlight();
    },
    endOfLine:function() {
	this.cursorColumn=this.cursorLineEntry.data.text.length;
	this.cursorLineEntry.data.display(this.cursorColumn);
	cursorSpan = document.getElementById("curpos");
	this.updateHighlight();
    },
    updateHighlight:function() {
	if (this.mark) {
	    if (cursorSpan) {
		var taBounds=this.textarea.getBoundingClientRect();
		this.mhighlight.coordsize=(taBounds.right-taBounds.left)+","+
		(taBounds.bottom-taBounds.top);
		this.mhighlight.coordorigin=taBounds.left+","+taBounds.top;
		var dot=cursorSpan.getBoundingClientRect();
		var height=dot.bottom-dot.top;
		var pathString="";
		var xOffset=2;
		this.mhighlight.style.display="block";
		if (this.cursorLineEntry==this.markLineEntry) {
		    var a;
		    var b;

		    if (dot.left<this.mark.left) {
			a=dot.left;
			b=this.mark.left;
		    }
		    else {
			a=this.mark.left;
			b=dot.left;
		    }
		    if (a!=b) {
			pathString+=('m '+a+","+dot.bottom+" ");
			pathString+=('l '+a+","+dot.top+", ");
			pathString+=(b+","+dot.top+", ");
			pathString+=(b+","+dot.bottom+" x e");
		    }
		    else {
			this.mhighlight.style.display="none";
		    }
		}
		else {
		    var startLeft;
		    var endLeft;
		    var startTop;
		    var startBottom;
		    var startLineEntry;
		    var endLineEntry;

		    if (dot.top<=this.mark.top) {
			startTop=dot.top;
			startLeft=dot.left;
			startBottom=dot.bottom;
			endLeft=this.mark.left;
			startLineEntry=this.cursorLineEntry;
			endLineEntry=this.markLineEntry;
		    }
		    else {
			startTop=this.mark.top;
			startLeft=this.mark.left;
			startBottom=this.mark.bottom;
			endLeft=dot.left;
			startLineEntry=this.markLineEntry;
			endLineEntry=this.cursorLineEntry;
		    }
		    var entry=startLineEntry;
		    var bounds;
		    startLeft+=xOffset;
		    endLeft+=xOffset;
		    do {
			if (entry.data.tokens.length==0) {
			    bounds=entry.data.div.getBoundingClientRect();
			    bounds.right=this.bod.clientLeft+dot.right-dot.left+xOffset;
			}
			else {
			    if ((entry==this.cursorLineEntry)&&(this.cursorColumn==entry.data.text.length)) {
				if (entry.data.div.lastChild.previousSibling!=null) {
				    bounds=entry.data.div.lastChild.previousSibling.getBoundingClientRect();				    
				}
				else bounds=entry.data.div.lastChild.getBoundingClientRect();
			    }
			    else bounds=entry.data.div.lastChild.getBoundingClientRect();
			}
			if (entry==startLineEntry) {
			    pathString+=('m '+startLeft+","+bounds.bottom+" ");
			    pathString+=("l "+startLeft+","+bounds.top+", ");
			    pathString+=((bounds.right+xOffset)+","+bounds.top+", ");
			    pathString+=((bounds.right+xOffset)+","+bounds.bottom+", ");
			}
			else {
			    pathString+=((bounds.right+xOffset)+","+bounds.top+" ");
			    pathString+=((bounds.right+xOffset)+","+bounds.bottom+" ");	
		    
			}
			entry=entry.next;
		    }
		    while (entry!=endLineEntry);

                    bounds=entry.data.div.getBoundingClientRect();
		    pathString+=(endLeft+","+bounds.top+", ");
		    pathString+=(endLeft+","+bounds.bottom+", ");
		    pathString+=(6+","+bounds.bottom+", ");
		    pathString+=(6+","+startBottom+" x e");
		}
		this.pathlight.v=pathString;
	    }
	}
    },
    // temporary highlight as for a matching open brace
    tempHighlight:function(entry,col) {
	var tempDiv=document.createElement("div");
	tempDiv.id="tmphlt";
	tempDiv.className="temphlt";
	var bounds=entry.data.div.getBoundingClientRect();
	tempDiv.style.top=bounds.top;
	tempDiv.style.left=bounds.left+(this.textWidth*col);
	tempDiv.style.height=this.textHeight;
	tempDiv.style.width=this.textWidth;
	this.bod.appendChild(tempDiv);
	setTimeout("removeTempHighlight()",1200);
    },
    // assumes endLineEntry.data.text.charCodeAt(endColumn) is a close
    findMatchingOpen:function(code,endLineEntry,endColumn) {
	var count=0;
	var openCode=LexMatchingOpen(code);
	var right=endColumn;
	var entry=endLineEntry;
	while (!(entry.isHead)) {
	    var text=entry.data.text;
	    for (var k=right;k>=0;k--) {
		var ch=text.charCodeAt(k);
		if (ch==openCode)
		    count--;
		else if (ch==code)
		    count++;
		if (count==0) {
		    return { lineEntry:entry, col:k };
		}
	    }
	    entry=entry.prev;
	    if (!(entry.isHead)) {
		right=entry.data.text.length-1;
	    }
	}
	return { lineEntry:null, col:0 };
    },
    insertChar:function(str) {
	if (this.incrSearch) {
	    this.addToIncrSearch(str);
	}
	else {
	    this.cursorLineEntry.data.insertString(this.cursorColumn,str);
	    var code=str.charCodeAt(0);
	    var indentChange=LexAdjustIndent(code,0);
	    if (indentChange<0) {
		var text=this.cursorLineEntry.data.text;
		var lineCol=this.findMatchingOpen(code,this.cursorLineEntry,this.cursorColumn);
		var startLineEntry=lineCol.lineEntry;
		var col=lineCol.col;
		this.tempHighlight(startLineEntry,col);
		if ((startLineEntry!=null)&&(startLineEntry!=this.cursorLineEntry)) {
		    if (this.cursorLineEntry.data.allWhitespaceTo(this.cursorColumn)) {
			this.indentLineRange(startLineEntry,this.cursorLineEntry);
			this.cursorColumn=this.cursorLineEntry.data.indentLevel*LineIndentSpaceCount;	    
		    }
		}
	    }
	    this.cursorColumn++;
	    if (code==LexCodeSMC) {
		this.indentCursorLine(true);
	    }
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateHighlight();
	}
    },
    cutRegion:function(otherLineEntry,otherColumn,otherLine) {
	if (this.cursorLineEntry==otherLineEntry) {
	    var a;
	    var b;

	    if (this.cursorColumn<=otherColumn) {
		a=this.cursorColumn;
		b=otherColumn;
	    }
	    else {
		a=otherColumn
		b=this.cursorColumn;
		this.cursorColumn-=(b-a);
	    }
	    if (a!=b) {
		var line=this.cursorLineEntry.data;
		var clippedText=line.text.substring(a,b)
		this.clipped=[clippedText];
		line.text=line.text.substring(0,a)+line.text.substring(b,line.text.length);
		line.tokens=LexLine(line.text);
		line.display(this.cursorColumn);
		cursorSpan = document.getElementById("curpos");		
	    }
	}
	else {
	    // multiline cut region
	    var startColumn
	    var endColumn;
	    var startLineEntry;
	    var endLineEntry;

	    if (this.cursorLine>otherLine) {
		startColumn=otherColumn;
		endColumn=this.cursorColumn;
		startLineEntry=otherLineEntry;
		endLineEntry=this.cursorLineEntry;
	    }
	    else {
		endColumn=otherColumn;
		startColumn=this.cursorColumn;
		endLineEntry=otherLineEntry;
		startLineEntry=this.cursorLineEntry;
	    }

	    // first line
	    var line=startLineEntry.data;
	    var clippedText=line.text.substring(startColumn,line.text.length)
	    this.clipped=[clippedText];
	    line.text=line.text.substring(0,startColumn);

	    // middle lines
	    var entry=startLineEntry.next;
	    var count=0;
	    while (entry!=endLineEntry) {
		var nextEntry=entry.next;
		this.clipped[this.clipped.length]=entry.data.text;
		this.textarea.removeChild(entry.data.div);
		entry.data.div=null;
		ListRemoveEntry(entry);
		count++;
		entry=nextEntry;
	    } 
	    
	    // last line
	    line=entry.data;
	    clippedText=line.text.substring(0,endColumn);
	    this.clipped[this.clipped.length]=clippedText;
	    this.textarea.removeChild(entry.data.div);
	    entry.data.div=null;
	    ListRemoveEntry(entry);
	    count++;
	    var remainderText=line.text.substring(endColumn,line.text.length);

	    line=startLineEntry.data;
	    line.join(remainderText);
	    this.cursorLineEntry=startLineEntry;
	    this.cursorColumn=startColumn;
	    line.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    if (this.cursorLine>otherLine) {
		this.cursorLine=otherLine;
	    }
	    if (count>0) {
		this.scrollUp(count,false);
	    }
	    this.updateStatus();
	}
    },
    paste:function() {
	if (this.clipped) {
	    if (this.clipped.length==1) {
		this.cursorLineEntry.data.insertString(this.cursorColumn,this.clipped[0]);
		this.cursorColumn+=this.clipped[0].length;
		this.cursorLineEntry.data.display(this.cursorColumn);
		cursorSpan = document.getElementById("curpos");
		this.updateHighlight();
	    }
	    else {
		// save starting point for formatting
		var startLineEntry=this.cursorLineEntry;
		if (startLineEntry.prev.isHead) {
		    startLineEntry.indentLevel=0;
		}
		else startLineEntry=startLineEntry.prev;

		// first line
		var line=this.cursorLineEntry.data;
		var precursor=line.text.substring(0,this.cursorColumn);
		var postcursor=line.text.substring(this.cursorColumn,line.length);
		line.text=precursor+this.clipped[0];
		line.tokens=LexLine(line.text);
		line.display(-1);
		var count=0;
		var nextEntry=this.cursorLineEntry.next;

		var div;
		var lastEntry;
		// remaining lines
		for (var i=1;i<this.clipped.length;i++) {
		    div=this.makeDiv();
		    var lineText=this.clipped[i];
		    var newLine=new Line(lineText,LexLine(lineText),div);
		    newLine.display(-1);
		    lastEntry=ListInsertBefore(nextEntry,newLine);
		    count++;
		}

		// last line adjustment

		var lastLine=lastEntry.data;
		this.cursorLineEntry=lastEntry;
		this.cursorLine+=count;
		this.cursorColumn=lastLine.text.length;
		lastLine.text+=postcursor;
		this.indentLineRange(startLineEntry,this.cursorLineEntry);
		lastLine.tokens=LexLine(lastLine.text);
		this.wholeScroll(false);
		lastLine.display(this.cursorColumn);
		cursorSpan = document.getElementById("curpos");
		this.updateStatus();
	    }
	}
    },
    cut:function() {
	if (this.mark) {
	    if (cursorSpan) {
		this.cutRegion(this.markLineEntry,this.markColumn,this.markLine);
		this.clearMark();
	    }
	}
    },
    // this assumes that lines have been inserted without 
    // corresponding divs, so we need to reset viewport
    // cursorLineEntry will be visible after the scroll
    wholeScroll:function(displayRequired) {
	var cursorDistance=this.cursorLine-this.topLine;
	var entry;
	var count=0;
	// TODO: reclaim divs corresponding to these lines
	var child=this.textarea.firstChild;
	while (child!=null) {
	    var nextChild=child.nextSibling;
	    this.textarea.removeChild(child);
	    child=nextChild;
	}
	if ((cursorDistance>=this.linesVisibleCount)||(cursorDistance<0)) {
	    // place cursor as close to the middle as possible
	    var down=true;
	    if (this.cursorLine<this.linesVisibleCount) {
		this.topLineEntry=this.lines.next;
		this.topLine=0;
		entry=this.topLineEntry;
	    }
	    else {
		this.topLine=1+this.cursorLine-this.linesVisibleCount;		
		this.bottomLineEntry=this.cursorLineEntry;
		down=false;
		entry=this.bottomLineEntry;
	    }
	    var nextDiv=null;

	    while (!(entry.isHead)&&(count<this.linesVisibleCount)) {
		if (displayRequired) {
		    if (entry==this.cursorLineEntry)
			cursorPos=this.cursorColumn;
		    else cursorPos=(-1);
		    entry.data.display(cursorPos);
		}
		count++;
		if (down) {
		    this.textarea.appendChild(this.getDiv(entry));		    
		    this.bottomLineEntry=entry;		    
		    entry=entry.next;
		}
		else {
		    if (nextDiv!=null)
			this.textarea.insertBefore(this.getDiv(entry),nextDiv);
		    else this.textarea.appendChild(this.getDiv(entry));
		    nextDiv=entry.data.div;
		    this.topLineEntry=entry;		    
		    entry=entry.prev;

		}
	    }
	}
	else  {
	    entry=this.topLineEntry;
	    count=0;
	    while (!(entry.isHead)&&(count<this.linesVisibleCount)) {
		if (displayRequired) {
		    if (entry==this.cursorLineEntry)
			cursorPos=this.cursorColumn;
		    else cursorPos=(-1);
		    entry.data.display(cursorPos);
		}
		this.textarea.appendChild(this.getDiv(entry));
		this.bottomLineEntry=entry;
		count++;
		entry=entry.next;
	    }
	}
    },
    deleteNext:function() {
	if (this.cursorColumn<this.cursorLineEntry.data.text.length) {
	    this.cursorLineEntry.data.deleteNext(this.cursorColumn);	    
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateHighlight();
	}
	else {
	    this.deleteLineBreak();
	}
    },
    deletePrev:function() {
	if (this.incrSearch) {
	    this.incrSearchBksp();
	}
	else {
	    if (this.cursorColumn>0) {
		this.cursorLineEntry.data.deletePrev(this.cursorColumn);
		this.cursorColumn--;
		this.cursorLineEntry.data.display(this.cursorColumn);
		cursorSpan = document.getElementById("curpos");
		this.updateHighlight();
	    }
	    else {
		if (this.cursorUp(true)) {
		    this.deleteLineBreak();
		}
	    }
	}
    },
    cursorLeft:function() {
	if (this.cursorColumn>0) {
	    this.cursorColumn--;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateHighlight();
	}
    },
    cursorRight:function() {
	if (this.cursorColumn<this.cursorLineEntry.data.text.length) {
	    this.cursorColumn++;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateHighlight();
	}
    },
    deleteToEOL:function() {
	this.clearMark();
	this.cutRegion(this.cursorLineEntry,this.cursorLineEntry.data.text.length,this.cursorLine);
    },
    // TODO: scroll document of length 0 or 1 line
    scrollUp:function(lineCount,topUp) {
        for (var i=0;i<lineCount;i++) {
	    var nextBottomLineEntry=this.bottomLineEntry.next;
	    if (nextBottomLineEntry.isHead) {
		return;
	    }
	    this.bottomLineEntry=nextBottomLineEntry;
	    this.textarea.appendChild(this.getDiv(this.bottomLineEntry));
	    if (topUp) {
		this.textarea.removeChild(this.topLineEntry.data.div);
		this.topLineEntry.data.div=null;
		this.topLineEntry=this.topLineEntry.next;
		this.topLine++;
	    }
	}
    },
    scrollDown:function(lineCount,topDown) {
        for (var i=0;i<lineCount;i++) {
	    if (topDown) {
		var afterDiv=this.topLineEntry.data.div;
		var prevTopLineEntry=this.topLineEntry.prev;
		if (prevTopLineEntry.isHead) {
		    return;
		}
		this.topLineEntry=prevTopLineEntry;
		this.topLine--;
		this.textarea.insertBefore(this.getDiv(this.topLineEntry),afterDiv);
	    }
	    this.textarea.removeChild(this.bottomLineEntry.data.div);
	    this.bottomLineEntry.data.div=null;
	    this.bottomLineEntry=this.bottomLineEntry.prev;
	}
    },
    scrollUpWCursor:function(lineCount,topUp) {
	var cursorAtTop=false;
	if ((this.cursorLine-lineCount)<this.topLine) {
	    this.clearMark();
	    cursorAtTop=true;
	    this.cursorLineEntry.data.display(-1);	    
	}
	else if (this.mark&&(this.markLine-lineCount<this.topLine)) {
	    this.clearMark();
	}
	else if (this.mark) {
	    this.mark.top-=this.textHeight;
	    this.mark.bottom-=this.textHeight;
	}
	this.scrollUp(lineCount,topUp);
	if (cursorAtTop) {
	    this.cursorLineEntry=this.topLineEntry;
	    this.cursorLine=this.topLine;
	    this.cursorColumn=0;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	}
	else {
	    this.updateHighlight();
	}
	
    },
    scrollDownWCursor:function(lineCount,topDown) {
	var cursorAtBottom=false;
	var bottomLine=this.topLine+this.linesVisibleCount-1;
	if ((this.cursorLine+lineCount)>bottomLine) {
	    this.clearMark();
	    cursorAtBottom=true;
	    this.cursorLineEntry.data.display(-1);	    
	}
	else if (this.mark&&(this.markLine+lineCount)>bottomLine) {
	    this.clearMark();
	}
	else if (this.mark) {
	    this.mark.top+=this.textHeight;
	    this.mark.bottom+=this.textHeight;
	}
	this.scrollDown(lineCount,topDown);
	if (cursorAtBottom) {
	    this.cursorLineEntry=this.bottomLineEntry;
	    this.cursorLine=this.topLine+this.linesVisibleCount-1;
	    this.cursorColumn=0;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	}
	else {
	    this.updateHighlight();
	}
    },
    home:function() {
	if (!(this.topLineEntry.prev.isHead)) {
	    this.clearMark();
	    this.cursorLineEntry.data.display(-1);
	    this.topLineEntry=this.lines.next;
	    this.topLine=0;
	    this.cursorLineEntry=this.topLineEntry;
	    this.cursorLine=0;
	    this.wholeScroll(false);
	    this.cursorColumn=0;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	}	
    },
    scrollPageDown:function() {
	if (!(this.topLineEntry.prev.isHead)) {
	    this.clearMark();
	    this.cursorLineEntry.data.display(-1);
	    this.scrollDown(this.linesVisibleCount,true);
	    this.cursorColumn=0;
	    this.cursorLine=this.topLine+this.linesVisibleCount-1;
	    this.cursorLineEntry=this.bottomLineEntry;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	}
    },
    scrollPageUp:function() {
	if (!(this.bottomLineEntry.next.isHead)) {
	    this.clearMark();
	    this.cursorLineEntry.data.display(-1);
	    this.scrollUp(this.linesVisibleCount,true);
	    this.cursorColumn=0;
	    this.cursorLine=this.topLine;
	    this.cursorLineEntry=this.topLineEntry;
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    this.updateStatus();
	}
    },
    cursorUp:function(eol) {
	if (this.cursorLineEntry==this.topLineEntry) {
	    if (this.topLineEntry.prev.isHead) {
		return false;
	    }
	    else this.scrollDown(this.linesVisibleCount>>1,true);
	}
	var formerLine=this.cursorLineEntry.data;
	this.cursorLineEntry=this.cursorLineEntry.prev;
	this.cursorLine--;
	this.updateStatus();
	if (this.cursorColumn>this.cursorLineEntry.data.text.length||eol) {
	    this.cursorColumn=this.cursorLineEntry.data.text.length;
	}
	formerLine.display(-1);
	this.cursorLineEntry.data.display(this.cursorColumn);
	cursorSpan = document.getElementById("curpos");
        this.updateHighlight();
	return true;
    },
    cursorDown:function() {
	if (this.cursorLineEntry==this.bottomLineEntry) {
	    if (this.bottomLineEntry.next.isHead) {
		return;
	    }
	    else this.scrollUp(this.linesVisibleCount>>1,true);
	}
	var formerLine=this.cursorLineEntry.data;
	this.cursorLineEntry=this.cursorLineEntry.next;
	this.cursorLine++;
	this.updateStatus();
	if (this.cursorColumn>this.cursorLineEntry.data.text.length) {
	    this.cursorColumn=this.cursorLineEntry.data.text.length;
	}
	formerLine.display(-1);
	this.cursorLineEntry.data.display(this.cursorColumn);
	cursorSpan = document.getElementById("curpos");
        this.updateHighlight();
    },
    indentCursorLine:function(cursorAtEnd) {
	var beforeCursorEntry=this.cursorLineEntry.prev;
	if (!beforeCursorEntry.isHead) {
	    var indentLevel=this.nextLineIndent(beforeCursorEntry)
	    if (this.cursorLineEntry.data.hasInitialClose()) {
		indentLevel--;
	    }
	    this.cursorLineEntry.data.setIndentLevel(indentLevel);
	    if (cursorAtEnd) {
		this.cursorColumn=this.cursorLineEntry.data.text.length;
	    }
	    else {
		this.cursorColumn=this.cursorLineEntry.data.indentLevel*LineIndentSpaceCount;
	    }
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");	    
	}
    },
    deleteLineBreak:function() {
	var nextLineEntry=this.cursorLineEntry.next;
	if (!(nextLineEntry.isHead)) {
	    this.cursorLineEntry.data.join(nextLineEntry.data.text);
	    this.cursorLineEntry.data.display(this.cursorColumn);
	    cursorSpan = document.getElementById("curpos");
	    if (!(this.bottomLineEntry.next.isHead)) {
	      this.bottomLineEntry=this.bottomLineEntry.next;
	      this.textarea.appendChild(this.getDiv(this.bottomLineEntry));
	    }
	    else if (!(this.topLineEntry.prev.isHead)) {
		this.textarea.insertBefore(this.getDiv(topLineEntry.prev),this.textarea.firstChild);
	      this.topLineEntry=this.topLineEntry.prev;
	    }
	    this.textarea.removeChild(nextLineEntry.data.div);
	    nextLineEntry.data.div=null;	    
	    ListRemoveEntry(nextLineEntry);
	}
        this.updateHighlight();
    },
    breakLine:function() {
	var remainderText=this.cursorLineEntry.data.split(this.cursorColumn);
	this.cursorLineEntry.data.display(-1);
	
	var div=document.createElement("div");
	div.className="code1";

	var nextDiv=null;
	if ((this.cursorLineEntry!=this.bottomLineEntry)&&(!(this.cursorLineEntry.next.isHead))) {
	    nextDiv=this.cursorLineEntry.next.data.div;
	}

	var newLine=new Line(remainderText,LexLine(remainderText),div);
	newLine.setIndentLevel(this.nextLineIndent(this.cursorLineEntry));

	var newEntry=ListInsertAfter(this.cursorLineEntry,newLine);

	if (nextDiv!=null) {
	  this.textarea.insertBefore(div,nextDiv);
	}
	else {
	  this.textarea.appendChild(div);
	}
	if (this.bottomLineEntry==this.cursorLineEntry) {
	  if (this.viewportFull()) {
	      this.scrollUp(1,true);
	  }
	  else this.bottomLineEntry=newEntry;
	}
	else if (this.viewportFull()) {
	    this.textarea.removeChild(this.bottomLineEntry.data.div);
	    this.bottomLineEntry.data.div=null;	    
	    this.bottomLineEntry=this.bottomLineEntry.prev;
	}
	this.cursorLineEntry=newEntry;
	this.cursorLine++;
	this.cursorColumn=newLine.indentLevel*LineIndentSpaceCount;
	this.updateStatus();
	newLine.display(this.cursorColumn);
        cursorSpan = document.getElementById("curpos");
        this.updateHighlight();
    }
};
