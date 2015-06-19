//
// Copyright (C) Microsoft. All rights reserved.
//

var fnlist = undefined;

function getFunctionList() {
  if (fnlist===undefined) {
    fnlist = functionList();
  }
  return fnlist;
}

function fileSelectionChanged(list) {
  var selected = $("#select-file-name").val();

  var functions = _(list).filter(function(item) {
    return item.filename == selected;
  })
  .map('function')
  .sortBy(function (a) {
    return a.toLowerCase();
  })
  .value();

  var options = _(functions).reduce(function(result, value) {
    result += '<option value="{0}">{0}</option>'.format(value);
    return result;
  }, '');

  $("#select-function-name").html(options);
  $("#resultOutput").html(JSON.stringify(list));
  $("#resultError").html("Updated UI");
}

function fileSelectionChangedEvent(event) {
  var list = getFunctionList();
  fileSelectionChanged(list);
  functionSelectionChanged(list);
}

function _filterAliasDynamic(item) {
  var str = item.filename;
  var dyn = "[dynamic";
  if (str.substring(0, dyn.length) === dyn) {
    item.filename = "[dynamic]";
  }
  return item.filename;
}

function PopulateFunctionList(list) {
  fnlist = list;  // cache the function listing from the target page

  var files = _(list).unique(_filterAliasDynamic)
  .map('filename')
  .sortBy(function (a) {
    return a.toLowerCase();
  })
  .value();

  var options = _(files).reduce(function(result, value) {
    result += '<option value="{0}">{0}</option>'.format(value);
    return result;
  }, '');

  $("#select-file-name").html(options);
  fileSelectionChanged(list);
  functionSelectionChanged(list);
}

function PopulateFunctionListForm() {
  var list = functionList();
  PopulateFunctionList(list);
}

function functionSelectionChanged(list) {
  var file = $("#select-file-name").val();
  var func = $("#select-function-name").val();

  var matches = _(list).filter(function(item) {
    var fileMatch = _filterAliasDynamic(item) == file;
    var funcMatch = item.function == func;
    return fileMatch && funcMatch;
  })
  .value();

  var sourceCode = "";
  var functionInfo = matches[0];
  if (!(functionInfo === undefined)) {
    sourceCode = functionInfo.source;
  }

  SetMonacoCode(sourceCode);
  
  sendMessageWithData(_port, HeaderMessage.REQUEST_JIT, functionInfo);
}

function functionSelectionChangedEvent(event) {
  var list = getFunctionList();
  functionSelectionChanged(list);
}

function InitializeFunctionListForm() {
  $("#select-file-name").on("change", fileSelectionChangedEvent);
  $("#select-function-name").on("change", functionSelectionChangedEvent);
}
