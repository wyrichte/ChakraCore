return X(this, a, b, true, c.attr);
function X(a, b, d, f, e, j) { 
    var i = a.length;
    if (typeof b === "object") { 
        for (var o in b)
                X(a, o, b[o], f, e, d);
        return a 
    }
        if (d !== w) { 
            f = !j && f && c.isFunction(d);
                    for (o = 0; o < i; o++)
            e(a[o], b, f ? d.call(a[o], o, e(a[o], b)) : d, j);
            return a;
        }
            return i ? e(a[0], b) : w;
            }