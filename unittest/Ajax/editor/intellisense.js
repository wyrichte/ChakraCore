// options is an array of strings for now
function Select(options,lineCount) {
    this.options=options;
    this.lineCount=lineCount;
    this.div=document.createElement("div");
    this.div.className="compsel";
    for (var i=0;(i<lineCount)&&(i<options.length);i++) {
	var optionDiv=document.createElement("div");
	if (i==2) {
	    optionDiv.className="compopthlt";
	}
	else {
	    optionDiv.className="compopt";	    
	}
	if (i==4) {
	    optionDiv.style.color="#0000FF";
	}
	optionDiv.innerHTML='<span>'+options[i]+'</span>';
	this.div.appendChild(optionDiv);
    }
}

