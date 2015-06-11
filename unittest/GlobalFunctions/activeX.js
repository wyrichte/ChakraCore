function write(args)
{
  WScript.Echo(args);
}

Function.prototype.k = 20;

write(ActiveXObject);
write(ActiveXObject.prototype);

write("check methods");
write("ActiveXObject.apply:" + ActiveXObject.hasOwnProperty("apply"));
write("ActiveXObject.call:" + ActiveXObject.hasOwnProperty("call"));
write("ActiveXObject.constructor:" + ActiveXObject.hasOwnProperty("constructor"));
write("ActiveXObject.hasOwnProperty:" + ActiveXObject.hasOwnProperty("hasOwnProperty"));
write("ActiveXObject.isPrototypeOf:" + ActiveXObject.hasOwnProperty("isPrototypeOf"));
write("ActiveXObject.length:" + ActiveXObject.hasOwnProperty("length"));
write("ActiveXObject.propertyIsEnumerable:" + ActiveXObject.hasOwnProperty("propertyIsEnumerable"));
write("ActiveXObject.prototype:" + ActiveXObject.hasOwnProperty("prototype"));
write("ActiveXObject.toLocaleString:" + ActiveXObject.hasOwnProperty("toLocaleString"));

write("ActiveXObject.prototype.apply:" + ActiveXObject.prototype.hasOwnProperty("apply"));
write("ActiveXObject.prototype.call:" + ActiveXObject.prototype.hasOwnProperty("call"));
write("ActiveXObject.prototype.constructor:" + ActiveXObject.prototype.hasOwnProperty("constructor"));
write("ActiveXObject.prototype.hasOwnProperty:" + ActiveXObject.prototype.hasOwnProperty("hasOwnProperty"));
write("ActiveXObject.prototype.isPrototypeOf:" + ActiveXObject.prototype.hasOwnProperty("isPrototypeOf"));
write("ActiveXObject.prototype.length:" + ActiveXObject.prototype.hasOwnProperty("length"));
write("ActiveXObject.prototype.propertyIsEnumerable:" + ActiveXObject.prototype.hasOwnProperty("propertyIsEnumerable"));
write("ActiveXObject.prototype.prototype:" + ActiveXObject.prototype.hasOwnProperty("prototype"));
write("ActiveXObject.prototype.toLocaleString:" + ActiveXObject.prototype.hasOwnProperty("toLocaleString"));


var ExcelApp = new ActiveXObject("Excel.Application");

ActiveXObject.prototype.x = "hello";
ActiveXObject.y = "bar";

write("ActiveXObject.length:" + ActiveXObject.length);
write("ActiveXObject.prototype:" +ActiveXObject.prototype);
write("ActiveXObject.prototype.length:" +ActiveXObject.prototype.length);

write("ActiveXObject.prototype");

for(var i in ActiveXObject.prototype)
{
  write("i:" + i +" value:"+ ActiveXObject.prototype[i]);
}
write("ActiveXObject");

for(var i in ActiveXObject)
{
  write("i:" + i +" value:"+ ActiveXObject[i]);
}

write("Flag test on ActiveXObject.length");
ActiveXObject.length = 20;
write("ActiveXObject.length:" + ActiveXObject.length);
delete ActiveXObject.length;
write("ActiveXObject.length:" + ActiveXObject.length);


write("Flag test on ActiveXObject.prototype");

delete ActiveXObject.prototype;
write(ActiveXObject.prototype);

ActiveXObject.prototype = 10;
write(ActiveXObject.prototype);



write(ActiveXObject.prototype.prototype);


write("Flag test on ActiveXObject");
//delete this.ActiveXObject;
write(this.ActiveXObject);

ActiveXObject = 10;
write(ActiveXObject);


/*
var ExcelApp = new ActiveXObject("Excel.Application");
var ExcelSheet = new ActiveXObject("Excel.Sheet");


// Make Excel visible through the Application object.
ExcelSheet.Application.Visible = true;	
// Place some text in the first cell of the sheet.					
ExcelSheet.ActiveSheet.Cells(1,1).Value = "This is column A, row 1";	
// Save the sheet.
ExcelSheet.SaveAs("C:\\TEST.XLS");	
// Close Excel with the Quit method on the Application object.			
		
ExcelSheet.Application.Quit();	
// Release the object variable.						
ExcelSheet = "";
*/