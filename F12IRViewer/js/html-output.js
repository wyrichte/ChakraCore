//
// Copyright (C) Microsoft. All rights reserved.
//

// this file must be included in HTML after irviewer.js, or this function [re]definition will fail

Output.out = "";

/** Update Clear to also clear the output string. */
Output.Clear = function() {
  Output._ResetBuffer();
  Output.out = "";
}

/** This function to override the original WScript.Echo function specifically for displaying in IE. */
Output._WriteOut = function() {
  var text = Output.buffer;
  Output.out += text + "\n";
};

/** Retrieve the output string and reset Output. */
Output.GetOutputString = function() {
  var out = Output.out;
  Output.Clear();
  return out;
};

// display HTML tags when in HTML mode
var isViewTags = true;
