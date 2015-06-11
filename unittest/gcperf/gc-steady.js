//! ScriptLibrary1.debug.js
//


ScriptLibrary1 = new Object();
////////////////////////////////////////////////////////////////////////////////
// ScriptLibrary1._memoryAlloc

ScriptLibrary1._memoryAlloc = function ScriptLibrary1__memoryAlloc() {
    /// <field name="_old" type="Array" elementType="Object">
    /// </field>
    /// <field name="_med" type="Array" elementType="Object">
    /// </field>
    /// <field name="_rand" type="ScriptLibrary1._rand">
    /// </field>
    /// <field name="_gaus" type="ScriptLibrary1._gaussian">
    /// </field>
    /// <field name="_iter_num" type="Number" integer="true" static="true">
    /// </field>
    this._rand = new ScriptLibrary1._rand();
    this._gaus = new ScriptLibrary1._gaussian();
}
ScriptLibrary1._memoryAlloc.main = function ScriptLibrary1__memoryAlloc$main(args) {
    /// <param name="args" type="Array" elementType="String">
    /// </param>
    ScriptLibrary1._memoryAlloc._iter_num = 40000000;
    var tStart, tEnd;
	tStart = new Date();
    ScriptLibrary1._memoryAlloc._test();
	tEnd = new Date();
	WScript.Echo('end, time= '+(tEnd-tStart)+'ms');

}
ScriptLibrary1._memoryAlloc._test = function ScriptLibrary1__memoryAlloc$_test() {
    var m = new ScriptLibrary1._memoryAlloc();
    m._timeTest(5, 1000, 50, ScriptLibrary1._memoryAlloc._iter_num, 17, 300, 3);
}
ScriptLibrary1._memoryAlloc.prototype = {
    
    _timeTest: function ScriptLibrary1__memoryAlloc$_timeTest(depth, old_data_size, med_data_size, iter_count, mean_alloc_size, med_time, young_time) {
        /// <param name="depth" type="Number" integer="true">
        /// </param>
        /// <param name="old_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="iter_count" type="Number" integer="true">
        /// </param>
        /// <param name="mean_alloc_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_time" type="Number" integer="true">
        /// </param>
        /// <param name="young_time" type="Number" integer="true">
        /// </param>
        this._gaus.initTable(mean_alloc_size, mean_alloc_size / 3);
        this._allocTest(old_data_size, med_data_size, mean_alloc_size);
        this._recurseSteadyState(depth, old_data_size, med_data_size, iter_count, mean_alloc_size, med_time, young_time);
        this._old = null;
        this._med = null;
    },
    
    _recurseSteadyState: function ScriptLibrary1__memoryAlloc$_recurseSteadyState(depth, old_data_size, med_data_size, iter_count, mean_alloc_size, med_time, young_time) {
        /// <param name="depth" type="Number" integer="true">
        /// </param>
        /// <param name="old_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="iter_count" type="Number" integer="true">
        /// </param>
        /// <param name="mean_alloc_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_time" type="Number" integer="true">
        /// </param>
        /// <param name="young_time" type="Number" integer="true">
        /// </param>
        if (!--depth) {
            this._steadyState(old_data_size, med_data_size, iter_count, mean_alloc_size, med_time, young_time);
        }
        else {
            this._recurseSteadyState(depth, old_data_size, med_data_size, iter_count, mean_alloc_size, med_time, young_time);
        }
    },
    
    _allocTest: function ScriptLibrary1__memoryAlloc$_allocTest(old_data_size, med_data_size, mean_alloc_size) {
        /// <param name="old_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="mean_alloc_size" type="Number" integer="true">
        /// </param>
        this._old = new Array(old_data_size);
        this._med = new Array(med_data_size);
        for (var j = 0; j < this._old.length; j++) {
            var k = this._gaus.getRand();
			var a = new Array(k);
            this._old[j] = a;
			for (var x = 0; x < k; x++) {
				a[x]=x;
			}
				
        }
        for (var j = 0; j < this._med.length; j++) {
            var k = this._gaus.getRand();
			var a = new Array(k);
            this._med[j] = a;
			for (var x = 0; x < k; x++) {
				a[x]=x;
			}
        }
    },
    
    _steadyState: function ScriptLibrary1__memoryAlloc$_steadyState(old_data_size, med_data_size, iter_count, mean_alloc_size, med_time, young_time) {
        /// <param name="old_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_data_size" type="Number" integer="true">
        /// </param>
        /// <param name="iter_count" type="Number" integer="true">
        /// </param>
        /// <param name="mean_alloc_size" type="Number" integer="true">
        /// </param>
        /// <param name="med_time" type="Number" integer="true">
        /// </param>
        /// <param name="young_time" type="Number" integer="true">
        /// </param>
        for (var j = 0; j < iter_count; j++) {
            var k = this._gaus.getRand();
            var newarray = new Array(k);
			for (var x = 0; x < k; x++) {
				newarray[x]=x;
			}
            if (!(j % med_time)) {
                var jin = this._rand.getRand(this._old.length);
                this._old[jin] = newarray;
            }
            if (!(j % young_time)) {
                var iin = this._rand.getRand(this._med.length);
                this._med[iin] = newarray;
            }
        }
    },
    
    _old: null,
    _med: null,
    _rand: null,
    _gaus: null
}


