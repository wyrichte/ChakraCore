//
// Test cases below use code modified from Mozilla's asm.js test suite
//

print(function () {"use asm";var x=0; function f() {x=42} function g() { return (f(),x)|0 } return g}()());
print(function () {"use asm";function f() {return 42} function g() { return f()|0 } return g}()());
print(function () {"use asm";function g(i) { i=i|0; return (i+2)|0 } function h(i) { i=i|0; return ((g(i)|0)+8)|0 } return h}()(50));
print(function () {"use asm";function g(i) { i=i|0; return (i+2)|0 } function h(i) { i=i|0; return ((g(i)|0)+i)|0 } return h}()(50));
print(function () {"use asm";function g(i) { i=+i; return +(i+.1) } function h(i) { i=+i; return +(+g(i)+.2) } return h}()(20));
print(function () {"use asm";function g(i,j) { i=i|0;j=j|0; return (i-j)|0 } function h(j,i) { j=j|0;i=i|0; return ((g(i,j)|0)+8)|0 } return h}()(10,20));
print(function () {"use asm";function g(i,j) { i=i|0;j=+j; return +(+~~i+j) } function h(i,j) { i=i|0;j=+j; return +(+g(i,j)+8.6) } return h}()(10,1.5));
print(function () {"use asm";function f(i,j,k,l,m,n,o) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0; return (n-o)|0 } return f}()(1,2,3,4,5,6,100));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0; return (o-p)|0 } return f}()(1,2,3,4,5,6,100,20));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0; return (((o+p)|0) + ((o+p)|0))|0 } return f}()(1,2,3,4,5,6,30,20));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p,q,r) { i=+i;j=+j;k=+k;l=+l;m=+m;n=+n;o=+o;p=+p;q=+q;r=+r; return +(q-r) } return f}()(1,2,3,4,5,6,7,8,40.2,1.4));
print(function () {"use asm";function f(i,j,k,l,m,n,o) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0; return (n-o)|0 } function g(i,j) { i=i|0;j=j|0; return f(0,0,0,0,0,i,j)|0 } return g}()(20,5));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0; return (o-p)|0 } function g(i,j) { i=i|0;j=j|0; return f(0,0,0,0,0,0,i,j)|0 } return g}()(20,5));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=+i;j=+j;k=+k;l=+l;m=+m;n=+n;o=+o;p=+p; return +(o-p) } function g(i,j) { i=+i;j=+j; return +f(0.0,0.0,0.0,0.0,0.0,0.0,i,j) } return g}()(.5, .1));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0; return (o-p)|0 } function g(i,j) { i=i|0;j=j|0; var k=0; k=(i+j)|0; return ((f(0,0,0,0,0,0,i,j)|0)+k)|0 } return g}()(20,10));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=+i;j=+j;k=+k;l=+l;m=+m;n=+n;o=+o;p=+p; return +(o-p) } function g(i,j) { i=+i;j=+j; var k=0.1; k=i+j; return +(+f(0.0,0.0,0.0,0.0,0.0,0.0,i,j)+k) } return g}()(.5, .1));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p,q) { i=i|0;j=j|0;k=k|0;l=l|0;m=+m;n=n|0;o=o|0;p=+p;q=q|0; return +((m-p) + +~~q) } function g(i,j,k) { i=+i;j=+j;k=k|0; return +f(0,0,0,0,j,0,0,i,k) } return g}()(.5, 20.1, 4));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0; return (o-p)|0 } function g(i,j) { i=i|0;j=j|0; return f(0,0,0,0,0,0,f(0,0,0,0,0,0,i,j)|0,j)|0 } return g}()(20,5));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0; return (o-p)|0 } function g(i,j) { i=i|0;j=j|0; return f(0,0,0,0,0,0,f(0,0,0,0,0,0,i,j)|0,f(0,0,0,0,0,0,j,i)|0)|0 } return g}()(20,5));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=+i;j=+j;k=+k;l=+l;m=+m;n=+n;o=+o;p=+p; return +(o-p) } function g(i,j) { i=+i;j=+j; return +f(0.0,0.0,0.0,0.0,0.0,0.0,+f(0.0,0.0,0.0,0.0,0.0,0.0,i,j),j) } return g}()(10.3, .2));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p) { i=+i;j=+j;k=+k;l=+l;m=+m;n=+n;o=+o;p=+p; return +(o-p) } function g(i,j) { i=+i;j=+j; return +f(0.0,0.0,0.0,0.0,0.0,0.0,+f(0.0,0.0,0.0,0.0,0.0,0.0,i,j),+f(0.0,0.0,0.0,0.0,0.0,0.0,j,i)) } return g}()(10.3, .2));
print(function () {"use asm";function f(i,j,k,l,m,n,o,p,q) { i=i|0;j=j|0;k=k|0;l=l|0;m=m|0;n=n|0;o=o|0;p=p|0;q=q|0; return (o-p)|0 } function g(i,j) { i=i|0;j=j|0; return f(0,0,0,0,0,0,i,f(0,0,0,0,0,0,i,j,0)|0,0)|0 } return g}()(20,5));
print(function () {"use asm";function f(i) {i=i|0; return i|0} function g() { return 42; return f(13)|0 } return g}()());
print(function () {"use asm";function e() { return 42 } function f(i) { i=i|0; switch(i|0) { case 0: return e()|0; default: return 13 } return 0 } function g() { return f(0)|0 } return g}()());
var m1 = function () {"use asm";function rec(i) { i=i|0; if (!i) return 0; return ((rec((i-1)|0)|0)+1)|0 } return rec}();
print(m1(100));
