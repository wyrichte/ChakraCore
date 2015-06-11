function global1() { }
function global2() { }

var a = 1;
var b = 2;

function scope1(p1, p2) {
  var c = 3;
  var d = 4;

  function scope2(p3, p4) {
    var g = 1;
    var h = 2;

    function scope3(p5, p6) {
      !|:
    }

    !|:
  }

  !|:
  var e = 5;
  var f = 6;
  !|:
}

!|:
var i - 20;

function global3() { }