////////////////////////////////////////////////////////////////////////////////
// ScriptLibrary1._rand

ScriptLibrary1._rand = function ScriptLibrary1__rand() {
    /// <field name="_x" type="Number" integer="true">
    /// </field>
}
ScriptLibrary1._rand.prototype = {
    _x: 49734321,
    
    internalgetRand: function ScriptLibrary1__rand$internalgetRand() {
        /// <returns type="Number" integer="true"></returns>
        this._x = (314159269 * this._x + 278281) & 2147483647;
        return this._x;
    },
    
    getRand: function ScriptLibrary1__rand$getRand(r) {
        /// <param name="r" type="Number" integer="true">
        /// </param>
        /// <returns type="Number" integer="true"></returns>
		var y = this.getFloat();
		var z = y*r;
        var x = Math.floor(z);
        return x;
    },
    
    getFloat: function ScriptLibrary1__rand$getFloat() {
    // Robert Jenkins' 32 bit integer hash function.
    this._x = ((this._x + 0x7ed55d16) + (this._x << 12))  & 0xffffffff;
    this._x = ((this._x ^ 0xc761c23c) ^ (this._x >>> 19)) & 0xffffffff;
    this._x = ((this._x + 0x165667b1) + (this._x << 5))   & 0xffffffff;
    this._x = ((this._x + 0xd3a2646c) ^ (this._x << 9))   & 0xffffffff;
    this._x = ((this._x + 0xfd7046c5) + (this._x << 3))   & 0xffffffff;
    this._x = ((this._x ^ 0xb55a4f09) ^ (this._x >>> 16)) & 0xffffffff;
    return (this._x & 0xfffffff) / 0x10000000;
    }
}


////////////////////////////////////////////////////////////////////////////////
// ScriptLibrary1._gaussian

ScriptLibrary1._gaussian = function ScriptLibrary1__gaussian() {
    /// <field name="tablE_SIZE" type="Number" integer="true" static="true">
    /// </field>
    /// <field name="_rand" type="ScriptLibrary1._rand">
    /// </field>
    /// <field name="_m_table" type="Array" elementType="Number" elementInteger="true">
    /// </field>
    this._rand = new ScriptLibrary1._rand();
}
ScriptLibrary1._gaussian.prototype = {
    
    getRand: function ScriptLibrary1__gaussian$getRand() {
        /// <returns type="Number" integer="true"></returns>
        return this._m_table[this._rand.getRand(ScriptLibrary1._gaussian.tablE_SIZE)];
    },
    
    initTable: function ScriptLibrary1__gaussian$initTable(m, s) {
        /// <param name="m" type="Number" integer="true">
        /// </param>
        /// <param name="s" type="Number">
        /// </param>
        this._m_table = new Array(ScriptLibrary1._gaussian.tablE_SIZE);
        for (var i = 0; i < ScriptLibrary1._gaussian.tablE_SIZE; i += 2) {
            var x1, x2, w, y1, y2;
            do {
                x1 = 2 * this._rand.getFloat() - 1;
                x2 = 2 * this._rand.getFloat() - 1;
                w = x1 * x1 + x2 * x2;
            } while (w >= 1);
            w = Math.sqrt((-2 * Math.log(w)) / w);
            y1 = x1 * w;
            y2 = x2 * w;
            this._m_table[i] =  Math.floor(Math.max(m + (y1 * s), 1));
            this._m_table[i + 1] =  Math.floor(Math.max(m + (y2 * s), 1));
        }
    },
    
    _m_table: null
}


ScriptLibrary1._memoryAlloc._iter_num = 0;
ScriptLibrary1._gaussian.tablE_SIZE = 1024;

//! This script was generated using Script# v0.7.0.0
ScriptLibrary1._memoryAlloc.main();
