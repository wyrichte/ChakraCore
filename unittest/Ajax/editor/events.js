function EventMousewheel(e) {
    var e = window.event || e;
    var delta=e.wheelDelta;
    if (delta<0) {
	editor.scrollDownWCursor(1,true);
    }
    else if (delta>0) {
	editor.scrollUpWCursor(1,true);
    }
    editor.updateStatus();
    if (e.stopPropagation)
	e.stopPropagation();
    e.cancelBubble=true;
    return false;
}

function EventMouseup(e) {
    var e = window.event || e;
    var target;
    if (e.target) {
	target = e.target;
    }
    else if (e.srcElement) {
	target = e.srcElement;
    }
    if (e.button==1) {
	editor.setCursorFromCoordinates(e.offsetX,e.offsetY,true);
	editor.updateHighlight();
    }
}

function EventMousedown(e) {
    var e = window.event || e;
    var target;
    if (e.target) {
	target = e.target;
    }
    else if (e.srcElement) {
	target = e.srcElement;
    }
    if (e.button==1) {
	editor.clearMark();
	editor.setCursorFromCoordinates(e.offsetX,e.offsetY,false);
	editor.setMark();
    }
}

function EventMousemove(e) {
    var e = window.event || e;
    var target;
    if (e.target) {
	target = e.target;
    }
    else if (e.srcElement) {
	target = e.srcElement;
    }
    // TODO: other browser's button codes
    if (e.button==1) {
	editor.setCursorFromCoordinates(e.offsetX,e.offsetY,false);
	editor.updateHighlight();
    }
}

function EventKeypress(e) {
    var e = window.event || e;
    var code=e.keyCode || e.charCode;
    if (e.ctrlKey) {
	if (e.stopPropagation)
	    e.stopPropagation();
	e.cancelBubble=true;
	e.returnValue=false;
	return false;
    }
    else {
	if ((code>=97)&&(code<=122)&&(e.shiftKey)) {
	    code-=32;
	}
	if ((code>=32)&&(code<127)) {
	    editor.insertChar(String.fromCharCode(code));
	}
    }
    return true;
}

function EventKeydown(e) {
    eventTime=(new Date()).getTime();
    var e = window.event || e;
    var code=e.keyCode || e.charCode;
    if ((e.ctrlKey)||(code==9)) {
	if (code==9) {
	    // TAB
	    editor.indentCursorLine(false);
	    editor.cancelIncrSearch();
	}
	else {
	    var s=String.fromCharCode(code);
	    switch (s) {
	    case " ":
		editor.setMark();
		break;
	    case "M":
		editor.testSelect();
		break;
	    case "V":
		editor.scrollPageUp();
		break;
	    case "S":
		editor.startIncrSearch(false);
		break;
	    case "R":
		editor.startIncrSearch(true);
		break;
	    case "K":
		editor.deleteToEOL();
		break;
	    case "J":
		editor.breakLine();		
		break;
	    case "W":
		editor.cut();
		break;
	    case "Y":
		editor.paste();
		break;
	    case "G":
		editor.clearMark();
		break;
	    case "F":
		editor.cursorRight();
		break;
	    case "B":
		editor.cursorLeft();
		break;
	    case "L":
		eventTime=editor.gotoLine();
		break;
	    case "D":
		editor.deleteNext();
		break;
	    case "P":
		editor.cursorUp(false);
		break;
	    case "N":
		editor.cursorDown();
		break;
	    case "A":
		editor.beginningOfLine();
		break;
	    case "E":
		editor.endOfLine();
		break;
	    }
	    if ((s!="S")&&(s!="R")&&(code!=17)) {
		editor.cancelIncrSearch();
	    }
	}
	e.cancelBubble=true;
	e.returnValue=false;
	try {
	    e.keyCode = 0;
	}
	catch (e) {

	}
	eventTime=(new Date()).getTime()-eventTime;
	if (eventTime>editor.maxEventTime) {
	    editor.maxEventTime=eventTime;
	    editor.updateStatus();
	}
	editor.totalEventTime+=eventTime;
	editor.eventCount++;
	return false;
    }
    else if (e.altKey) {
	var salt=String.fromCharCode(code);
	if (salt=="V") {
	    editor.scrollPageDown();
	}
	e.cancelBubble=true;
	e.returnValue=false;
	try {
	    e.keyCode = 0;
	}
	catch (e) {

	}
	eventTime=(new Date()).getTime()-eventTime;
	if (eventTime>editor.maxEventTime) {
	    editor.maxEventTime=eventTime;
	    editor.updateStatus();
	}
	editor.totalEventTime+=eventTime;
	editor.eventCount++;
	editor.cancelIncrSearch();
	return false;
    }
    else {
	if (code==8) {
	    editor.deletePrev();
	}
	else if (code==46) {
	    editor.deleteNext();
	}
	else if (code==33) {
	    editor.scrollPageDown();
	}
	else if (code==36) {
	    editor.home();
	}
	else if (code==34) {
	    editor.scrollPageUp();
	}
	else if (code==37) {
	    editor.cursorLeft();
	}
	else if (code==38) {
	    editor.cursorUp(false);
	}
	else if (code==39) {
	    editor.cursorRight();
	}
	else if (code==40) {
	    editor.cursorDown();
	}
	else if (code==13) {
	    editor.breakLine();
	}
	else {
	    return true;
	}
	eventTime=(new Date()).getTime()-eventTime;
	editor.totalEventTime+=eventTime;
	editor.eventCount++;
	if (eventTime>editor.maxEventTime) {
	    editor.maxEventTime=eventTime;
	    editor.updateStatus();
	}
	if (code!=8) {
	    editor.cancelIncrSearch();
	}
	if (e.stopPropagation)
	    e.stopPropagation();
	e.cancelBubble=true;
	e.returnValue=false;
	return false;
    }
}

function EventKeyup(e) {
    var e = window.event || e;
    var code=e.keyCode || e.charCode;
    if (e.ctrlKey) {
	var s=String.fromCharCode(code);
	if (e.stopPropagation)
	    e.stopPropagation();
	e.cancelBubble=true;
	e.returnValue=false;
	return false;
    }
}

function EventResize(e) {
    editor.resize();
    var e = window.event || e;
    e.cancelBubble=true;
    e.returnValue=false;
}