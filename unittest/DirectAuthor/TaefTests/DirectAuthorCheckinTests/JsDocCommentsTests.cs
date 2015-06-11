namespace DirectAuthorCheckinTests
{
    using System.Linq;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    //
    // Be careful with the overwritten ParseRequest method.
    // Make sure you read the comments there before adding new test cases with the '`' operator
    //
    // When adding a test method to this class, include the same test method
    // to the derived class, JsDocCommentsWithExtensionsTests.
    //
    [TestClass]
    public class JsDocCommentsTests : CompletionHintTests
    {
        #region Completion Tests

        [TestMethod]
        public void JsDocTypeTag_Completion()
        {
            TestCompletionHint(@"
                    /**
                     * @type {Number}
                     */
                    var x;

                    x.|
                ",
             "toExponential", hint =>
             {
                 Assert.IsNotNull(hint);
             });
        }

        [TestMethod]
        public void JsDocReturnsTag_Completion()
        {
            TestCompletionHint(@"
                    /**
                     * @returns {Animal} animal description
                     */
                    function Animal()
                    { 
                      this.legs = 4; 
                    }

                    var animal = new Animal();|
                ",
             "animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Animal");
                 hint.Description.Expect("animal description");
             });
        }

        [TestMethod]
        public void JsDocReturnsTagMultipleTypes_Completion()
        {
            TestCompletionHint(@"
                    /**
                     * @returns {Animal`Fruit} animal description
                     */
                    function Animal()
                    { 
                      this.legs = 4; 
                    }

                    var animal = new Animal();|
                ",
             "animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Animal");
                 hint.Description.Expect("animal description");
             });
        }

        [TestMethod]
        public void JsDocTypeTagOnGlobalVariable_Completion()
        {
            TestCompletionHint(@"
                    /**
                     * @type {Apple}
                     */
                    var apple = new Apple();|
                ",
             "apple", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Apple");
             });
        }

        [TestMethod]
        public void JsDocPropertyTagOnVarDeclField_Completion()
        {
            TestCompletionHint(@"
                    /**
                     * @property {Orange} orange - orange is orange
                     */
                    var fruitbasket = {
                      orange: new Orange()
                    }
                    fruitbasket.|
                ",
             "orange", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Orange");
                 hint.Description.Expect("orange is orange");
             });
        }

        [TestMethod]
        public void JsDocMultipleMatchingPropertyTagsOnVarDeclField_Completion()
        {
            string source = @"
                    /**
                     * @property {Orange} orange - orange is orange
                     * @property {Apple} apple - apple is apple
                     */
                    var fruitbasket = {
                      orange: new Orange(),
                      apple: new Apple()
                    }
                    fruitbasket.|
                ";
            TestCompletionHint(source,
             "orange", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Orange");
                 hint.Description.Expect("orange is orange");
             });
            TestCompletionHint(source,
             "apple", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Apple");
                 hint.Description.Expect("apple is apple");
             });
        }

        [TestMethod]
        public void JsDocPropertyTagOnAssignmentField_Completion()
        {
            TestCompletionHint(@"
                    var fruitbasket = [];
                    /**
                     * @property {Orange} orange - orange is orange
                     */
                    fruitbasket = {
                      orange: new Orange()
                    }
                    fruitbasket.|
                ",
             "orange", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Orange");
                 hint.Description.Expect("orange is orange");
             });
        }

        [TestMethod]
        public void JsDocMismatchPropertyTagOnAssignmentField_Completion()
        {
            TestCompletionHint(@"
                    var fruitbasket = [];
                    /**
                     * @property {Apple} apple - apple is apple
                     */
                    fruitbasket = {
                      orange: new Orange()
                    }
                    fruitbasket.|
                ",
             "orange", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Description.Expect(null);
             });
        }


        [TestMethod]
        public void JsDocPropertyTagOnFunctionField_Completion()
        {
            TestCompletionHint(@"  
                    /**
                     * @property {Number} pear - Pear is green
                     */
                    function Food()
                    {
                        this.pear = 12;
                    }

                    /**
                     * @param {Food} food - food to eat
                     */
                    function eat(food)
                    {
                        food.|;
                    }
                ",
                "pear", hint =>
                {
                    hint.TypeName.Expect("Number");
                    hint.Description.Expect("Pear is green");
                    hint.Scope.Expect(AuthorScope.ascopeMember);
                    hint.Type.Expect(AuthorType.atNumber);
                });
        }

        [TestMethod]
        public void JsDocMultiplePropertyTagOnFunctionField_Completion()
        {
            TestCompletionHint(@"  
                    /**
                     * @property {Number} papaya - Papaya is large
                     * @property {String} avocado - Avocado originated from Mexico
                     */
                    function Food()
                    {
                        this.papaya = 12;
                        this.avocado = 24;
                    }

                    /**
                     * @param {Food} food - food to eat
                     */
                    function eat(food)
                    {
                        food.|;
                    }
                ",
                "avocado", hint =>
                {
                    hint.TypeName.Expect("String");
                    hint.Description.Expect("Avocado originated from Mexico");
                    hint.Scope.Expect(AuthorScope.ascopeMember);
                    hint.Type.Expect(AuthorType.atNumber);
                });
        }

        [TestMethod]
        public void JsDocParamTag_Completion()
        {
            PerformRequests(@"
                /**
                 * @param {String} banana Banana is yellow
                 */
                function test(banana)
                {
                    banana|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("banana");
                 hint.Description.Expect("Banana is yellow");
                 hint.TypeName.Expect("String");
             });
        }

        [TestMethod]
        public void JsDocParamTagWithHyphen_Completion()
        {
            PerformRequests(@"
                /**
                 * @param {String} banana - Banana is yellow
                 */
                function test(banana)
                {
                    banana|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("banana");
                 hint.Description.Expect("Banana is yellow");
                 hint.TypeName.Expect("String");
             });
        }

        [TestMethod]
        public void JsDocParamTagWithMultipleLinesDescription_Completion()
        {
            PerformRequests(@"
                /**
                 * @param {String} banana - Banana is yellow
                 * This is second line of the param description
                 */
                function test(banana)
                {
                    banana|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("banana");
                 hint.Description.Expect("Banana is yellow\r\nThis is second line of the param description");
                 hint.TypeName.Expect("String");
             });
        }


        #endregion

        #region GetFunctionHelp Tests

        [TestMethod]
        public void JsDocParamTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * @param {String} a text
                 */
                function f1(a)
                {
                }
                f1|;
            }",
            "f1",
            hint =>
            {
                Assert.IsNotNull(hint);
                hint.Type.Expect(AuthorType.atFunction);
                hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetParameters().ToEnumerable().Single().Description.Expect("text");
            });
        }

        [TestMethod]
        public void JsDocReturnTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * @return {Number} the data
                 */
                function f1(a)
                {
                }
                f1|;
            }",
            "f1",
            hint =>
            {
                hint.Type.Expect(AuthorType.atFunction);
                hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetReturnValue().Description.Expect("the data");
            });
        }

        [TestMethod]
        public void JsDocReturnsTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * @returns {Number} the data
                 */
                function f1(a)
                {
                }
                f1|;
            }",
            "f1",
            hint =>
            {
                hint.Type.Expect(AuthorType.atFunction);
                hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetReturnValue().Description.Expect("the data");
            });
        }

        [TestMethod]
        public void JsDocDeprecatedTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * @deprecated danger pomegranate are around!
                 */
                function odorfruit(a)
                {
                }
                odorfruit|;
            }",
            "odorfruit",
            hint =>
            {
                hint.Type.Expect(AuthorType.atFunction);
                hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetDeprecated().Message.Expect("danger pomegranate are around!");
            });
        }

        [TestMethod]
        public void JsDocReturnsTagMultipleTypes_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * @returns {Animal`Fruit} animal description
                     */
                    function Animal()
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 var signatures = hint.GetFunctionHelp().GetSignatures().ToEnumerable().ToArray();
                 Assert.AreEqual<int>(2, signatures.Length);
                 signatures[0].GetReturnValue().Type.Expect("Animal");
                 signatures[0].GetReturnValue().Description.Expect("animal description");
                 signatures[1].GetReturnValue().Type.Expect("Fruit");
                 signatures[1].GetReturnValue().Description.Expect("animal description");
             });
        }

        [TestMethod]
        public void JsDocParamTagMultipleTypes_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * @param {Apple`Orange} food
                     */
                    function Animal(food)
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 var signatures = hint.GetFunctionHelp().GetSignatures().ToEnumerable().ToArray();
                 Assert.AreEqual<int>(2, signatures.Length);
                 signatures[0].GetParameters().ToEnumerable().Single().Type.Except("Apple");
                 signatures[1].GetParameters().ToEnumerable().Single().Type.Except("Orange");
             });
        }

        [TestMethod]
        public void JsDocParamTagAndReturnsTagMultipleTypes_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * Build an animal with food.
                     * @param {Food} food - food description
                     * @returns {Animal`Fruit} animal description
                     */
                    function Animal(food)
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 var signatures = hint.GetFunctionHelp().GetSignatures().ToEnumerable().ToArray();
                 Assert.AreEqual<int>(2, signatures.Length);
                 signatures[0].Description.Expect("Build an animal with food.");
                 signatures[0].GetParameters().ToEnumerable().Single().Type.Expect("Food");
                 signatures[0].GetParameters().ToEnumerable().Single().Description.Expect("food description");
                 signatures[0].GetReturnValue().Type.Expect("Animal");
                 signatures[0].GetReturnValue().Description.Expect("animal description");
                 signatures[1].Description.Expect("Build an animal with food.");
                 signatures[1].GetParameters().ToEnumerable().Single().Type.Expect("Food");
                 signatures[1].GetParameters().ToEnumerable().Single().Description.Expect("food description");
                 signatures[1].GetReturnValue().Type.Expect("Fruit");
                 signatures[1].GetReturnValue().Description.Expect("animal description");
             });
        }

        [TestMethod]
        public void JsDocDescriptionOnly_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * This is an Animal.
                     */
                    function Animal(food)
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().Description.Expect("This is an Animal.");
             });
        }

        [TestMethod]
        public void JsDocDescriptionTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * @description This is an Animal.
                     */
                    function Animal(food)
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().Description.Expect("This is an Animal.");
             });
        }

        [TestMethod]
        public void JsDocSummaryTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * @summary This is an Animal.
                     */
                    function Animal(food)
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().Description.Expect("This is an Animal.");
             });
        }

        [TestMethod]
        public void JsDocDescriptionOnlyWithErrorTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                    /**
                     * This is an Animal.
                     * @unknown this should fail
                     */
                    function Animal(food)
                    { 
                      this.legs = 4; 
                    }

                    Animal|;
                ",
             "Animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atFunction);
                 hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().Description.Expect("This is an Animal.");
             });
        }

        [TestMethod]
        public void JsDocParamTagWithErrorTag_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * @param {String } a - first parameter
                 * @unknown this should fail
                 * @param { Number} b - second parameter
                 */
                function f1(a, b)
                {
                }
                f1|;
            }",
            "f1",
            hint =>
            {
                Assert.IsNotNull(hint);
                hint.Type.Expect(AuthorType.atFunction);
                var parameters = hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetParameters().ToEnumerable().ToArray();
                parameters[0].Type.Expect("String");
                parameters[0].Description.Expect("first parameter");
                parameters[1].Type.Expect("Number");
                parameters[1].Description.Expect("second parameter");
            });
        }

        [TestMethod]
        public void JsDocOptionalParam_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * Say hello
                 * @param {String} name - The person you wanted to say hello to.
                 * @param {String= } greeting - What do you want to greet with
                 */
                function sayHello(name, greeting)
                {
                }

                sayHello|
            }",
            "sayHello",
            hint =>
            {
                Assert.IsNotNull(hint);
                hint.Type.Expect(AuthorType.atFunction);
                var parameters = hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetParameters().ToEnumerable().ToArray();
                parameters[0].Type.Expect("String");
                Assert.IsFalse(parameters[0].Optional);
                parameters[1].Type.Expect("String");
                Assert.IsTrue(parameters[1].Optional);
            });
        }

        [TestMethod]
        public void JsDocStarType_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * Add some object value
                 * @param { * } item - The item to add
                 */
                function add(item)
                {
                }

                add|
            }",
            "add",
            hint =>
            {
                Assert.IsNotNull(hint);
                hint.Type.Expect(AuthorType.atFunction);
                var parameters = hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetParameters().ToEnumerable().ToArray();
                Assert.IsTrue(parameters[0].Type.Equals("Object"));
            });
        }


        [TestMethod]
        public void JsDocFunctionType_GetFunctionHelp()
        {
            TestCompletionHint(@"
                /**
                 * Add some object value
                 * @param {Function} item1 - The item 1 to add
                 * @param {function} item2 - The item 2 to add
                 * @param {function(string, number)} item3 - The item 3 to add
                 * @param {function (string, number)} item4 - The item 4 to add
                 * @param {function" + '\t' + @"(string, number)} item5 - The item 5 to add
                 */
                function add(item1, item2, item3, item4, item5)
                {
                }

                add|
            }",
            "add",
            hint =>
            {
                Assert.IsNotNull(hint);
                hint.Type.Expect(AuthorType.atFunction);
                var parameters = hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().GetParameters().ToEnumerable().ToArray();
                Assert.IsTrue(parameters[0].Type.Equals("Function"));
                Assert.IsTrue(parameters[1].Type.Equals("Function"));
                Assert.IsTrue(parameters[2].Type.Equals("Function"));
                Assert.IsTrue(parameters[3].Type.Equals("Function"));
                Assert.IsTrue(parameters[4].Type.Equals("Function"));
            });
        }

        #endregion

        // The pipe operator is used for delineating requests, but it is also heavily used in JSDoc to separate multiple types
        // Therefore the operator '`' is used in the request instead, this will make the request parsing logic happy.
        // However, before sending the request to the language service implementation, we will need to replace the text back.
        // It is important that in this test, the operator '`' is not used since it will be unconditionally replaced back 
        // to | here
        protected override ParsedRequests ParseRequests(string text)
        {
            ParsedRequests beforeReplacement = base.ParseRequests(text);
            if (beforeReplacement.Text != null)
            {
                beforeReplacement.Text = beforeReplacement.Text.Replace("`", "|");
            }

            return beforeReplacement;
        }
    }
}
