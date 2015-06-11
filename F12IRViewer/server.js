//
// Copyright (C) Microsoft. All rights reserved.
//

var http = require('http').createServer(httpHandler),
    fs = require('fs'),
    exec = require('child_process').exec,
    url = require('url');

http.listen(8080);

var _serverError = {
    IsError: false,
    Error:'',
    FaultingModule: ''
};

//----------------------HTTPHANDLER-------------------------//

function httpHandler(req, res) {
    var request = url.parse(req.url, true);
    var action = request.pathname.toLowerCase().trim();

    if (req.method == 'GET')
    {
        try {
            var file;
            var writeFile = true;
            var defaultAction = "/irviewer-standalone.html";

            var contentType = { 'Content-Type': 'text/html' };

            if (action == '/' || action == '/index.html' || action == '/index') {
                action = defaultAction;
            } else if (action.indexOf('.js') !== -1) {
                contentType = { 'Content-Type': 'text/javascript' };
            } else if (action.indexOf('.css') !== -1) {
                contentType = { 'Content-Type': 'text/css' };
            } else if (action.indexOf('.png') !== -1) {
                contentType = { 'Content-Type': 'image/png' };
            } else if (action.indexOf('.jpg') !== -1) {
                contentType = { 'Content-Type': 'image/jpg' };
            } else if (action.indexOf('builds') !== -1 ) {
                writeFile = false;
                console.log('GetBuilds');
                console.log(action);
                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify(BUILDS));
            } else {
                // TODO might be able to delete this entire block
                try {
                    file = fs.readFileSync(__dirname + action);
                    res.writeHead(200, { 'Content-Type': 'application/octet-stream' });
                    res.end(file);
                } catch (e) {
                    res.writeHead(404, { 'Content-Type': 'text/html' });
                    res.end('404');
                }
            }

            if (fs.existsSync(__dirname + action) && writeFile) {
                file = fs.readFileSync(__dirname + action);
                res.writeHead(200, contentType);
                res.end(file);
            } else {
                res.writeHead(404, { 'Content-Type': 'text/html' });
                res.end('404');
            }
        } catch(e) {
            res.writeHead(404, { 'Content-Type': 'text/html' });
            res.end('404');
        }
    }
    else if (req.method == 'POST')
    {
        var payload = "";
        req.on('data',function(data){
           payload += data;
        });
        req.on('end', function(){
           switch(action){
             case '/run':
                console.log('Run');
                console.log(payload);
                res.writeHead(200, { 'Content-Type': 'application/json' });
                Helper.RunCode(JSON.parse(payload), res);
                break;
           }
        });
    }
}

//-----------------------------------------------------------//

/**
 * A simple formatter routine designed to mimic C# style String.Format().
 *
 * @param A list of arguments to be passed to the formatter.
 * @return The string, formatted as in C#.
 */
if (typeof String.prototype.format === "undefined") {
  String.prototype.format = function() {
    // use the implicit "arguments" parameter to get parameters at the given positions.
    var fmt = String(this);
    for (var i = 0; i < arguments.length; i++) {
      var formatter = '{'+i+'}';
      while (fmt.indexOf(formatter) !== -1) {
        fmt = fmt.replace(formatter, arguments[i]);
      }
    }
    return fmt;
  };
}

/**
 * Take a collection of strings and set them as properties on
 * a JS object with the value corresponding to their position in the
 * collection.
 *
 * @param values The collection of strings to make into Enum values.
 * @return An Enum for which values can be accessed as "EnumName.Value"
 */
function Enum(values) {
  var e = {};
  for (var i = 0; i < values.length; i++) {
    var name = values[i];
    var value = i;
    e[name] = value;
  }
  return e;
}

// Returns a random integer between min and max
// Using Math.round() will give you a non-uniform distribution!
function getRandomInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function getDateString(d) {
  function pad(n){return n<10 ? '0'+n : n}
  return d.getFullYear()+'-'
      + pad(d.getMonth()+1)+'-'
      + pad(d.getDate())+'-'
      + pad(d.getHours())+''
      + pad(d.getMinutes())+''
      + pad(d.getSeconds())+'';
}

//-----------------------------------------------------------//

var Helper = {};

