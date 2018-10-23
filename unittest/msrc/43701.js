function boom() {
    return '';
}

for (var x = 0; x < 0x1000; x++) { 
  try{
    go();
  }
  catch(e){
  }
}
    
function go() {
    var a = 0x1;
    (function(){})();
    
    // Keeping intact the original POC submitted, but all this poc needs is a string as the with object.
    with(boom(arr = new Array())) {
        for (var x = 0; x < 10; x++) {
            arr[x] += (match `1337`)
        }
    }
}
print("passed");