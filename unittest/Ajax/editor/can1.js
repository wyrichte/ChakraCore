function cursorToggle(){
    if (cursorSpan) {
	if (cursorStatus == "on") {
	    cursorStatus = "off";
	    cursorSpan.className = "cursorOff";
	}
	else {
	    cursorStatus = "on";
	    cursorSpan.className = "cursorOn";
	}
    }
}
                                                                                           
var editor;
var timerId;
var cursorStatus = "on";
var cursorSpan;

function removeTempHighlight() {
    var bod=document.getElementById("bod");
    var hlt=document.getElementById("tmphlt");
    bod.removeChild(hlt);
}

function initialize() {
    LexInitialize();
    var bod=document.getElementById("bod");
    editor=new Editor(bod);

    if (window.IOLib!=undefined) {
	editor.load("c:\\privateie\\editor.js");
    }
    else {
	editor.loadTemp();
	editor.reDisplay();
    }
    editor.updateStatus();
    cursorSpan = document.getElementById("curpos");
    timerId = setInterval(cursorToggle, 700);
    document.onkeydown = EventKeydown;
    document.onkeypress = EventKeypress;
    document.onkeyup = EventKeyup;
    document.onmousewheel = EventMousewheel;
    bod.onresize = EventResize;
}
