//
// Copyright (C) Microsoft. All rights reserved.
//

function ClearSelection() {
  if (window.getSelection().removeAllRanges) {
    window.getSelection().removeAllRanges();
  }
}
