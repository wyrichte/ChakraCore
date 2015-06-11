function Foo()
{
    ///<field name="bar" type="Foo" mayBeNull="true"></field> 
    this.bar = null;
}

Foo.prototype.test = function () { 
  this./**ml:bar**/
} 