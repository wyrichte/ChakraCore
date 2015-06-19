//
// Copyright (C) Microsoft. All rights reserved.
//

/**
  isViewTags : boolean

  Indicates whether to display HTML tags in the output.
  This is set to true for the HTML client (in html-output.js).
*/
var isViewTags = false;

/**
  ViewIR

  Display the IR for this function.

  @param inst The first IR instruction in the function.
*/
function ViewIR(inst, metadata) {
  while (inst)
  {
    Instruction.Dump(inst, isViewTags, metadata);
    inst = inst.next;
  }
}
