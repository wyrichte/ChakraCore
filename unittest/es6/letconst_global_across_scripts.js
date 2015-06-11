print('\n==== Basic let and const variables at global scope -- Second Script ====\n');

let shadow_let = 'global let shadow_let';
const shadow_const = 'global const shadow_const';

print('\nNaked references from second script\n');

print(a);
print(b);
print(c);
print(d);
print(e);

print(shadow_let);
print(shadow_const);

print('\nthis. references from second script\n');

print(this.a);
print(this.b);
print(this.c);
print(this.d);
print(this.e);

print(this.shadow_let);
print(this.shadow_const);

print('\nfor-in enumeration of this from second script\n');

for (let p in this)
{
    if (filter(p))
    {
        print(p + ': ' + this[p]);
    }
}
