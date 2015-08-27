function Shape() {
  this.color = "white"; 
}

function Circle() {
  this.diameter = 1;
}

Circle.prototype = new Shape();




