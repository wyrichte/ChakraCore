var num1;
var num2;

function nextFibo(done) {
  var temp = num2;
  num2 = num1 + num2;
  num1 = temp;
  done(num1, num2);
}


