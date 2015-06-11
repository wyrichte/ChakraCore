(function () {
    var extensibilityDoc = (function () {
        function CompletionEvent() {
            /// <field name='scope' type='String'>global | members</field>
            /// <field name='items' type='Array' elementType='CompletionItem'/>
            /// <field name='target' type='Object'/>
            /// <field name='targetName' type='String'/>
            /// <field name='offset' type='Number'/>
            this.items = [new CompletionItem()];
        }
        function CompletionHintEvent() {
            /// <field name='completionItem' type='CompletionItem'/>
            /// <field name='symbolHelp' type='SymbolHelp'/>
        }
        function SignatureHelpEvent() {
            /// <field name='target' type='Object'/>
            /// <field name='parentObject' type='Object'/>
            /// <field name='offset' type='Number'/>
            /// <field name='functionHelp' type='FunctionHelp'/>
            /// <field name='functionComments' type='FunctionComments'/>
        }
        function CompletionItem() {
            /// <field name='name' type='String'/>
            /// <field name='kind' type='String'>method | field | label | property | identifier | parameter | variable | reserved</field>
            /// <field name='scope' type='String'>global | local | parameter | member</field>
            /// <field name='glyph' type='String'>vs:GlyphGroupMethod | vs:GlyphGroupField | vs:GlyphGroupProperty | vs:GlyphGroupClass | ...<para>See StandardGlyphGroup Enumeration: http://msdn.microsoft.com/en-us/library/microsoft.visualstudio.language.intellisense.standardglyphgroup.aspx for all possible values</para></field>
            /// <field name='comment' type='String'/>
            /// <field name='parentObject' type='Object'/>
            /// <field name='value' type='Object'/>
        }
        function SymbolHelp() {
            /// <field name='name' type='String'/>
            /// <field name='symbolType' type='String'/>
            /// <field name='symbolDisplayType' type='String'/>
            /// <field name='elementType' type='String'/>
            /// <field name='scope' type='String'>global | local | parameter | member</field>
            /// <field name='description' type='String'/>
            /// <field name='locid' type='String'/>
            /// <field name='helpKeyword' type='String'/>
            /// <field name='externalFile' type='String'/>
            /// <field name='externalid' type='String'/>
            /// <field name='functionHelp' type='FunctionHelp'/>
        }
        function FunctionHelp() {
            /// <field name='functionName' type='String'/>
            /// <field name='signatures' type='Array' elementType='Signature'/>
            this.signatures = [new Signature()];
        }
        function Signature() {
            /// <field name='description' type='String'/>
            /// <field name='locid' type='String'/>
            /// <field name='helpKeyword' type='String'/>
            /// <field name='externalFile' type='String'/>
            /// <field name='externalid' type='String'/>
            /// <field name='returnValue' type='ReturnValue'/>
            /// <field name='params' type='Array' elementType='Parameter'/>
            this.params = [new Parameter()];
        }
        function ReturnValue() {
            /// <field name='type' type='String'/>
            /// <field name='elementType' type='String'/>
            /// <field name='description' type='String'/>
            /// <field name='locid' type='String'/>
            /// <field name='helpKeyword' type='String'/>
            /// <field name='externalFile' type='String'/>
            /// <field name='externalid' type='String'/>
        }
        function Parameter() {
            /// <field name='name' type='String'/>
            /// <field name='type' type='String'/>
            /// <field name='elementType' type='String'/>
            /// <field name='description' type='String'/>
            /// <field name='locid' type='String'/>
            /// <field name='optional' type='Boolean'/>
        }
        function FunctionComments() {
            /// <field name='inside' type='String'/>
            /// <field name='above' type='String'/>
            /// <field name='paramComments' type='ParamComment'/>
            this.paramComments = [new ParamComment()];
        }
        function ParamComment() {
            /// <field name='name' type='String'/>
            /// <field name='comment' type='String'/>
        }
        return {
            completionEvent: new CompletionEvent(),
            completionHintEvent: new CompletionHintEvent(),
            signatureHelpEvent: new SignatureHelpEvent(),
            paramComment: new ParamComment
        };
    })();

    var addEventListener = intellisense.addEventListener;
    intellisense.addEventListener = function (type, handler) {
        /// <signature>
        /// <param name='type' type='String'>statementcompletion | statementcompletionhint | signaturehelp</param>
        /// <param name='handler' type='Function'/>
        /// </signature>
        addEventListener.apply(this, arguments);
        var argDoc;
        switch (type) {
            case 'statementcompletion': argDoc = extensibilityDoc.completionEvent; break;
            case 'signaturehelp': argDoc = extensibilityDoc.signatureHelpEvent; break;
            case 'statementcompletionhint': argDoc = extensibilityDoc.completionHintEvent; break;
        }
        if (argDoc) setTimeout(function () { handler(argDoc); }, 0);
    };

    var getFunctionComments = intellisense.getFunctionComments;
    intellisense.getFunctionComments = function () {
        /// <signature>
        /// <param name='func' type='Function' />
        /// <returns type='FunctionComments'/>
        /// </signature>
        var result = getFunctionComments.apply(this, arguments);
        result = (result && Object.hasOwnProperty(result, 'inside')) ? result : { inside: '', above: '', paramComments: [extensibilityDoc.paramComment] };
        intellisense.annotate(result, {
            /// <field type='String'/>
            inside: '',
            /// <field type='String'/>
            above: '',
            /// <field type='Array' elementType='ParamComment' />
            paramComments: undefined
        });
        return result;
    };

    intellisense.annotate(intellisense, {
        annotate: function () {
            /// <signature>
            /// <param name='targetObject' type='Object' />
            /// <param name='annotations' type='Object' />
            /// </signature>
        },
        logMessage: function () {
            /// <signature>
            /// <param name='msg' type='String' />
            /// </signature>
        },
        nullWithCompletionsOf: function (type) {
            /// <signature>
            /// <param name='value'/>
            /// </signature>
        },
        undefinedWithCompletionsOf: function (type) {
            /// <signature>
            /// <param name='value'/>
            /// </signature>
        },
        redirectDefinition: function () {
            /// <signature>
            /// <param name='func' type='Function' />
            /// <param name='redirectTo' type='Function' />
            /// </signature>
        }
    });
})();