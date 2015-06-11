//
// Copyright (C) Microsoft. All rights reserved.
//

function ClearSelection() {
  if (window.getSelection) {
    if (window.getSelection().empty) {  // Chrome
      window.getSelection().empty();
    } else if (window.getSelection().removeAllRanges) {  // Firefox & modern IE
      window.getSelection().removeAllRanges();
    }
  } else if (document.selection) {  // IE? (legacy)
    document.selection.empty();
  }
}
