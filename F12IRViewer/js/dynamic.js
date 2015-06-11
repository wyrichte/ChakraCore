//
// Copyright (C) Microsoft. All rights reserved.
//

// requires dynamic-selection.js

/**
  Clears .highlighted from all elements.
  Adds .highlighted to all elements of the given class.
*/
function Highlight(str) {
  $(".highlighted").removeClass("highlighted");
  $("."+str).addClass("highlighted");

  displayCodeFormattingAllPhases();
}

function HighlightAll() {
  $(".ir-inst > span").addClass("highlighted");

  displayCodeFormattingAllPhases();
}

function RemoveHighlights() {
  $(".highlighted").removeClass("highlighted");

  displayCodeFormattingAllPhases();
}

function FoldAll() {
  $(".ir-statement-compact").removeClass("hidden");
  $(".ir-statement-full").addClass("hidden");
}

function UnfoldAll() {
  $(".ir-statement-compact").addClass("hidden");
  $(".ir-statement-full").removeClass("hidden");
}

function AttachClickEvents() {
  $(".ir-dump").on("click", ".ir-inst > span:not(.ir-statement-toggle-hide, .ir-statement-toggle-show)", function(e) {
    if (!e.ctrlKey) {
      $(".highlighted").removeClass("highlighted");
    } else {
      ClearSelection();
    }

    var name = $(this).attr("name");

    if (e.ctrlKey) {
      $(".ir-inst > span[name='{0}']".format(name)).toggleClass("highlighted");
    } else {
      $(".ir-inst > span[name='{0}']".format(name)).addClass("highlighted");
    }

    displayCodeFormattingAllPhases();
  });

  $(".ir-dump").on("mouseover", ".ir-inst > span", function(e) {
    $(".preview").removeClass("preview");
    var name = $(this).attr("name");
    $(".ir-inst > span[name='{0}']".format(name)).addClass("preview");
  });

  $(".ir-dump").on("mouseout", ".ir-inst > span", function(e) {
    $(".preview").removeClass("preview");
  });

  $(".ir-dump").on("click", function(e) {
    if (e.ctrlKey) {
      ClearSelection();
    }
  });

  $(".ir-dump").on("mouseover", ".ir-statement", function(e) {
    var name = $(this).attr("name");
    $("[name='{0}'] .ir-statement-toggle-hide".format(name)).removeClass("invisible");
  });

  $(".ir-dump").on("mouseout", ".ir-statement", function(e) {
    var name = $(this).attr("name");
    $("[name='{0}'] .ir-statement-toggle-hide".format(name)).addClass("invisible");
  });

  $(".ir-dump").on("click", ".ir-statement-toggle-hide", function(e) {
    var name = $(this).attr("name");
    var id = name.replace(/\D*(\d*)/, '$1');
    $("[name='ir-statement-compact-{0}']".format(id)).removeClass("hidden");
    $("[name='ir-statement-full-{0}']".format(id)).addClass("hidden");
  });

  $(".ir-dump").on("click", ".ir-statement-toggle-show", function(e) {
    var name = $(this).attr("name");
    var id = name.replace(/\D*(\d*)/, '$1');
    $("[name='ir-statement-compact-{0}']".format(id)).addClass("hidden");
    $("[name='ir-statement-full-{0}']".format(id)).removeClass("hidden");
  });
}
