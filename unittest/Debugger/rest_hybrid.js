function justRest(...a) {
  /**bp:stack();locals();**/
}

justRest();
justRest(1, 2, 3);

function someParams(a, b, ...c) {
  /**bp:stack();locals();**/
}

someParams();
someParams(1);
someParams(1, 2);
someParams(1, 2, 3);
someParams(1, 2, 3, 4);
someParams(1, 2, 3, 4, 5);

class C {
  justRest(...a) {
    /**bp:stack();locals();**/
  }
  someParams(a, b, ...c) {
    /**bp:stack();locals();**/
  }
}

let classC = new C();

classC.justRest();
classC.justRest(1, 2, 3);

classC.someParams();
classC.someParams(1);
classC.someParams(1, 2);
classC.someParams(1, 2, 3);
classC.someParams(1, 2, 3, 4);
classC.someParams(1, 2, 3, 4, 5);


let arrowJustRest = (...a) => {
  /**bp:stack();locals();**/
}

arrowJustRest();
arrowJustRest(1, 2, 3);

let arrowSomeParams = (a, b, ...c) => {
  /**bp:stack();locals();**/
}

arrowSomeParams();
arrowSomeParams(1);
arrowSomeParams(1, 2);
arrowSomeParams(1, 2, 3);
arrowSomeParams(1, 2, 3, 4);
arrowSomeParams(1, 2, 3, 4, 5);

let obj = {
  justRest(...a) {
    /**bp:stack();locals();**/
  },
  someParams(a, b, ...c) {
    /**bp:stack();locals();**/
  }
}

obj.justRest();
obj.justRest(1, 2, 3);

obj.someParams();
obj.someParams(1);
obj.someParams(1, 2);
obj.someParams(1, 2, 3);
obj.someParams(1, 2, 3, 4);
obj.someParams(1, 2, 3, 4, 5);

WScript.Echo("PASS");
