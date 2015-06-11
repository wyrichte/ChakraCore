function foo(/**target:fooA**/a, .../**target:fooB**/b) {
  return /**gd:fooA**/a + /**gd:fooB**/b./**ml:length,copyWithin**/length;
}
foo(/**pl:a,...b**/1, 2, 3);

function fooArgs(/**target:fooArgsA**/a, .../**target:fooArgsB**/b) {
  arguments;
  return /**gd:fooArgsA**/a + /**gd:fooArgsB**/b./**ml:length,copyWithin**/length;
}
fooArgs(/**pl:a,...b**/1, 2, 3);

function fooEval(/**target:fooEvalA**/a, .../**target:fooEvalB**/b) {
  eval("");
  return /**gd:fooEvalA**/a + /**gd:fooEvalB**/b./**ml:length,copyWithin**/length;
}
fooEval(/**pl:a,...b**/1, 2, 3);

function fooThis(/**target:fooThisA**/a, .../**target:fooThisB**/b) {
  this;
  return /**gd:fooThisA**/a + /**gd:fooThisB**/b./**ml:length,copyWithin**/length;
}
fooThis(/**pl:a,...b**/1, 2, 3);

// Nested functions

function fooNested(/**target:fooNestedA**/a, .../**target:fooNestedB**/b) {
  function bar(/**target:fooNestedC**/c, .../**target:fooNestedD**/d) {
    return /**gd:fooNestedC**/c + /**gd:fooNestedD**/d./**ml:length,copyWithin**/length;
  }
  return /**gd:fooNestedA**/a + /**gd:fooNestedB**/b./**ml:length,copyWithin**/length
  +  bar(/**pl:!a,!b,c,...d**/ /**gd:fooNestedA**/a,  .../**gd:fooNestedB**/b);
}
fooNested(/**pl:a,...b,!c,!d**/1, 2, 3);

function fooArgsNested(/**target:fooArgsNestedA**/a, .../**target:fooArgsNestedB**/b) {
  arguments;
  function bar(/**target:fooArgsNestedC**/c, .../**target:fooArgsNestedD**/d) {
    return /**gd:fooArgsNestedC**/c + /**gd:fooArgsNestedD**/d./**ml:length,copyWithin**/length;
  }
  return /**gd:fooArgsNestedA**/a + /**gd:fooArgsNestedB**/b./**ml:length,copyWithin**/length
  +  bar(/**pl:!a,!b,c,...d**/ /**gd:fooArgsNestedA**/a,  .../**gd:fooArgsNestedB**/b);
}
fooArgsNested(/**pl:a,...b,!c,!d**/1, 2, 3);

function fooEvalNested(/**target:fooEvalNestedA**/a, .../**target:fooEvalNestedB**/b) {
  eval("");
  function bar(/**target:fooEvalNestedC**/c, .../**target:fooEvalNestedD**/d) {
    return /**gd:fooEvalNestedC**/c + /**gd:fooEvalNestedD**/d./**ml:length,copyWithin**/length;
  }
  return /**gd:fooEvalNestedA**/a + /**gd:fooEvalNestedB**/b./**ml:length,copyWithin**/length
  +  bar(/**pl:!a,!b,c,...d**/ /**gd:fooEvalNestedA**/a,  .../**gd:fooEvalNestedB**/b);
}
fooEvalNested(/**pl:a,...b,!c,!d**/1, 2, 3);

function fooThisNested(/**target:fooThisNestedA**/a, .../**target:fooThisNestedB**/b) {
  this;
  function bar(/**target:fooThisNestedC**/c, .../**target:fooThisNestedD**/d) {
    return /**gd:fooThisNestedC**/c + /**gd:fooThisNestedD**/d./**ml:length,copyWithin**/length;
  }
  return /**gd:fooThisNestedA**/a + /**gd:fooThisNestedB**/b./**ml:length,copyWithin**/length
  +  bar(/**pl:!a,!b,c,...d**/ /**gd:fooThisNestedA**/a,  .../**gd:fooThisNestedB**/b);
}
fooThisNested(/**pl:a,...b,!c,!d**/1, 2, 3);

function fooArrowNested(/**target:fooArrowNestedA**/a, .../**target:fooArrowNestedB**/b) {
  let bar = (/**target:fooArrowNestedC**/c, .../**target:fooArrowNestedD**/d) => {
    return /**gd:fooArrowNestedC**/c + /**gd:fooArrowNestedD**/d./**ml:length,copyWithin**/length;
  }
  return /**gd:fooArrowNestedA**/a + /**gd:fooArrowNestedB**/b./**ml:length,copyWithin**/length
  +  bar(/**pl:!a,!b,c,...d**/ /**gd:fooArrowNestedA**/a,  .../**gd:fooArrowNestedB**/b);
}
fooArrowNested(/**pl:a,...b,!c,!d**/1, 2, 3);

function fooCaptured(/**target:fooCapturedA**/a, .../**target:fooCapturedB**/b) {
  function bar() {
    return /**gd:fooCapturedA**/a + /**gd:fooCapturedB**/b./**ml:length,copyWithin**/length;
  }
  return bar();
}
fooCaptured(/**pl:a,...b,!c,!d**/1, 2, 3);
