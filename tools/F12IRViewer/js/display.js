//
// Copyright (C) Microsoft. All rights reserved.
//

// failsafe if parseIR is not defined
if (!parseIR) {
  function parseIR(text) {
    Output.WriteLine(text);
    Output.WriteLine("> parseIR() is undefined.");
  }
  var dumpir;  // define the dumpir global variable to avoid errors
  var parseIR_missing = true;  // to remember that the parseIR function was not defined
}

//
// support functions
//

function preformat(str) {
  return "<pre>"+str+"</pre>";
}

function DisplayHelper(inst, metadata) {
  if (inst===undefined) {
    Output.WriteLine("> Unable to parse script.");
  } else {
    ViewIR(inst, metadata);
  }

  return Output.GetOutputString();
}

//
// display code formatting (in page)
//

function displayCodeFormatting(e) {
  var code = $('#{0}'.format(e)).html();
  code = code.replace(/\&nbsp\;/gm, ' ');
  $('#{0}-text'.format(e)).text(code);
}

function displayCodeFormattingAllPhases() {
  displayCodeFormatting("irbuilder-dump");
  displayCodeFormatting("globopt-dump");
  displayCodeFormatting("lowerer-dump");
  displayCodeFormatting("regalloc-dump");
  displayCodeFormatting("encoder-dump");
}

//
// core
//

function DisplayIRData(ir) {
  if ((typeof ir) == "undefined") {
    // clear the viewer because there was no code to display
    $('#irbuilder-dump').html("");
    $('#globopt-dump').html("");
    $('#lowerer-dump').html("");
    $('#regalloc-dump').html("");
    $('#encoder-dump').html("");
  } else {
    $('#irbuilder-dump').html(DisplayHelper(ir.IRBuilder, ir.metadata));
    $('#globopt-dump').html(DisplayHelper(ir.GlobOpt, ir.metadata));
    $('#lowerer-dump').html(DisplayHelper(ir.Lowerer, ir.metadata));
    $('#regalloc-dump').html(DisplayHelper(ir.RegAlloc, ir.metadata));
    $('#encoder-dump').html(DisplayHelper(ir.Encoder, ir.metadata));
  }
}

function DisplayParsedIR() {
  var ir = parseScript();
  DisplayIRData(ir);
}
