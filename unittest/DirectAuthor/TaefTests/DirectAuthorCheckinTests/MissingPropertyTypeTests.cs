using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class MissingPropertyTypeTests : CompletionsBase
    {
        [TestInitialize]
        new public void Initialize()
        {
            base.Initialize();
        }

        [TestCleanup]
        new public void Cleanup()
        {
            base.Cleanup();
        }

        [TestMethod]
        public void MissingConstructorField()
        {
            PerformCompletionRequests(
                @"function Point(x, y) {
                    /// <field name='x' type='Number' />
                    /// <field name='y' type='Number' />
                  };

                  var p = new Point();
                  p.|x,y|;
                  p.x.|Number|;
                  p.y.|Number|;");
        }

        [TestMethod]
        public void UnsetFieldsStillUndefined()
        {
            PerformCompletionRequests(
                @"function Point(x, y) {
                    /// <field name='x' type='Number' />
                    /// <field name='y' type='Number' />
                  };

                  var p = new Point();
                  var b = typeof p.x === 'undefined' ? '' : 1;
                  b.|String|;");
        }

        [TestMethod]
        public void DeletedField()
        {
            PerformCompletionRequests(
                @"function Point(x, y) {
                    /// <field name='x' type='Number' />
                    /// <field name='y' type='Number' />
                    this.x = x;
                    this.y = y;
                  };

                  var p = new Point();
                  delete p.x;
                  p.x.|Number|;");
        }

        [TestMethod]
        [Ignore]
        public void MissingElementLiteral()
        {
            PerformCompletionRequests(
                @"var p = {
                    /// <field name='x' type='Number' />
                    /// <field name='y' type='Number' />
                  };
                  p.|x,y|;
                  p.x.|Number|;
                  p.y.|Number|;");
        }

        [TestMethod]
        public void MissingParameter()
        {
            PerformCompletionRequests(
                @"function foo(x, y) {
                    /// <param name='x' type='Number' />
                    /// <param name='y' type='Number' />
                    x.|Number|;
                    y.|Number|;
                  }");
        }

        [TestMethod]
        public void MissingVariableGlobal()
        {
            PerformCompletionRequests(
                @"/// <var name='x' type='Number' />
                  var x;
                  x.|Number|;");
        }

        [TestMethod]
        public void MissingVariableLocal()
        {
            PerformCompletionRequests(
                @"function foo() {
                    /// <var name='x' type='Number' />
                    var x;
                    x.|Number|;
                  }");
        }

        [TestMethod]
        public void MissingElemmentGlobal()
        {
            PerformCompletionRequests(
                @"/// <var name='arr' type='Array' elementType='Number' />
                  var a = [];
                  a[5].|Number|;
                  var b = a[5];
                  b.|Number|;");

        }

        [TestMethod]
        public void MissingElementLocal()
        {
            PerformCompletionRequests(
                @"function foo() {
                      /// <var name='arr' type='Array' elementType='Number' />
                      var a = [];
                      a[5].|Number|;
                      var b = a[5];
                      b.|Number|;
                  }");
        }

        [TestMethod]
        public void MissingElementFieldConstructor()
        {
            PerformCompletionRequests(
                @"function C() {
                    /// <field name='arr' type='Array' elementType='Number' />
                  };
                  var o = new C();
                  o.|arr|;
                  o.arr[5].|Number|;
                  var n = o.arr[5];
                  n.|Number|;");
        }

        [TestMethod]
        public void MissingParameterArrays()
        {
            PerformCompletionRequests(
                @"function testFoo(a,d) { 
                    /// <param name='a' type='Int16Array' elementType='Number'>param a</param> 
                    /// <param name='d' type='Array' elementType='Number'>param a</param> 
                    /// <returns type='Array' elementType='String'>return document type</returns>
                    
                    a[1].|Number|;
                    d[1].|Number|;
                }

                var b = testFoo();
                b[5].|String|;");
        }

        [TestMethod]
        [Ignore] // enable when bug 301415 is fixed
        public void MissingElementFieldLiteral()
        {
            PerformCompletionRequests(
                @"var o = {
                    /// <field name='arr' type='Array' elementType='Number' />
                  };
                  o.|arr|;
                  o.arr[5].|Number|;
                  var n = o.arr[5];
                  n.|Number|;");
        }

        [TestMethod]
        public void AssignedPrototypeNull()
        {
            PerformCompletionRequests(
                @"/// <reference name=''MicrosoftAjax.js''/>
                  function Person() {
                      /// <field name='Age' type='Number'>Age of person.</field> 
                  }
                  Person.prototype.Age = null;
                  Person.registerClass('Person');
                  var p = new Person();
                  var age = p.Age;
                  age.|Number|");

        }

        [TestMethod]
        public void LiteralFieldDeclarations()
        {
            PerformCompletionRequests(
                @"/// <var type='Number'>vraiable arr</var>
                var arr = {
                    /// <field name='width' type='Number'>this is width</field>  
                    width: 9,
                    /// <field name='arrBan' type='Array' elementType='Number'>this is array banana</field>
                    arrBan:[]
                };

                arr.arrBan[1].|Number|;

                /// <var type='Array' elementType='Number'></var>
                var a;
                a.|Array|;
                a[5].|Number|;");
        }
    }
}
