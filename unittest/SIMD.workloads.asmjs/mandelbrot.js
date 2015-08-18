// Asm.js module buffer.
function asmjsModule(global, imp, buffer) {
    "use asm"
    var b8 = new global.Uint8Array(buffer);
    var toF = global.Math.fround;
    var i4 = global.SIMD.Int32x4;
    var i4check = i4.check;
    var i4extractLane = i4.extractLane;
    var f4 = global.SIMD.Float32x4;
    var f4check = f4.check;
    var i4add = i4.add;
    var i4and = i4.and;
    var f4add = f4.add;
    var f4sub = f4.sub;
    var f4mul = f4.mul;
    var f4lessThanOrEqual = f4.lessThanOrEqual;
    var f4splat = f4.splat;
    var f4extractLane = f4.extractLane;
    var imul = global.Math.imul;

    const one4 = i4(1, 1, 1, 1), two4 = f4(2.0, 2.0, 2.0, 2.0), four4 = f4(4.0, 4.0, 4.0, 4.0);

    const mk0 = 0x00ffffff;
    function declareHeapLength() {
        b8[0x00ffffff] = 0;
        //b8[0x00000fff] = 0;
    }

    function mapColorAndSetPixel(x, y, width, value, max_iterations) {
        x = x | 0;
        y = y | 0;
        width = width | 0;
        value = value | 0;
        max_iterations = max_iterations | 0;

        var rgb = 0, r = 0, g = 0, b = 0, index = 0;

        index = ((((imul((width >>> 0), (y >>> 0)) | 0) + x) | 0) * 4) | 0;
        if ((value | 0) == (max_iterations | 0)) {
            r = 0;
            g = 0;
            b = 0;
        } else {
            rgb = ~~toF(toF(toF(toF(value >>> 0) * toF(0xffff)) / toF(max_iterations >>> 0)) * toF(0xff));
            r = rgb & 0xff;
            g = (rgb >>> 8) & 0xff;
            b = (rgb >>> 16) & 0xff;
        }
        b8[(index & mk0) >> 0] = r;
        b8[(index & mk0) + 1 >> 0] = g;
        b8[(index & mk0) + 2 >> 0] = b;
        b8[(index & mk0) + 3 >> 0] = 255;
        return 0;
    }

    function mandelPixelX4(xf, yf, yd, max_iterations) {
        xf = toF(xf);
        yf = toF(yf);
        yd = toF(yd);
        max_iterations = max_iterations | 0;
        var c_re4 = f4(0.0, 0.0, 0.0, 0.0), c_im4 = f4(0.0, 0.0, 0.0, 0.0);
        var z_re4 = f4(0.0, 0.0, 0.0, 0.0), z_im4 = f4(0.0, 0.0, 0.0, 0.0);
        var count4 = i4(0, 0, 0, 0);
        var z_re24 = f4(0.0, 0.0, 0.0, 0.0), z_im24 = f4(0.0, 0.0, 0.0, 0.0);
        var new_re4 = f4(0.0, 0.0, 0.0, 0.0), new_im4 = f4(0.0, 0.0, 0.0, 0.0);
        var i = 0;
        var mi4 = i4(0, 0, 0, 0);

        c_re4 = f4splat(xf);
        c_im4 = f4(yf, toF(yd + yf), toF(yd + toF(yd + yf)), toF(yd + toF(yd + toF(yd + yf))));

        z_re4 = c_re4;
        z_im4 = c_im4;

        for (i = 0; (i | 0) < (max_iterations | 0) ; i = (i + 1) | 0) {
            z_re24 = f4mul(z_re4, z_re4);
            z_im24 = f4mul(z_im4, z_im4);

            mi4 = f4lessThanOrEqual(f4add(z_re24, z_im24), four4);
            // If all 4 values are greater than 4.0, there's no reason to continue.
            if ((mi4.signMask | 0) == 0x00)
                break;

            new_re4 = f4sub(z_re24, z_im24);
            new_im4 = f4mul(f4mul(two4, z_re4), z_im4);
            z_re4 = f4add(c_re4, new_re4);
            z_im4 = f4add(c_im4, new_im4);
            count4 = i4add(count4, i4and(mi4, one4));
        }
        return i4check(count4);
    }

    function mandelColumnX4(x, width, height, xf, yf, yd, max_iterations) {
        x = x | 0;
        width = width | 0;
        height = height | 0;
        xf = toF(xf);
        yf = toF(yf);
        yd = toF(yd);
        max_iterations = max_iterations | 0;

        var y = 0;
        var ydx4 = toF(0);
        var m4 = i4(0, 0, 0, 0);

        ydx4 = toF(yd * toF(4));
        for (y = 0; (y | 0) < (height | 0) ; y = (y + 4) | 0) {
            m4 = i4check(mandelPixelX4(toF(xf), toF(yf), toF(yd), max_iterations));
            mapColorAndSetPixel(x | 0, y | 0, width, i4extractLane(m4, 0), max_iterations) | 0;
            mapColorAndSetPixel(x | 0, (y + 1) | 0, width, i4extractLane(m4, 1), max_iterations) | 0;
            mapColorAndSetPixel(x | 0, (y + 2) | 0, width, i4extractLane(m4, 2), max_iterations) | 0;
            mapColorAndSetPixel(x | 0, (y + 3) | 0, width, i4extractLane(m4, 3), max_iterations) | 0;
            yf = toF(yf + ydx4);
        }
        return 0;
    }

    function mandel(width, height, xc, yc, scale, max_iterations) {
        width = width | 0;
        height = height | 0;
        xc = toF(xc);
        yc = toF(yc);
        scale = toF(scale);
        max_iterations = max_iterations | 0;

        var x0 = toF(0), y0 = toF(0);
        var xd = toF(0), yd = toF(0);
        var xf = toF(0);
        var x = 0;

        x0 = toF(xc - toF(scale * toF(1.5)));
        y0 = toF(yc - scale);
        xd = toF(toF(scale * toF(3)) / toF(width >>> 0));
        yd = toF(toF(scale * toF(2)) / toF(height >>> 0));
        xf = x0;

        for (x = 0; (x | 0) < (width | 0) ; x = (x + 1) | 0) {
            mandelColumnX4(x, width, height, xf, y0, yd, max_iterations) | 0;
            xf = toF(xf + xd);
        }
        return 0;
    }

    return mandel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

function nonSimdAsmjsModule(global, imp, buffer) {
    "use asm"
    var b8 = new global.Uint8Array(buffer);
    var toF = global.Math.fround;
    var imul = global.Math.imul;

    const mk0 = 0x007fffff;
    function declareHeapLength() {
        b8[0x00ffffff] = 0;
    }

    function mandelPixelX1(xf, yf, yd, max_iterations) {
        xf = toF(xf);
        yf = toF(yf);
        yd = toF(yd);
        max_iterations = max_iterations | 0;

        var z_re = toF(0), z_im = toF(0);
        var z_re2 = toF(0), z_im2 = toF(0);
        var new_re = toF(0), new_im = toF(0);
        var count = 0, i = 0, mi = 0;

        z_re = xf;
        z_im = yf;

        for (i = 0; (i | 0) < (max_iterations | 0) ; i = (i + 1) | 0) {
            z_re2 = toF(z_re * z_re);
            z_im2 = toF(z_im * z_im);

            if (toF(z_re2 + z_im2) > toF(4))
                break;

            new_re = toF(z_re2 - z_im2);
            new_im = toF(toF(z_re * toF(2)) * z_im);
            z_re = toF(xf + new_re);
            z_im = toF(yf + new_im);
            count = (count + 1) | 0;
        }
        return count | 0;
    }

    function mapColorAndSetPixel(x, y, width, value, max_iterations) {
        x = x | 0;
        y = y | 0;
        width = width | 0;
        value = value | 0;
        max_iterations = max_iterations | 0;

        var rgb = 0, r = 0, g = 0, b = 0, index = 0;

        index = ((((imul((width >>> 0), (y >>> 0)) | 0) + x) | 0) * 4) | 0;

        if ((value | 0) == (max_iterations | 0)) {
            r = 0;
            g = 0;
            b = 0;
        } else {
            rgb = ~~toF(toF(toF(toF(value >>> 0) * toF(0xffff)) / toF(max_iterations >>> 0)) * toF(0xff));
            r = rgb & 0xff;
            g = (rgb >>> 8) & 0xff;
            b = (rgb >>> 16) & 0xff;
        }
        //WScript.Echo("W = " + width + ", x = " + x + ",y = " + y);
        //WScript.Echo("Index = " + (index & mk0));
        //WScript.Echo("r, g, b = " + r + "," + g + "," + b);
        b8[(index & mk0) >> 0] = r;
        b8[(index & mk0) + 1 >> 0] = g;
        b8[(index & mk0) + 2 >> 0] = b;
        b8[(index & mk0) + 3 >> 0] = 255;
        return 0;
    }

    function mandelColumnX1(x, width, height, xf, yf, yd, max_iterations) {
        x = x | 0;
        width = width | 0;
        height = height | 0;
        xf = toF(xf);
        yf = toF(yf);
        yd = toF(yd);
        max_iterations = max_iterations | 0;

        var y = 0, m = 0;

        yd = toF(yd);
        for (y = 0; (y | 0) < (height | 0) ; y = (y + 1) | 0) {
            m = mandelPixelX1(toF(xf), toF(yf), toF(yd), max_iterations) | 0;
            mapColorAndSetPixel(x | 0, y | 0, width, m, max_iterations) | 0;
            yf = toF(yf + yd);
        }
        return 0;
    }

    function mandelX1(width, height, xc, yc, scale, max_iterations) {
        width = width | 0;
        height = height | 0;
        xc = toF(xc);
        yc = toF(yc);
        scale = toF(scale);
        max_iterations = max_iterations | 0;

        var x0 = toF(0), y0 = toF(0), xd = toF(0), yd = toF(0), xf = toF(0);
        var x = 0;

        x0 = toF(xc - toF(scale * toF(1.5)));
        y0 = toF(yc - scale);
        xd = toF(toF(scale * toF(3)) / toF(width >>> 0));
        yd = toF(toF(scale * toF(2)) / toF(height >>> 0));
        xf = x0;

        for (x = 0; (x | 0) < (width | 0) ; x = (x + 1) | 0) {
            mandelColumnX1(x, width, height, xf, y0, yd, max_iterations) | 0;
            xf = toF(xf + xd);
        }
        return 0;
    }

    function mandel(width, height, xc, yc, scale, max_iterations) {
        width = width | 0;
        height = height | 0;
        xc = toF(xc);
        yc = toF(yc);
        scale = toF(scale);
        max_iterations = max_iterations | 0;

        var x0 = toF(0), y0 = toF(0);
        var xd = toF(0), yd = toF(0);
        var xf = toF(0);
        var x = 0;

        x0 = toF(xc - toF(scale * toF(1.5)));
        y0 = toF(yc - scale);
        xd = toF(toF(scale * toF(3)) / toF(width >>> 0));
        yd = toF(toF(scale * toF(2)) / toF(height >>> 0));
        xf = x0;

        for (x = 0; (x | 0) < (width | 0) ; x = (x + 1) | 0) {
            mandelColumnX1(x, width, height, xf, y0, yd, max_iterations) | 0;
            xf = toF(xf + xd);
        }
        return 0;
    }

    return mandel;
}
//////////////////////////////////////////////////////////////////////
var use_simd = 0;
var iters = 300;
var max_iterations = 100;
var _width = 32, _height = 32;

// Asm.js module buffer.
var buffer = new ArrayBuffer(16 * 1024 * 1024);

var mandelSimd = asmjsModule(this, {}, buffer);
var mandelNonSimd = nonSimdAsmjsModule(this, {}, buffer);

function drawMandelbrot(width, height, xc, yc, scale, use_simd) {
    //logger.msg ("drawMandelbrot(xc:" + xc + ", yc:" + yc + ")");
    //WScript.Echo("drawMandelbrot(xc:" + xc + ", yc:" + yc + ")");
    if (use_simd)
        mandelSimd(width, height, xc, yc, scale, max_iterations);
    else
        mandelNonSimd(width, height, xc, yc, scale, max_iterations);
}

function animateMandelbrot() {
    var scale_start = 1.0;
    var scale_end = 0.0005;
    var xc_start = -0.5;
    var yc_start = 0.0;
    var xc_end = 0.0;
    var yc_end = 0.75;
    var steps = 200.0;
    var scale_step = (scale_end - scale_start) / steps;
    var xc_step = (xc_end - xc_start) / steps;
    var yc_step = (yc_end - yc_start) / steps;
    var scale = scale_start;
    var xc = xc_start;
    var yc = yc_start;
    var i = 0;


    function draw1() {
        drawMandelbrot(_width, _height, xc, yc, scale, use_simd);
        if (scale < scale_end || scale > scale_start) {
            scale_step = -scale_step;
            xc_step = -xc_step;
            yc_step = -yc_step;
        }
        scale += scale_step;
        xc += xc_step;
        yc += yc_step;
        i++;
    }

    for (var i = 0; i < iters; i++)
        draw1();
}
use_simd = true;
//use_simd = false;
animateMandelbrot();
arr = new Uint8Array(buffer);
for (var i = 0; i < 16 * _width * _height; i += 4) {
    WScript.Echo(arr[i] + "," + arr[i + 1] + "," + arr[i + 2] + "," + arr[i + 3]);
}
