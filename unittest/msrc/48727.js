let str = {};
let b = 'A'.repeat(129);
for (let i = 0; i < 129; i++) {
    str = 'BBBBBBBBB' + str + b;
}

let rep_str = str.repeat(0xfff); 
let toLocale_str = String.prototype.toLocaleLowerCase.apply(rep_str,[]);

let res = rep_str.toString().lastIndexOf(toLocale_str,20);

print(res == -1 ? 'Pass' : 'Fail');
