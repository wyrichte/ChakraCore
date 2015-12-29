for (let forletin in { a: 1, b: 2, c: 3 } /**bp:locals()**/) {
    forletin; /**bp:locals()**/
}

for (const forconstin in { a: 1, b: 2, c: 3 } /**bp:locals()**/) {
    forconstin; /**bp:locals()**/
}

for (let forletof of [ 1, 2, 3, ] /**bp:locals()**/) {
    forletof; /**bp:locals()**/
}

for (const forconstof of [ 1, 2, 3 ] /**bp:locals()**/) {
    forconstof; /**bp:locals()**/
}

for (let forlet = 0; /**bp:locals()**/
        forlet < 3; /**bp:locals()**/
        forlet += 1) /**bp:locals()**/
{
    forlet; /**bp:locals()**/
}

{
    // const for loop variable cannot be assigned to so use a local
    // counter variable instead.  Scope it to a block so it doesn't
    // add noise to the other locals() calls above
    let forconsti = 0;
    for (const forconst = 0; /**bp:locals()**/
            forconsti < 1; /**bp:locals()**/
            forconsti += 1) /**bp:locals()**/
    {
        forconst; /**bp:locals()**/
    }
}

print("pass");
