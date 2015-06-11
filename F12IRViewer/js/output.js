//
// Copyright (C) Microsoft. All rights reserved.
//

if (!Output) {
  var Output = {};

  Output.buffer = "";
  Output.column = 0;
  Output._htmlSpaces = false;

  Output._ResetBuffer = function() {
    Output.buffer = "";
    Output.column = 0;
  };

  Output.Clear = function() {
    Output._ResetBuffer();
  };

  /** This function to be changed as appropriate for the display. */
  Output._WriteOut = function() {
    WScript.Echo(Output.buffer);
  };

  Output.Flush = function() {
    Output._WriteOut();
    Output._ResetBuffer();
  };

  Output.SetColumn = function(n) {
    do {  // print at least one space
      if (Output._htmlSpaces) {
        Output.WriteZeroLength("&nbsp;");
        Output.column += 1;
      } else {
        Output.Write(" ");
      }
    } while (Output.column < n);
  };

  Output.Write = function(str) {
    var len = str.length;
    if (Output._htmlSpaces) {
      str = str.replace(/  /g, ' &nbsp;');
    }
    Output.WriteZeroLength(str);
    Output.column += len;
  };

  Output.WriteZeroLength = function(str) {
    Output.buffer += str;
  };

  Output.WriteLine = function(str) {
    Output.Write(str);
    Output.Flush();
  };

  Output.NewLine = function(str) {
    if (Output._htmlSpaces) {
      Output.WriteZeroLength('&nbsp;</div><div class="ir-inst">');
      Output.Flush();
    } else {
      Output.WriteLine("");
    }
  };

  Output.BlankLine = function(str) {
    if (Output._htmlSpaces) {
      Output.WriteZeroLength('<div class="ir-inst">&nbsp;</div>');
      Output.Flush();
    } else {
      Output.WriteLine("");
    }
  };

  Output.Break = function(str) {
    if (Output._htmlSpaces) {
      Output.WriteZeroLength('<br />');
      Output.Flush();
    } else {
      Output.WriteLine("");
    }
  };

  Output.SetHtmlSpacesStyle = function(htmlSpaces) {
    Output._htmlSpaces = htmlSpaces;
  };
}
