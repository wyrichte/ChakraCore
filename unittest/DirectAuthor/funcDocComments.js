function FunctionParam() {
    function funcParam(a) {
        /// <param name="a" type="Function">
        ///     <summary>this is a function param</summary>
        ///     <signature>
        ///         <summary>func param signature 2</summary>
        ///         <param name="a2">func param - parameter 1</param>
        ///         <param name="b2">func param - parameter 2</param>
        ///         <returns>func param - return description</returns>
        /// </signature>
        /// </param>
    }
    funcParam(|);
}

function ImplicitSignature() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name="a" type="String">parameter a</param>
        /// <param name="b" type="Number">parameter b</param>
        /// <returns type="String">return description</returns>
    }
    func(|);
}

function ExplicitSingleSignatureSameAsDecl() {
    function func(a, b) {
        /// <signature>
        ///     <summary>explicit signature</summary>
        ///     <param name="a" type="String">parameter a</param>
        ///     <param name="b" type="Number">parameter b</param>
        ///     <returns type="String">return description</returns>
        /// </signature>
    }
    func(|);
}

function ExplicitSignatureNotSameAsDecl() {
    function func(a, b, c) {
        /// <signature>
        ///     <summary>single signature, different from declaration</summary>
        ///     <param name="a" type="String">parameter a</param>
        ///     <returns type="String">return description</returns>
        /// </signature>
    }
    func(|);
}

function MultipleSignatures() {
    function func(p1, p2) {
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name="a1" type="String">parameter 1</param>
        ///     <returns type="String">return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name="a2" type="String">parameter 1</param>
        ///     <param name="b2" type="String">parameter 2</param>
        ///     <returns type="String">return description 2</returns>
        /// </signature>
    }
    func(|);
}

function ImplicitAndMultipleSignatures() {
    function func(p1, p2) {
        /// <summary>implicit signature - should not be seen</summary>
        /// <param name="a" type="String">parameter a</param>
        /// <param name="b" type="Number">parameter b</param>
        /// <returns type="String">return description</returns>
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name="a1" type="String">parameter 1</param>
        ///     <returns type="String">return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name="a2" type="String">parameter 1</param>
        ///     <param name="b2" type="String">parameter 2</param>
        ///     <returns type="String">return description 2</returns>
        /// </signature>
    }
    func(|);
}

function MissingReturns() {
    function missingReturns(p1, p2) {
        /// <signature>
        ///     <summary>missing returns 1</summary>
        ///     <param name="a1" type="String">parameter 1</param>
        /// </signature>
        /// <signature>
        ///     <summary>missing returns 2</summary>
        ///     <param name="a2" type="String">parameter 1</param>
        ///     <param name="b2" type="String">parameter 2</param>
        /// </signature>
    }
    missingReturns(|);
}

function MissingType() {
    function missingType(p1, p2) {
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name="a1">parameter 1</param>
        ///     <returns>return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name="a2">parameter 1</param>
        ///     <param name="b2">parameter 2</param>
        ///     <returns>return description 2</returns>
        /// </signature>
    }
    missingType(|);
}

function MissingParamName() {
    function missingParamName(a, b) {
        /// <param type="String">parameter a</param>
    }
    missingParamName(|);
}

function MissingReturnsDescr() {
    function missingReturnDescr(a, b) {
        /// <returns type="Number" />
    }
    missingReturnDescr(|);
}

function Invalid content() {
    function invalidContent(a, b) {
        /// some text
    }
    invalidContent(|);
}

function MissingSignatureClosingTag() {
    function missingSignatureClosingTag(a, b) {
        /// <signature>
        ///     <summary>missing signature closing tag</summary>
        ///     <param name="a" type="String">parameter a</param>
        ///     <param name="b" type="Number">parameter b</param>
        ///     <returns type="String">return description</returns>
    }
    missingSignatureClosingTag(|);
}

function InvalidTags() {
    function invalidTag(a, b) {
        /// <summary>implicit signature</summary>
        /// <sometag />
        /// <param name="a" type="String">parameter a</param>
        /// <param name="b" type="Number">parameter b</param>
        /// <returns type="String">return description</returns>
    }
    invalidTag(|);
}

function EmptyParamTags() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name="a" type="String" />
        /// <param name="b" type="Number" />
        /// <returns type="String">return description</returns>
    }
    func(|);
}

function IgnoreComments() {
    function func(a) {
        /// <signature>
        ///     <!-- XML comments are silently skipped outside root elements-->
        ///     <summary>summary text </summary>
        ///     <param name="a" type="Number"><!-- XML comments are ignored --> param description</param>
        ///     <returns>return <!-- XML comments are still ignored --> description</returns>
        /// </signature>
    }
    func(|);
}

function IgnoreProcessingInstructions() {
    function func(a) {
        /// <signature>
        ///     <summary>summary <?processing instructions Ignored?> text </summary>
        ///     <param name="a" type="Number"><?this processing instruction too?> param description</param>
        /// </signature>
    }
    func(|);
}

function PreserveCDATA() {
    function func(a) {
        /// <signature>
        ///     <![CDATA[CDATA tags and thier content skipped]]>
        ///     <summary>summary text <![CDATA[CDATA tags preserved]]> </summary>
        ///     <param name="a" type="Number"><![CDATA[CDATA tags are still preserved]]> param description</param>
        /// </signature>
    }
    func(|);
}

function PreserveFormating() {
    function func(a) {
        /// <signature>
        ///     <summary> summary
        ///               text
        ///               on multiple lines   
        ///         </summary>
        ///     <param name="a" type="Number">
        ///
        ///
        ///              Param           Description              </param>
        ///     <returns>return                description </returns>
        /// </signature>
    }
    func(|);
}

function UnknownContent() {
    function func(a) {
        /// <signature xmlns:foo="http://w">
        ///     <summary xml:space="preserve">summary text:<doccomment>content inside an unknown element<para>with a paragraph</para>
        ///     and some more <d><e><f>unknown</f></e></d></doccomment>tags</summary>
        ///     <param name="a" type="Number"><foo:bold>param description</foo:bold></param>
        ///     <returns unknownAttribute="some value"><AnotherTag name="value"/>return description</returns>
        /// </signature>
    }
    func(|);
}

function ParameterNameMatching() {
    function func(a1, b2) {
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name="  a1   ">parameter 1</param>
        ///     <returns>return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name=" a2        ">parameter 1</param>
        ///     <param name="                                    b2">parameter 2</param>
        ///     <returns>return description 2</returns>
        /// <signature>
        ///     <summary>signature 3</summary>
        ///     <param name="b2">parameter 2</param>
        ///     <returns>return description 2</returns>
        /// </signature>
    }
    func(|);
}

function ParameterMatching2() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name="a d" type="String">param name does not match and should not show in output</param>
        /// <param name=" b        c " type="Number">param name does not match and should not show in output</param>
        /// <returns type="String">return description</returns>
    }
    func(|);
}