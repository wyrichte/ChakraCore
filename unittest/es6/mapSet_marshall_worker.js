this.onmessage = function(event){
    try {
        var a = event.data;
        postMessage(a);
    } catch(e){
        postMessage("Error");
    }
}