Helper.CacheCode = function(runParams) {
  var id = runParams.clientID;
  var contents = runParams.code.toString();

  var rand = getRandomInt(0x1000, 0xFFFFFFFF);
  var randHex = rand.toString(16);

  var cacheDir = "_cachedScripts";
  var destination = cacheDir + '\\' + '{0}-{1}-{2}.js'.format(id, randHex, getDateString(new Date()));
  console.log(destination);

  if (!fs.existsSync(cacheDir)) {
    fs.mkdirSync(cacheDir);
  }

  fs.writeFileSync(destination, contents);
};

Helper.GetCode = function(runParams) {
  var targetCode = runParams.code.toString();
  targetCode = targetCode.replace(/\\\\/g, "\\\\"); // escape backslashes in input code
  targetCode = targetCode.replace(/\"/g, "\\\""); // escape double quotes
  targetCode = targetCode.replace(/\r\n/g, "\\n\\\n"); // escape newlines
  var code = '\
    var code = "{0}";\
    parseIR(code);\
    WScript.Echo("__ir_viewer__"+JSON.stringify(dumpir));\
  '.format(targetCode);
  console.log(code);
  return code;
};

var SearchJsonState = new Enum([
  'Normal',
  'String'
]);

Helper.SearchJsonEnd = function(json) {
  var state = SearchJsonState.Normal;
  var count = 0;
  var i = 0;
  for (; i < json.length; i++) {
    if (state === SearchJsonState.Normal) {
      switch (json.charAt(i)) {
        case '{':
          count++;
          break;
        case '}':
          count--;
          if (count === 0) {
            return i;
          }
          break;
        case '"':
          state = SearchJsonState.String;
        default:
          break;
      }
    } else if (state === SearchJsonState.String) {
      // ignore curly braces inside of strings
      switch (json.charAt(i)) {
        case '"':
          // be careful to ignore escaped quotation marks inside of strings
          if (i > 0 && json.charAt(i-1) === '\\') {
            state = SearchJsonState.String;
          } else {
            state = SearchJsonState.Normal;
          }
          break;
        default:
          break;
      }
    }
  }
  return -1;
}

Helper.RunCode = function(runParams, res) {
    try {
        var result = {};
        var jshost = "\\\\MININT-0IN5E3B\\Public\\jshost";
        var outputFileName = runParams.clientID + ".js";
        var outputFile = runParams.clientID + '\\' + outputFileName;
        var implicitSwitches = " -prejit ";

        jshost += "\\jshost.exe " + implicitSwitches + ' ' + runParams.switches + ' ' + runParams.clientID + '\\' + outputFileName;

        // Helper.CacheCode(runParams);
        var code = Helper.GetCode(runParams);

        var dir = "" + runParams.clientID;

        // failsafe delete temp files
        if (fs.existsSync(dir)) {
            if (fs.existsSync(outputFile)) {
                fs.unlinkSync(outputFile, function(err){
                    console.log("Error While Deleting File: " + err);
                });
            }
            fs.rmdirSync(dir);
        }

        fs.mkdirSync(dir);
        fs.writeFileSync(outputFile, code);
        console.log(jshost);
        console.log('Dir Opened ' + dir);

        var child = exec(jshost, function(err, stdout, stderr) {
            if (err) {
                result.Error = stderr.toString();
            }

            var output = stdout.toString();
            var location = output.search(/__ir_viewer__\{[\s\S]*\}/);
            output = output.slice(location);
            output = output.slice(output.search(/\{/));  // cut out __ir_viewer__ tag at beginning

            var endIndex = Helper.SearchJsonEnd(output);
            output = output.slice(0, endIndex + 1);

            result.Output = output;
            console.log("StdOut: " + stdout.toString());

            // cleanup temp files
            if (fs.existsSync(dir)) {
                if (fs.existsSync(outputFile)) {
                    fs.unlinkSync(outputFile, function(err){
                        console.log("Error While Deleting File: " + err);
                    });
                }
                fs.rmdirSync(dir);
            }

            res.end(JSON.stringify(result));

            console.log(runParams.code.toString());
            console.log("Run completed at: {0}".format(new Date()));
        });
    } catch(e) {
        console.log(e);
        console.log("Continuing past error...");
    }
};

//-----------------------------------------------------------//

//
// done loading server
//

console.log("IRViewer Sandbox deployed. Server listening...");
