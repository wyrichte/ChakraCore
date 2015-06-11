var ForReading = 1;

var f = fso.OpenTextFile(currentPath + "\Dynamic.js", ForReading);
var script = f.ReadAll();

eval(script);