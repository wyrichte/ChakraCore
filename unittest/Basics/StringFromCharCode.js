function print(value)
{
    WScript.Echo(value);
}

print(String.fromCharCode(65, 66, 67));
print(String.fromCharCode(65.23, 66, 67.98));
// TODO: Add string parsing: "65", etc.
