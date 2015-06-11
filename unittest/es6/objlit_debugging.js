var sym = 'sym';
var o;
o = { 
  [sym] : {
    bar: function () {
      sym;
      sym;    /**bp:stack()**/
    }
  },
  ['simplestring'] : function () {
    this;
    this;    /**bp:stack()**/
  },
  ['simplestringshorthand'] () {
    this;
    this;    /**bp:stack()**/
  },
  get ['simplestringgetset'] () {
    return 0;    /**bp:stack()**/
  },
  set ['simplestringgetset'] (a) {
    a;    /**bp:stack()**/
  },
  ['complex' + 'expr'] : function () {
    this;
    this;    /**bp:stack()**/
  },
  ['complex' + 'expr' + 'shorthand'] () {
    this;
    this;    /**bp:stack()**/
  },
  get ['complex' + 'expr' + 'getset'] () {
    return 0;    /**bp:stack()**/
  },
  set ['complex' + 'expr' + 'getset'] (a) {
    a;    /**bp:stack()**/
  },
  [123] : function () {
    this;
    this;    /**bp:stack()**/
  },
  [321] () {
    this;
    this;    /**bp:stack()**/
  },
  get [312] () {
    return 0;    /**bp:stack()**/
  },
  set [312] (a) {
    a;    /**bp:stack()**/
  },
  [123.456] : function () {
    this;
    this;    /**bp:stack()**/
  },
  [654.321] () {
    this;
    this;    /**bp:stack()**/
  },
  get [645.312] () {
    return 0;    /**bp:stack()**/
  },
  set [645.312] (a) {
    a;    /**bp:stack()**/
  }
};

var temp;
o[sym].bar();
o['simplestring']();
o['simplestringshorthand']();
temp = o["simplestringgetset"];
o["simplestringgetset"] = temp;
o['complexexpr']();
o['complexexprshorthand']();
temp = o["complexexprgetset"];
o["complexexprgetset"] = temp;
o[123]();
o[321]();
temp = o[312];
o[312] = temp;
o[123.456]();
o[654.321]();
temp = o[645.312];
o[645.312] = temp;
/**bp:evaluate('o', 2)**/

WScript.Echo("pass");
