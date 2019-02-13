function inlinee() {
    return inlinee.arguments[0]; 
}

function opt(func) {

    let stack_obj = {}
    for(let i =0;i<200;i++)
        stack_obj['a'+i] = i 

    let heap_obj = inlinee(stack_obj);

    func(heap_obj)
    
    stack_obj.b = 10000
}

function func(heap){
    for(let i =0;i<20000;i++)
        heap['aa'+i] = i
}


function main() {
    for (let i = 0; i < 200; i++) {
        opt(new Function(''));
    }

    opt(func)
}
main();
print("passed");
