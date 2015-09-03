function foo(p)
{
    if ((p | 0) == 91)
        while (1) p = 91;
    return p;
}
foo(0);
foo(0);
foo(0);
foo(0);
foo(0);
foo(0);
print("Done!");