using System;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    [TestClass]
    public class JQueryGeneratedTests : JQueryBase
    {
        /*
         *     Note: This file is auto-generated. If you need to change any of the tests, or their attributes
         *           please update the script that generates it at CreateJQueryTests\Program.cs
         */

        [TestMethod]
        public void event_delegateTarget_Example1()
        {
            // Test Block 1.
            // Entry event.delegateTarget: The element where the currently-called jQuery event handler was attached.
            // Example 1: When a button in any box class is clicked, change the box's background color to red.
            PerformJQueryTest(@"
                $("".box"").on(""click"", ""button"", function(event) {
                  $(event.delegateTarget).css(""background-color"", ""red"");
                });  
            ", "delegateTarget");
        }

        [TestMethod]
        public void off_Example1()
        {
            // Test Block 2.
            // Entry off: Remove an event handler.
            // Example 1: Add and remove event handlers on the colored button.
            PerformJQueryTest(@"
                function aClick() {
                  $(""div"").show().fadeOut(""slow"");
                }
                $(""#bind"").click(function () {
                  $(""body"").on(""click"", ""#theone"", aClick)
                    .find(""#theone"").text(""Can Click!"");
                });
                $(""#unbind"").click(function () {
                  $(""body"").off(""click"", ""#theone"", aClick)
                    .find(""#theone"").text(""Does nothing..."");
                });
            ");
        }

        [TestMethod]
        public void off_Example2()
        {
            // Test Block 3.
            // Entry off: Remove an event handler.
            // Example 2: Remove all event handlers from all paragraphs:
            PerformJQueryTest(@"
                $(""p"").off()
            ");
        }

        [TestMethod]
        public void off_Example3()
        {
            // Test Block 4.
            // Entry off: Remove an event handler.
            // Example 3: Remove all delegated click handlers from all paragraphs:
            PerformJQueryTest(@"
                $(""p"").off( ""click"", ""**"" )
            ");
        }

        [TestMethod]
        public void off_Example4()
        {
            // Test Block 5.
            // Entry off: Remove an event handler.
            // Example 4: Remove just one previously bound handler by passing it as the third argument:
            PerformJQueryTest(@"
                var foo = function () {
                  // code to handle some kind of event
                };
                // ... now foo will be called when paragraphs are clicked ...
                $(""body"").on(""click"", ""p"", foo);
                // ... foo will no longer be called.
                $(""body"").off(""click"", ""p"", foo); 
            ");
        }

        [TestMethod]
        public void off_Example5()
        {
            // Test Block 6.
            // Entry off: Remove an event handler.
            // Example 5: Unbind all delegated event handlers by their namespace:
            PerformJQueryTest(@"
                var validate = function () {
                  // code to validate form entries
                };
                // delegate events under the "".validator"" namespace
                $(""form"").on(""click.validator"", ""button"", validate);
                $(""form"").on(""keypress.validator"", ""input[type='text']"", validate); 
                // remove event handlers in the "".validator"" namespace
                $(""form"").off("".validator"");
            ");
        }

        [TestMethod]
        public void on_Example1()
        {
            // Test Block 7.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 1: Display a paragraph's text in an alert when it is clicked:
            PerformJQueryTest(@"
                $(""p"").on(""click"", function(){
                alert( $(this).text() );
                });
            ");
        }

        [TestMethod]
        public void on_Example2()
        {
            // Test Block 8.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 2: Pass data to the event handler, which is specified here by name:
            PerformJQueryTest(@"
                function myHandler(event) {
                alert(event.data.foo);
                }
                $(""p"").on(""click"", {foo: ""bar""}, myHandler)
            ");
        }

        [TestMethod]
        public void on_Example3()
        {
            // Test Block 9.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 3: Cancel a form submit action and prevent the event from bubbling up by returning false:
            PerformJQueryTest(@"
                $(""form"").on(""submit"", false)
            ");
        }

        [TestMethod]
        public void on_Example4()
        {
            // Test Block 10.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 4: Cancel only the default action by using .preventDefault().
            PerformJQueryTest(@"
                $(""form"").on(""submit"", function(event) {
                  event.preventDefault();
                });
            ");
        }

        [TestMethod]
        public void on_Example5()
        {
            // Test Block 11.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 5: Stop submit events from bubbling without preventing form submit, using .stopPropagation().
            PerformJQueryTest(@"
                $(""form"").on(""submit"", function(event) {
                  event.stopPropagation();
                });
            ");
        }

        [TestMethod]
        public void on_Example6()
        {
            // Test Block 12.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 6: Attach and trigger custom (non-browser) events.
            PerformJQueryTest(@"
                $(""p"").on(""myCustomEvent"", function(e, myName, myValue){
                  $(this).text(myName + "", hi there!"");
                  $(""span"").stop().css(""opacity"", 1)
                    .text(""myName = "" + myName)
                    .fadeIn(30).fadeOut(1000);
                });
                $(""button"").click(function () {
                  $(""p"").trigger(""myCustomEvent"", [ ""John"" ]);
                });
            ");
        }

        [TestMethod]
        public void on_Example7()
        {
            // Test Block 13.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 7: Attach multiple event handlers simultaneously using a map.
            PerformJQueryTest(@"
                $(""div.test"").on({
                  click: function(){
                    $(this).addClass(""active"");
                  },
                  mouseenter: function(){
                    $(this).addClass(""inside"");
                  },
                  mouseleave: function(){
                    $(this).removeClass(""inside"");
                  }
                });
            ");
        }

        [TestMethod]
        public void on_Example8()
        {
            // Test Block 14.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 8: Click any paragraph to add another after it. Note that .on() allows a click event on any paragraph--even new ones--since the event is handled by the ever-present body element after it bubbles to there.
            PerformJQueryTest(@"
                    var count = 0;
                    $(""body"").on(""click"", ""p"", function(){
                      $(this).after(""<p>Another paragraph! ""+(++count)+""</p>"");
                    });
            ");
        }

        [TestMethod]
        public void on_Example9()
        {
            // Test Block 15.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 9: Display each paragraph's text in an alert box whenever it is clicked:
            PerformJQueryTest(@"
                $(""body"").on(""click"", ""p"", function(){
                  alert( $(this).text() );
                });
            ");
        }

        [TestMethod]
        public void on_Example10()
        {
            // Test Block 16.
            // Entry on: Attach an event handler function for one or more events to the selected elements.
            // Example 10: Cancel a link's default action using the preventDefault method.
            PerformJQueryTest(@"
                $(""body"").on(""click"", ""a"", function(event){
                  event.preventDefault();
                });
            ", "preventDefault");
        }

        [TestMethod]
        public void jQuery_isNumeric_Example1()
        {
            // Test Block 17.
            // Entry jQuery.isNumeric: Determines whether its argument is a number.
            // Example 1: Sample return values of $.isNumeric with various inputs.
            PerformJQueryTest(@"
                $.isNumeric(""-10"");  // true
                $.isNumeric(16);     // true
                $.isNumeric(0xFF);   // true
                $.isNumeric(""0xFF""); // true
                $.isNumeric(""8e5"");  // true (exponential notation string)
                $.isNumeric(3.1415); // true
                $.isNumeric(+10);    // true
                $.isNumeric(0144);   // true (octal integer literal)
                $.isNumeric("""");     // false
                $.isNumeric({});     // false (empty object)
                $.isNumeric(NaN);    // false
                $.isNumeric(null);   // false
                $.isNumeric(true);   // false
                $.isNumeric(Infinity); // false
                $.isNumeric(undefined); // false
            ");
        }

        [TestMethod]
        public void focus_Example1()
        {
            // Test Block 18.
            // Entry focus: Selects element if it is currently focused.
            // Example 1: Adds the focused class to whatever element has focus
            PerformJQueryTest(@"
                $( ""#content"" ).delegate( ""*"", ""focus blur"", function( event ) {
                    var elem = $( this );
                    setTimeout(function() {
                       elem.toggleClass( ""focused"", elem.is( "":focus"" ) );
                    }, 0);
                });
            ");
        }

        [TestMethod]
        public void deferred_pipe_Example1()
        {
            // Test Block 19.
            // Entry deferred.pipe:  Utility method to filter and/or chain Deferreds.  
            // Example 1: Filter resolve value:
            PerformJQueryTest(@"
                var defer = $.Deferred(),
                    filtered = defer.pipe(function( value ) {
                      return value * 2;
                    });
                defer.resolve( 5 );
                filtered.done(function( value ) {
                  alert( ""Value is ( 2*5 = ) 10: "" + value );
                });
            ");
        }

        [TestMethod]
        public void deferred_pipe_Example2()
        {
            // Test Block 20.
            // Entry deferred.pipe:  Utility method to filter and/or chain Deferreds.  
            // Example 2: Filter reject value:
            PerformJQueryTest(@"
                var defer = $.Deferred(),
                    filtered = defer.pipe( null, function( value ) {
                      return value * 3;
                    });
                defer.reject( 6 );
                filtered.fail(function( value ) {
                  alert( ""Value is ( 3*6 = ) 18: "" + value );
                });
            ");
        }

        [TestMethod]
        public void deferred_pipe_Example3()
        {
            // Test Block 21.
            // Entry deferred.pipe:  Utility method to filter and/or chain Deferreds.  
            // Example 3: Chain tasks:
            PerformJQueryTest(@"
                var request = $.ajax( url, { dataType: ""json"" } ),
                    chained = request.pipe(function( data ) {
                      return $.ajax( url2, { data: { user: data.userId } } );
                    });
                chained.done(function( data ) {
                  // data retrieved from url2 as provided by the first request
                });
            ", "url", "url2", "userId");
        }

        [TestMethod]
        public void deferred_always_Example1()
        {
            // Test Block 22.
            // Entry deferred.always:  Add handlers to be called when the Deferred object is either resolved or rejected. 
            // Example 1: Since the jQuery.get() method returns a jqXHR object, which is derived from a Deferred object, we can attach a callback for both success and error using the deferred.always() method.
            PerformJQueryTest(@"
                $.get(""test.php"").always( function() { 
                  alert(""$.get completed with success or error callback arguments""); 
                } );
            ");
        }

        [TestMethod]
        public void promise_Example1()
        {
            // Test Block 23.
            // Entry promise:  Return a Promise object to observe when all actions of a certain type bound to the collection, queued or not, have finished. 
            // Example 1: Using .promise() on a collection with no active animation returns a resolved Promise:
            PerformJQueryTest(@"
                var div = $( ""<div />"" );
                div.promise().done(function( arg1 ) {
                  // will fire right away and alert ""true""
                  alert( this === div && arg1 === div );
                });
            ");
        }

        [TestMethod]
        public void promise_Example2()
        {
            // Test Block 24.
            // Entry promise:  Return a Promise object to observe when all actions of a certain type bound to the collection, queued or not, have finished. 
            // Example 2: Resolve the returned Promise when all animations have ended (including those initiated in the animation callback or added later on):
            PerformJQueryTest(@"
                $(""button"").bind( ""click"", function() {
                  $(""p"").append( ""Started..."");
                  $(""div"").each(function( i ) {
                    $( this ).fadeIn().fadeOut( 1000 * (i+1) );
                  });
                  $( ""div"" ).promise().done(function() {
                    $( ""p"" ).append( "" Finished! "" );
                  });
                });
            ");
        }

        [TestMethod]
        public void promise_Example3()
        {
            // Test Block 25.
            // Entry promise:  Return a Promise object to observe when all actions of a certain type bound to the collection, queued or not, have finished. 
            // Example 3: Resolve the returned Promise using a $.when() statement (the .promise() method makes it possible to do this with jQuery collections):
            PerformJQueryTest(@"
                var effect = function() {
                  return $(""div"").fadeIn(800).delay(1200).fadeOut();
                };
                $(""button"").bind( ""click"", function() {
                  $(""p"").append( "" Started... "");
                  $.when( effect() ).done(function() {
                    $(""p"").append("" Finished! "");
                  });
                });
            ");
        }

        [TestMethod]
        public void removeProp_Example1()
        {
            // Test Block 26.
            // Entry removeProp: Remove a property for the set of matched elements.
            // Example 1: Set a numeric property on a paragraph and then remove it. 
            PerformJQueryTest(@"
                var $para = $(""p"");
                $para.prop(""luggageCode"", 1234);
                $para.append(""The secret luggage code is: "", String($para.prop(""luggageCode"")), "". "");
                $para.removeProp(""luggageCode"");
                $para.append(""Now the secret luggage code is: "", String($para.prop(""luggageCode"")), "". "");
            ");
        }

        [TestMethod]
        public void prop_Example1()
        {
            // Test Block 27.
            // Entry prop: Get the value of a property for the first element in the set of matched elements.
            // Example 1: Display the checked property and attribute of a checkbox as it changes.
            PerformJQueryTest(@"
                $(""input"").change(function() {
                  var $input = $(this);
                  $(""p"").html("".attr('checked'): <b>"" + $input.attr('checked') + ""</b><br>""
                              + "".prop('checked'): <b>"" + $input.prop('checked') + ""</b><br>""
                              + "".is(':checked'): <b>"" + $input.is(':checked') ) + ""</b>"";
                }).change();
            ");
        }

        [TestMethod]
        public void prop_1_Example1()
        {
            // Test Block 28.
            // Entry prop_1: Set one or more properties for the set of matched elements.
            // Example 1: Disable all checkboxes on the page.
            PerformJQueryTest(@"
                $(""input[type='checkbox']"").prop({
                  disabled: true
                });
            ");
        }

        [TestMethod]
        public void jQuery_holdReady_Example1()
        {
            // Test Block 29.
            // Entry jQuery.holdReady: Holds or releases the execution of jQuery's ready event.
            // Example 1: Delay the ready event until a custom plugin has loaded.
            PerformJQueryTest(@"
                $.holdReady(true);
                $.getScript(""myplugin.js"", function() {
                     $.holdReady(false);
                });
            ");
        }

        [TestMethod]
        public void jQuery_hasData_Example1()
        {
            // Test Block 30.
            // Entry jQuery.hasData: Determine whether an element has any jQuery data associated with it.
            // Example 1: Set data on an element and see the results of hasData.
            PerformJQueryTest(@"
                $(function(){
                  var $p = jQuery(""p""), p = $p[0];
                  $p.append(jQuery.hasData(p)+"" ""); /* false */
                  jQuery.data(p, ""testing"", 123);
                  $p.append(jQuery.hasData(p)+"" ""); /* true*/
                  jQuery.removeData(p, ""testing"");
                  $p.append(jQuery.hasData(p)+"" ""); /* false */
                });
            ");
        }

        [TestMethod]
        public void jquery_Example1()
        {
            // Test Block 31.
            // Entry jquery: A string containing the jQuery version number.
            // Example 1: Determine if an object is a jQuery object
            PerformJQueryTest(@"
                var a = { what: ""A regular JS object"" },
                    b = $('body');
                if ( a.jquery ) { // falsy, since it's undefined
                    alert(' a is a jQuery object! ');    
                }
                if ( b.jquery ) { // truthy, since it's a string
                    alert(' b is a jQuery object! ');
                }
            ", "jquery");
        }

        [TestMethod]
        public void jquery_Example2()
        {
            // Test Block 32.
            // Entry jquery: A string containing the jQuery version number.
            // Example 2: Get the current version of jQuery running on the page
            PerformJQueryTest(@"
                alert( 'You are running jQuery version: ' + $.fn.jquery );
            ");
        }

        [TestMethod]
        public void deferred_promise_Example1()
        {
            // Test Block 33.
            // Entry deferred.promise:  Return a Deferred's Promise object. 
            // Example 1: Create a Deferred and set two timer-based functions to either resolve or reject the Deferred after a random interval. Whichever one fires first "wins" and will call one of the callbacks. The second timeout has no effect since the Deferred is already complete (in a resolved or rejected state) from the first timeout action. Also set a timer-based progress notification function, and call a progress handler that adds "working..." to the document body.
            PerformJQueryTest(@"
                function asyncEvent(){
                    var dfd = new jQuery.Deferred();
                    // Resolve after a random interval
                    setTimeout(function(){
                        dfd.resolve(""hurray"");
                    }, Math.floor(400+Math.random()*2000));
                    // Reject after a random interval
                    setTimeout(function(){
                        dfd.reject(""sorry"");
                    }, Math.floor(400+Math.random()*2000));
                    // Show a ""working..."" message every half-second
                    setTimeout(function working(){
                        if ( dfd.state() === ""pending"" ) {
                            dfd.notify(""working... "");
                            setTimeout(working, 500);
                        }
                    }, 1);
                    // Return the Promise so caller can't change the Deferred
                    return dfd.promise();
                }
                // Attach a done, fail, and progress handler for the asyncEvent
                $.when( asyncEvent() ).then(
                    function(status){
                        alert( status+', things are going well' );
                    },
                    function(status){
                        alert( status+', you fail this time' );
                    },
                    function(status){
                        $(""body"").append(status);
                    }
                );
            ");
        }

        [TestMethod]
        public void deferred_promise_Example2()
        {
            // Test Block 34.
            // Entry deferred.promise:  Return a Deferred's Promise object. 
            // Example 2: Use the target argument to promote an existing object to a Promise:
            PerformJQueryTest(@"
                // Existing object
                var obj = {
                  hello: function( name ) {
                    alert( ""Hello "" + name );
                  }
                },
                // Create a Deferred
                defer = $.Deferred();
                // Set object as a promise
                defer.promise( obj );
                // Resolve the deferred
                defer.resolve( ""John"" );
                // Use the object as a Promise
                obj.done(function( name ) {
                  obj.hello( name ); // will alert ""Hello John""
                }).hello( ""Karl"" ); // will alert ""Hello Karl""
            ", "defer");
        }

        [TestMethod]
        public void jQuery_parseXML_Example1()
        {
            // Test Block 35.
            // Entry jQuery.parseXML: Parses a string into an XML document.
            // Example 1: Create a jQuery object using an XML string and obtain the value of the title node.
            PerformJQueryTest(@"
                var xml = ""<rss version='2.0'><channel><title>RSS Title</title></channel></rss>"",
                    xmlDoc = $.parseXML( xml ),
                    $xml = $( xmlDoc ),
                    $title = $xml.find( ""title"" );
                /* append ""RSS Title"" to #someElement */
                $( ""#someElement"" ).append( $title.text() );
                /* change the title to ""XML Title"" */
                $title.text( ""XML Title"" );
                /* append ""XML Title"" to #anotherElement */
                $( ""#anotherElement"" ).append( $title.text() );
            ");
        }

        [TestMethod]
        public void jQuery_when_Example1()
        {
            // Test Block 36.
            // Entry jQuery.when: Provides a way to execute callback functions based on one or more objects, usually Deferred objects that represent asynchronous events.
            // Example 1: Execute a function after two ajax requests are successful. (See the jQuery.ajax() documentation for a complete description of success and error cases for an ajax request).
            PerformJQueryTest(@"
                $.when($.ajax(""/page1.php""), $.ajax(""/page2.php"")).done(function(a1,  a2){
                    /* a1 and a2 are arguments resolved for the 
                        page1 and page2 ajax requests, respectively */
                   var jqXHR = a1[2]; /* arguments are [ ""success"", statusText, jqXHR ] */
                   if ( /Whip It/.test(jqXHR.responseText) ) {
                      alert(""First page has 'Whip It' somewhere."");
                   }
                });
            ", "responseText");
        }

        [TestMethod]
        public void jQuery_when_Example2()
        {
            // Test Block 37.
            // Entry jQuery.when: Provides a way to execute callback functions based on one or more objects, usually Deferred objects that represent asynchronous events.
            // Example 2: Execute the function myFunc when both ajax requests are successful, or myFailure if either one has an error.
            PerformJQueryTest(@"
                $.when($.ajax(""/page1.php""), $.ajax(""/page2.php""))
                  .then(myFunc, myFailure);
            ", "myFunc", "myFailure");
        }

        [TestMethod]
        public void deferred_fail_Example1()
        {
            // Test Block 38.
            // Entry deferred.fail:  Add handlers to be called when the Deferred object is rejected. 
            // Example 1: Since the jQuery.get method returns a jqXHR object, which is derived from a Deferred, you can attach a success and failure callback using the deferred.done() and deferred.fail() methods.
            PerformJQueryTest(@"
                $.get(""test.php"")
                  .done(function(){ alert(""$.get succeeded""); })
                  .fail(function(){ alert(""$.get failed!""); });
            ");
        }

        [TestMethod]
        public void deferred_done_Example1()
        {
            // Test Block 39.
            // Entry deferred.done:  Add handlers to be called when the Deferred object is resolved. 
            // Example 1: Since the jQuery.get method returns a jqXHR object, which is derived from a Deferred object, we can attach a success callback using the .done() method.
            PerformJQueryTest(@"
                $.get(""test.php"").done(function() { 
                  alert(""$.get succeeded""); 
                });
            ");
        }

        [TestMethod]
        public void deferred_done_Example2()
        {
            // Test Block 40.
            // Entry deferred.done:  Add handlers to be called when the Deferred object is resolved. 
            // Example 2: Resolve a Deferred object when the user clicks a button, triggering a number of callback functions:
            PerformJQueryTest(@"
                // 3 functions to call when the Deferred object is resolved
                function fn1() {
                  $(""p"").append("" 1 "");
                }
                function fn2() {
                  $(""p"").append("" 2 "");
                }
                function fn3(n) {
                  $(""p"").append(n + "" 3 "" + n);
                }
                // create a deferred object
                var dfd = $.Deferred();
                // add handlers to be called when dfd is resolved
                dfd
                // .done() can take any number of functions or arrays of functions
                .done( [fn1, fn2], fn3, [fn2, fn1] )
                // we can chain done methods, too
                .done(function(n) {
                  $(""p"").append(n + "" we're done."");
                });
                // resolve the Deferred object when the button is clicked
                $(""button"").bind(""click"", function() {
                  dfd.resolve(""and"");
                });
            ");
        }

        [TestMethod]
        public void deferred_then_Example1()
        {
            // Test Block 41.
            // Entry deferred.then:  Add handlers to be called when the Deferred object is resolved or rejected. 
            // Example 1: Since the jQuery.get method returns a jqXHR object, which is derived from a Deferred object, we can attach handlers using the .then method.
            PerformJQueryTest(@"
                $.get(""test.php"").then(
                    function(){ alert(""$.get succeeded""); },
                    function(){ alert(""$.get failed!""); }
                );
            ");
        }

        [TestMethod]
        public void jQuery_sub_Example1()
        {
            // Test Block 42.
            // Entry jQuery.sub: Creates a new copy of jQuery whose properties and methods can be modified without affecting the original jQuery object.
            // Example 1: Adding a method to a jQuery sub so that it isn't exposed externally:
            PerformJQueryTest(@"
                  (function(){
                    var sub$ = jQuery.sub();
                    sub$.fn.myCustomMethod = function(){
                      return 'just for me';
                    };
                    sub$(document).ready(function() {
                      sub$('body').myCustomMethod() // 'just for me'
                    });
                  })();
                  typeof jQuery('body').myCustomMethod // undefined
            ", "myCustomMethod", "myCustomMethod");
        }

        [TestMethod]
        public void jQuery_sub_Example2()
        {
            // Test Block 43.
            // Entry jQuery.sub: Creates a new copy of jQuery whose properties and methods can be modified without affecting the original jQuery object.
            // Example 2: Override some jQuery methods to provide new functionality.
            PerformJQueryTest(@"
                (function() {
                  var myjQuery = jQuery.sub();
                  myjQuery.fn.remove = function() {
                    // New functionality: Trigger a remove event
                    this.trigger(""remove"");
                    // Be sure to call the original jQuery remove method
                    return jQuery.fn.remove.apply( this, arguments );
                  };
                  myjQuery(function($) {
                    $("".menu"").click(function() {
                      $(this).find("".submenu"").remove();
                    });
                    // A new remove event is now triggered from this copy of jQuery
                    $(document).bind(""remove"", function(e) {
                      $(e.target).parent().hide();
                    });
                  });
                })();
                // Regular jQuery doesn't trigger a remove event when removing an element
                // This functionality is only contained within the modified 'myjQuery'.
            ");
        }

        [TestMethod]
        public void jQuery_sub_Example3()
        {
            // Test Block 44.
            // Entry jQuery.sub: Creates a new copy of jQuery whose properties and methods can be modified without affecting the original jQuery object.
            // Example 3: Create a plugin that returns plugin-specific methods.
            PerformJQueryTest(@"
                (function() {
                  // Create a new copy of jQuery using sub()
                  var plugin = jQuery.sub();
                  // Extend that copy with the new plugin methods
                  plugin.fn.extend({
                    open: function() {
                      return this.show();
                    },
                    close: function() {
                      return this.hide();
                    }
                  });
                  // Add our plugin to the original jQuery
                  jQuery.fn.myplugin = function() {
                    this.addClass(""plugin"");
                    // Make sure our plugin returns our special plugin version of jQuery
                    return plugin( this );
                  };
                })();
                $(document).ready(function() {
                  // Call the plugin, open method now exists
                  $('#main').myplugin().open();
                  // Note: Calling just $(""#main"").open() won't work as open doesn't exist!
                });
            ", "myplugin", "hide");
        }

        [TestMethod]
        public void fadeToggle_Example1()
        {
            // Test Block 45.
            // Entry fadeToggle: Display or hide the matched elements by animating their opacity.
            // Example 1: Fades first paragraph in or out, completing the animation within 600 milliseconds and using a linear easing. Fades last paragraph in or out for 200 milliseconds, inserting a "finished" message upon completion. 
            PerformJQueryTest(@"
                $(""button:first"").click(function() {
                  $(""p:first"").fadeToggle(""slow"", ""linear"");
                });
                $(""button:last"").click(function () {
                  $(""p:last"").fadeToggle(""fast"", function () {
                    $(""#log"").append(""<div>finished</div>"");
                  });
                });
            ");
        }

        [TestMethod]
        public void jQuery_type_Example1()
        {
            // Test Block 46.
            // Entry jQuery.type: Determine the internal JavaScript [[Class]] of an object.
            // Example 1: Find out if the parameter is a RegExp.
            PerformJQueryTest(@"
                $(""b"").append( """" + jQuery.type(/test/) );
            ");
        }

        [TestMethod]
        public void jQuery_isWindow_Example1()
        {
            // Test Block 47.
            // Entry jQuery.isWindow: Determine whether the argument is a window.
            // Example 1: Finds out if the parameter is a window.
            PerformJQueryTest(@"
                $(""b"").append( """" + $.isWindow(window) );
            ");
        }

        [TestMethod]
        public void toggle_Example1()
        {
            // Test Block 48.
            // Entry toggle: Bind two or more handlers to the matched elements, to be executed on alternate clicks.
            // Example 1: Click to toggle highlight on the list item.
            PerformJQueryTest(@"
                    $(""li"").toggle(
                      function () {
                        $(this).css({""list-style-type"":""disc"", ""color"":""blue""});
                      },
                      function () {
                        $(this).css({""list-style-type"":""disc"", ""color"":""red""});
                      },
                      function () {
                        $(this).css({""list-style-type"":"""", ""color"":""""});
                      }
                    );
            ");
        }

        [TestMethod]
        public void toggle_Example2()
        {
            // Test Block 49.
            // Entry toggle: Bind two or more handlers to the matched elements, to be executed on alternate clicks.
            // Example 2: To toggle a style on table cells:
            PerformJQueryTest(@"
                $(""td"").toggle(
                  function () {
                    $(this).addClass(""selected"");
                  },
                  function () {
                    $(this).removeClass(""selected"");
                  }
                );
            ");
        }

        [TestMethod]
        public void jQuery_fx_interval_Example1()
        {
            // Test Block 50.
            // Entry jQuery.fx.interval: The rate (in milliseconds) at which animations fire.
            // Example 1: Cause all animations to run with less frames.
            PerformJQueryTest(@"
                jQuery.fx.interval = 100;
                $(""input"").click(function(){
                  $(""div"").toggle( 3000 );
                });
            ");
        }

        [TestMethod]
        public void event_namespace_Example1()
        {
            // Test Block 51.
            // Entry event.namespace: The namespace specified when the event was triggered.
            // Example 1: Determine the event namespace used.
            PerformJQueryTest(@"
                $(""p"").bind(""test.something"", function(event) {
                  alert( event.namespace );
                });
                $(""button"").click(function(event) {
                  $(""p"").trigger(""test.something"");
                });  
            ", "namespace");
        }

        [TestMethod]
        public void undelegate_Example1()
        {
            // Test Block 52.
            // Entry undelegate: Remove a handler from the event for all elements which match the current selector, based upon a specific set of root elements.
            // Example 1: Can bind and unbind events to the colored button.
            PerformJQueryTest(@"
                function aClick() {
                  $(""div"").show().fadeOut(""slow"");
                }
                $(""#bind"").click(function () {
                  $(""body"").delegate(""#theone"", ""click"", aClick)
                    .find(""#theone"").text(""Can Click!"");
                });
                $(""#unbind"").click(function () {
                  $(""body"").undelegate(""#theone"", ""click"", aClick)
                    .find(""#theone"").text(""Does nothing..."");
                });
            ");
        }

        [TestMethod]
        public void undelegate_Example2()
        {
            // Test Block 53.
            // Entry undelegate: Remove a handler from the event for all elements which match the current selector, based upon a specific set of root elements.
            // Example 2: To unbind all delegated events from all paragraphs, write:
            PerformJQueryTest(@"
                $(""p"").undelegate()
            ");
        }

        [TestMethod]
        public void undelegate_Example3()
        {
            // Test Block 54.
            // Entry undelegate: Remove a handler from the event for all elements which match the current selector, based upon a specific set of root elements.
            // Example 3: To unbind all delegated click events from all paragraphs, write:
            PerformJQueryTest(@"
                $(""p"").undelegate( ""click"" )
            ");
        }

        [TestMethod]
        public void undelegate_Example4()
        {
            // Test Block 55.
            // Entry undelegate: Remove a handler from the event for all elements which match the current selector, based upon a specific set of root elements.
            // Example 4: To undelegate just one previously bound handler, pass the function in as the third argument:
            PerformJQueryTest(@"
                var foo = function () {
                  // code to handle some kind of event
                };
                // ... now foo will be called when paragraphs are clicked ...
                $(""body"").delegate(""p"", ""click"", foo);
                // ... foo will no longer be called.
                $(""body"").undelegate(""p"", ""click"", foo); 
            ");
        }

        [TestMethod]
        public void undelegate_Example5()
        {
            // Test Block 56.
            // Entry undelegate: Remove a handler from the event for all elements which match the current selector, based upon a specific set of root elements.
            // Example 5: To unbind all delegated events by their namespace:
            PerformJQueryTest(@"
                var foo = function () {
                  // code to handle some kind of event
                };
                // delegate events under the "".whatever"" namespace
                $(""form"").delegate("":button"", ""click.whatever"", foo);
                $(""form"").delegate(""input[type='text']"", ""keypress.whatever"", foo); 
                // unbind all events delegated under the "".whatever"" namespace
                $(""form"").undelegate("".whatever"");
            ");
        }

        [TestMethod]
        public void delegate_Example1()
        {
            // Test Block 57.
            // Entry delegate: Attach a handler to one or more events for all elements that match the selector, now or in the future, based on a specific set of root elements.
            // Example 1: Click a paragraph to add another. Note that .delegate() attaches a click event handler to all paragraphs - even new ones.
            PerformJQueryTest(@"
                    $(""body"").delegate(""p"", ""click"", function(){
                      $(this).after(""<p>Another paragraph!</p>"");
                    });
            ");
        }

        [TestMethod]
        public void delegate_Example2()
        {
            // Test Block 58.
            // Entry delegate: Attach a handler to one or more events for all elements that match the selector, now or in the future, based on a specific set of root elements.
            // Example 2: To display each paragraph's text in an alert box whenever it is clicked:
            PerformJQueryTest(@"
                $(""body"").delegate(""p"", ""click"", function(){
                  alert( $(this).text() );
                });
            ");
        }

        [TestMethod]
        public void delegate_Example3()
        {
            // Test Block 59.
            // Entry delegate: Attach a handler to one or more events for all elements that match the selector, now or in the future, based on a specific set of root elements.
            // Example 3: To cancel a default action and prevent it from bubbling up, return false:
            PerformJQueryTest(@"
                $(""body"").delegate(""a"", ""click"", function() { return false; })
            ");
        }

        [TestMethod]
        public void delegate_Example4()
        {
            // Test Block 60.
            // Entry delegate: Attach a handler to one or more events for all elements that match the selector, now or in the future, based on a specific set of root elements.
            // Example 4: To cancel only the default action by using the preventDefault method.
            PerformJQueryTest(@"
                $(""body"").delegate(""a"", ""click"", function(event){
                  event.preventDefault();
                });
            ", "preventDefault");
        }

        [TestMethod]
        public void delegate_Example5()
        {
            // Test Block 61.
            // Entry delegate: Attach a handler to one or more events for all elements that match the selector, now or in the future, based on a specific set of root elements.
            // Example 5: Can bind custom events too.
            PerformJQueryTest(@"
                    $(""body"").delegate(""p"", ""myCustomEvent"", function(e, myName, myValue){
                      $(this).text(""Hi there!"");
                      $(""span"").stop().css(""opacity"", 1)
                               .text(""myName = "" + myName)
                               .fadeIn(30).fadeOut(1000);
                    });
                    $(""button"").click(function () {
                      $(""p"").trigger(""myCustomEvent"");
                    });
            ");
        }

        [TestMethod]
        public void jQuery_error_Example1()
        {
            // Test Block 62.
            // Entry jQuery.error: Takes a string and throws an exception containing it.
            // Example 1: Override jQuery.error for display in Firebug.
            PerformJQueryTest(@"
                jQuery.error = console.error;
            ");
        }

        [TestMethod]
        public void jQuery_parseJSON_Example1()
        {
            // Test Block 63.
            // Entry jQuery.parseJSON: Takes a well-formed JSON string and returns the resulting JavaScript object.
            // Example 1: Parse a JSON string.
            PerformJQueryTest(@"
                var obj = jQuery.parseJSON('{""name"":""John""}');
                alert( obj.name === ""John"" );
            ");
        }

        [TestMethod]
        public void jQuery_proxy_Example1()
        {
            // Test Block 64.
            // Entry jQuery.proxy: Takes a function and returns a new one that will always have a particular context.
            // Example 1: Change the context of functions bound to a click handler using the "function, context" signature. Unbind the first handler after first click.
            PerformJQueryTest(@"
                var me = {
                  type: ""zombie"",
                  test: function(event) {
                    // Without proxy, `this` would refer to the event target
                    // use event.target to reference that element.
                    var element = event.target;
                    $(element).css(""background-color"", ""red"");
                    // With proxy, `this` refers to the me object encapsulating
                    // this function.
                    $(""#log"").append( ""Hello "" + this.type + ""<br>"" );
                    $(""#test"").unbind(""click"", this.test);
                  }
                };
                var you = {
                  type: ""person"",
                  test: function(event) {
                    $(""#log"").append( this.type + "" "" );
                  }
                };
                // execute you.test() in the context of the `you` object
                // no matter where it is called
                // i.e. the `this` keyword will refer to `you`
                var youClick = $.proxy( you.test, you );
                // attach click handlers to #test
                $(""#test"")
                  // this === ""zombie""; handler unbound after first click
                  .click( $.proxy( me.test, me ) )
                  // this === ""person""
                  .click( youClick )
                  // this === ""zombie""
                  .click( $.proxy( you.test, me ) )
                  // this === ""<button> element""
                  .click( you.test );
            ");
        }

        [TestMethod]
        public void jQuery_proxy_Example2()
        {
            // Test Block 65.
            // Entry jQuery.proxy: Takes a function and returns a new one that will always have a particular context.
            // Example 2: Enforce the context of the function using the "context, function name" signature. Unbind the handler after first click.
            PerformJQueryTest(@"
                  var obj = {
                    name: ""John"",
                    test: function() {
                      $(""#log"").append( this.name );
                      $(""#test"").unbind(""click"", obj.test);
                    }
                  };
                  $(""#test"").click( jQuery.proxy( obj, ""test"" ) );
            ");
        }

        [TestMethod]
        public void focusout_Example1()
        {
            // Test Block 66.
            // Entry focusout: Bind an event handler to the "focusout" JavaScript event.
            // Example 1: Watch for a loss of focus to occur inside paragraphs and note the difference between the focusout count and the blur count.
            PerformJQueryTest(@"
                var fo = 0, b = 0;
                $(""p"").focusout(function() {
                  fo++;
                  $(""#fo"")
                    .text(""focusout fired: "" + fo + ""x"");
                }).blur(function() {
                  b++;
                  $(""#b"")
                    .text(""blur fired: "" + b + ""x"");
                });
            ");
        }

        [TestMethod]
        public void focusin_Example1()
        {
            // Test Block 67.
            // Entry focusin: Bind an event handler to the "focusin" event.
            // Example 1: Watch for a focus to occur within the paragraphs on the page.
            PerformJQueryTest(@"
                    $(""p"").focusin(function() {
                         $(this).find(""span"").css('display','inline').fadeOut(1000);
                    });
            ");
        }

        [TestMethod]
        public void has_Example1()
        {
            // Test Block 68.
            // Entry has: Reduce the set of matched elements to those that have a descendant that matches the selector or DOM element.
            // Example 1: Check if an element is inside another.
            PerformJQueryTest(@"
                  $(""ul"").append(""<li>"" + ($(""ul"").has(""li"").length ? ""Yes"" : ""No"") + ""</li>"");
                  $(""ul"").has(""li"").addClass(""full"");
            ");
        }

        [TestMethod]
        public void jQuery_contains_Example1()
        {
            // Test Block 69.
            // Entry jQuery.contains: Check to see if a DOM element is within another DOM element.
            // Example 1: Check if an element is inside another. Text and comment nodes are not supported.
            PerformJQueryTest(@"
                jQuery.contains(document.documentElement, document.body); // true
                jQuery.contains(document.body, document.documentElement); // false
            ");
        }

        [TestMethod]
        public void delay_Example1()
        {
            // Test Block 70.
            // Entry delay: Set a timer to delay execution of subsequent items in the queue.
            // Example 1: Animate the hiding and showing of two divs, delaying the first before showing it.
            PerformJQueryTest(@"
                    $(""button"").click(function() {
                      $(""div.first"").slideUp(300).delay(800).fadeIn(400);
                      $(""div.second"").slideUp(300).fadeIn(400);
                    });
            ");
        }

        [TestMethod]
        public void parentsUntil_Example1()
        {
            // Test Block 71.
            // Entry parentsUntil: Get the ancestors of each element in the current set of matched elements, up to but not including the element matched by the selector, DOM node, or jQuery object.
            // Example 1: Find the ancestors of <li class="item-a"> up to <ul class="level-1"> and give them a red background color. Also, find ancestors of <li class="item-2"> that have a class of "yes" up to <ul class="level-1"> and give them a green border.
            PerformJQueryTest(@"
                $(""li.item-a"").parentsUntil("".level-1"")
                  .css(""background-color"", ""red"");
                $(""li.item-2"").parentsUntil( $(""ul.level-1""), "".yes"" )
                  .css(""border"", ""3px solid green"");
            ");
        }

        [TestMethod]
        public void prevUntil_Example1()
        {
            // Test Block 72.
            // Entry prevUntil: Get all preceding siblings of each element up to but not including the element matched by the selector, DOM node, or jQuery object.
            // Example 1: Find the siblings that precede <dt id="term-2"> up to the preceding <dt> and give them a red background color. Also, find previous <dd> siblings of <dt id="term-3"> up to <dt id="term-1"> and give them a green text color.
            PerformJQueryTest(@"
                $(""#term-2"").prevUntil(""dt"")
                  .css(""background-color"", ""red"");
                var term1 = document.getElementById('term-1');
                $(""#term-3"").prevUntil(term1, ""dd"")
                  .css(""color"", ""green"");
            ");
        }

        [TestMethod]
        public void nextUntil_Example1()
        {
            // Test Block 73.
            // Entry nextUntil: Get all following siblings of each element up to but not including the element matched by the selector, DOM node, or jQuery object passed.
            // Example 1: Find the siblings that follow <dt id="term-2"> up to the next <dt> and give them a red background color. Also, find <dd> siblings that follow <dt id="term-1"> up to <dt id="term-3"> and give them a green text color. 
            PerformJQueryTest(@"
                $(""#term-2"").nextUntil(""dt"")
                  .css(""background-color"", ""red"");
                var term3 = document.getElementById(""term-3"");
                $(""#term-1"").nextUntil(term3, ""dd"")
                  .css(""color"", ""green"");
            ");
        }

        [TestMethod]
        public void event_isImmediatePropagationStopped_Example1()
        {
            // Test Block 74.
            // Entry event.isImmediatePropagationStopped:   Returns whether event.stopImmediatePropagation() was ever called on this event object. 
            // Example 1: Checks whether event.stopImmediatePropagation() was called.
            PerformJQueryTest(@"
                function immediatePropStopped(e) {
                  var msg = """";
                  if ( e.isImmediatePropagationStopped() ) {
                    msg =  ""called""
                  } else {
                    msg = ""not called"";
                  }
                  $(""#stop-log"").append( ""<div>"" + msg + ""</div>"" );
                }
                $(""button"").click(function(event) {
                  immediatePropStopped(event);
                  event.stopImmediatePropagation();
                  immediatePropStopped(event);
                });  
            ");
        }

        [TestMethod]
        public void event_stopImmediatePropagation_Example1()
        {
            // Test Block 75.
            // Entry event.stopImmediatePropagation:  Keeps the rest of the handlers from being executed and prevents the event from bubbling up the DOM tree.    
            // Example 1: Prevents other event handlers from being called.
            PerformJQueryTest(@"
                $(""p"").click(function(event){
                  event.stopImmediatePropagation();
                });
                $(""p"").click(function(event){
                  // This function won't be executed
                  $(this).css(""background-color"", ""#f00"");
                });  
                $(""div"").click(function(event) {
                  // This function will be executed
                    $(this).css(""background-color"", ""#f00"");
                });
            ");
        }

        [TestMethod]
        public void event_isPropagationStopped_Example1()
        {
            // Test Block 76.
            // Entry event.isPropagationStopped:   Returns whether event.stopPropagation() was ever called on this event object. 
            // Example 1: Checks whether event.stopPropagation() was called
            PerformJQueryTest(@"
                function propStopped(e) {
                  var msg = """";
                  if ( e.isPropagationStopped() ) {
                    msg =  ""called""
                  } else {
                    msg = ""not called"";
                  }
                  $(""#stop-log"").append( ""<div>"" + msg + ""</div>"" );
                }
                $(""button"").click(function(event) {
                  propStopped(event);
                  event.stopPropagation();
                  propStopped(event);
                });  
            ");
        }

        [TestMethod]
        public void event_stopPropagation_Example1()
        {
            // Test Block 77.
            // Entry event.stopPropagation: Prevents the event from bubbling up the DOM tree, preventing any parent handlers from being notified of the event.   
            // Example 1: Kill the bubbling on the click event.
            PerformJQueryTest(@"
                $(""p"").click(function(event){
                  event.stopPropagation();
                  // do something
                });  
            ");
        }

        [TestMethod]
        public void event_isDefaultPrevented_Example1()
        {
            // Test Block 78.
            // Entry event.isDefaultPrevented: Returns whether event.preventDefault() was ever called on this event object. 
            // Example 1: Checks whether event.preventDefault() was called.
            PerformJQueryTest(@"
                $(""a"").click(function(event){
                  alert( event.isDefaultPrevented() ); // false
                  event.preventDefault();
                  alert( event.isDefaultPrevented() ); // true
                });  
            ");
        }

        [TestMethod]
        public void event_preventDefault_Example1()
        {
            // Test Block 79.
            // Entry event.preventDefault:  If this method is called, the default action of the event will not be triggered. 
            // Example 1: Cancel the default action (navigation) of the click.
            PerformJQueryTest(@"
                $(""a"").click(function(event) {
                  event.preventDefault();
                  $('<div/>')
                    .append('default ' + event.type + ' prevented')
                    .appendTo('#log');
                });
            ");
        }

        [TestMethod]
        public void event_timeStamp_Example1()
        {
            // Test Block 80.
            // Entry event.timeStamp: The difference in milliseconds between the time the browser created the event and January 1, 1970.
            // Example 1: Display the time since the click handler last executed.
            PerformJQueryTest(@"
                var last, diff;
                $('div').click(function(event) {
                  if ( last ) {
                    diff = event.timeStamp - last
                    $('div').append('time since last event: ' + diff + '<br/>');
                  } else {
                    $('div').append('Click again.<br/>');
                  }
                  last = event.timeStamp;
                });  
            ");
        }

        [TestMethod]
        public void event_result_Example1()
        {
            // Test Block 81.
            // Entry event.result:  The last value returned by an event handler that was triggered by this event, unless the value was undefined.  
            // Example 1: Display previous handler's return value
            PerformJQueryTest(@"
                $(""button"").click(function(event) {
                  return ""hey"";
                });
                $(""button"").click(function(event) {
                  $(""p"").html( event.result );
                });  
            ", "result");
        }

        [TestMethod]
        public void event_which_Example1()
        {
            // Test Block 82.
            // Entry event.which:  For key or button events, this attribute indicates the specific button or key that was pressed.  
            // Example 1: Log what key was depressed.
            PerformJQueryTest(@"
                $('#whichkey').bind('keydown',function(e){ 
                  $('#log').html(e.type + ': ' +  e.which );
                });  
            ");
        }

        [TestMethod]
        public void event_pageY_Example1()
        {
            // Test Block 83.
            // Entry event.pageY: The mouse position relative to the top edge of the document. 
            // Example 1: Show the mouse position relative to the left and top edges of the document (within this iframe).
            PerformJQueryTest(@"
                $(document).bind('mousemove',function(e){ 
                            $(""#log"").text(""e.pageX: "" + e.pageX + "", e.pageY: "" + e.pageY); 
                }); 
            ");
        }

        [TestMethod]
        public void event_pageX_Example1()
        {
            // Test Block 84.
            // Entry event.pageX: The mouse position relative to the left edge of the document. 
            // Example 1: Show the mouse position relative to the left and top edges of the document (within the iframe).
            PerformJQueryTest(@"
                $(document).bind('mousemove',function(e){ 
                            $(""#log"").text(""e.pageX: "" + e.pageX + "", e.pageY: "" + e.pageY); 
                }); 
            ");
        }

        [TestMethod]
        public void event_currentTarget_Example1()
        {
            // Test Block 85.
            // Entry event.currentTarget:  The current DOM element within the event bubbling phase.  
            // Example 1: Alert that currentTarget matches the `this` keyword.
            PerformJQueryTest(@"
                $(""p"").click(function(event) {
                  alert( event.currentTarget === this ); // true
                });  
            ");
        }

        [TestMethod]
        public void event_relatedTarget_Example1()
        {
            // Test Block 86.
            // Entry event.relatedTarget:   The other DOM element involved in the event, if any. 
            // Example 1: On mouseout of anchors, alert the element type being entered.
            PerformJQueryTest(@"
                $(""a"").mouseout(function(event) {
                  alert(event.relatedTarget.nodeName); // ""DIV""
                });  
            ");
        }

        [TestMethod]
        public void event_data_Example1()
        {
            // Test Block 87.
            // Entry event.data:  The optional data passed to jQuery.fn.bind when the current executing handler was bound.  
            // Example 1: The description of the example.
            PerformJQueryTest(@"
                $(""a"").each(function(i) {
                  $(this).bind('click', {index:i}, function(e){
                     alert('my index is ' + e.data.index);
                  });
                });   
            ");
        }

        [TestMethod]
        public void event_target_Example1()
        {
            // Test Block 88.
            // Entry event.target:  The DOM element that initiated the event.  
            // Example 1: Display the tag's name on click
            PerformJQueryTest(@"
                $(""body"").click(function(event) {
                  $(""#log"").html(""clicked: "" + event.target.nodeName);
                });  
            ");
        }

        [TestMethod]
        public void event_target_Example2()
        {
            // Test Block 89.
            // Entry event.target:  The DOM element that initiated the event.  
            // Example 2: Implements a simple event delegation: The click handler is added to an unordered list, and the children of its li children are hidden. Clicking one of the li children toggles (see toggle()) their children.
            PerformJQueryTest(@"
                function handler(event) {
                  var $target = $(event.target);
                  if( $target.is(""li"") ) {
                    $target.children().toggle();
                  }
                }
                $(""ul"").click(handler).find(""ul"").hide();
            ");
        }

        [TestMethod]
        public void event_type_Example1()
        {
            // Test Block 90.
            // Entry event.type:  Describes the nature of the event.  
            // Example 1: On all anchor clicks, alert the event type.
            PerformJQueryTest(@"
                $(""a"").click(function(event) {
                  alert(event.type); // ""click""
                }); 
            ");
        }

        [TestMethod]
        public void jQuery_fx_off_Example1()
        {
            // Test Block 91.
            // Entry jQuery.fx.off: Globally disable all animations.
            // Example 1: Toggle animation on and off
            PerformJQueryTest(@"
                var toggleFx = function() {
                  $.fx.off = !$.fx.off;
                };
                toggleFx();
                $(""button"").click(toggleFx)
                $(""input"").click(function(){
                  $(""div"").toggle(""slow"");
                });
            ", "off", "off");
        }

        [TestMethod]
        public void each_Example1()
        {
            // Test Block 92.
            // Entry each: Iterate over a jQuery object, executing a function for each matched element. 
            // Example 1: Iterates over three divs and sets their color property.
            PerformJQueryTest(@"
                    $(document.body).click(function () {
                      $(""div"").each(function (i) {
                        if (this.style.color != ""blue"") {
                          this.style.color = ""blue"";
                        } else {
                          this.style.color = """";
                        }
                      });
                    });
            ");
        }

        [TestMethod]
        public void each_Example2()
        {
            // Test Block 93.
            // Entry each: Iterate over a jQuery object, executing a function for each matched element. 
            // Example 2: If you want to have the jQuery object instead of the regular DOM element, use the $(this) function, for example:
            PerformJQueryTest(@"
                    $(""span"").click(function () {
                      $(""li"").each(function(){
                        $(this).toggleClass(""example"");
                      });
                    });
            ");
        }

        [TestMethod]
        public void each_Example3()
        {
            // Test Block 94.
            // Entry each: Iterate over a jQuery object, executing a function for each matched element. 
            // Example 3: You can use 'return' to break out of each() loops early.
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      $(""div"").each(function (index, domEle) {
                        // domEle == this
                        $(domEle).css(""backgroundColor"", ""yellow""); 
                        if ($(this).is(""#stop"")) {
                          $(""span"").text(""Stopped at div index #"" + index);
                          return false;
                        }
                      });
                    });
            ");
        }

        [TestMethod]
        public void pushStack_Example1()
        {
            // Test Block 95.
            // Entry pushStack: Add a collection of DOM elements onto the jQuery stack.
            // Example 1: Add some elements onto the jQuery stack, then pop back off again.
            PerformJQueryTest(@"
                jQuery([])
                    .pushStack( document.getElementsByTagName(""div"") )
                        .remove()
                    .end();
            ");
        }

        [TestMethod]
        public void jQuery_globalEval_Example1()
        {
            // Test Block 96.
            // Entry jQuery.globalEval: Execute some JavaScript code globally.
            // Example 1: Execute a script in the global context.
            PerformJQueryTest(@"
                function test(){
                    jQuery.globalEval(""var newVar = true;"")
                }
                test();
                // newVar === true
            ");
        }

        [TestMethod]
        public void jQuery_isXMLDoc_Example1()
        {
            // Test Block 97.
            // Entry jQuery.isXMLDoc: Check to see if a DOM node is within an XML document (or is an XML document).
            // Example 1: Check an object to see if it's in an XML document.
            PerformJQueryTest(@"
                jQuery.isXMLDoc(document) // false
                jQuery.isXMLDoc(document.body) // false
            ");
        }

        [TestMethod]
        public void jQuery_removeData_Example1()
        {
            // Test Block 98.
            // Entry jQuery.removeData: Remove a previously-stored piece of data.
            // Example 1: Set a data store for 2 names then remove one of them.
            PerformJQueryTest(@"
                var div = $(""div"")[0];
                $(""span:eq(0)"").text("""" + $(""div"").data(""test1""));
                jQuery.data(div, ""test1"", ""VALUE-1"");
                jQuery.data(div, ""test2"", ""VALUE-2"");
                $(""span:eq(1)"").text("""" + jQuery.data(div, ""test1""));
                jQuery.removeData(div, ""test1"");
                $(""span:eq(2)"").text("""" + jQuery.data(div, ""test1""));
                $(""span:eq(3)"").text("""" + jQuery.data(div, ""test2""));
            ");
        }

        [TestMethod]
        public void jQuery_data_Example1()
        {
            // Test Block 99.
            // Entry jQuery.data: Store arbitrary data associated with the specified element. Returns the value that was set.
            // Example 1: Store then retrieve a value from the div element.
            PerformJQueryTest(@"
                var div = $(""div"")[0];
                    jQuery.data(div, ""test"", { first: 16, last: ""pizza!"" });
                    $(""span:first"").text(jQuery.data(div, ""test"").first);
                    $(""span:last"").text(jQuery.data(div, ""test"").last);
            ");
        }

        [TestMethod]
        public void jQuery_data_1_Example1()
        {
            // Test Block 100.
            // Entry jQuery.data_1: Returns value at named data store for the element, as set by jQuery.data(element, name, value), or the full data store for the element.
            // Example 1: Get the data named "blah" stored at for an element.
            PerformJQueryTest(@"
                $(""button"").click(function(e) {
                  var value, div = $(""div"")[0];
                  switch ($(""button"").index(this)) {
                    case 0 :
                      value = jQuery.data(div, ""blah"");
                      break;
                    case 1 :
                      jQuery.data(div, ""blah"", ""hello"");
                      value = ""Stored!"";
                      break;
                    case 2 :
                      jQuery.data(div, ""blah"", 86);
                      value = ""Stored!"";
                      break;
                    case 3 :
                      jQuery.removeData(div, ""blah"");
                      value = ""Removed!"";
                      break;
                  }
                  $(""span"").text("""" + value);
                });
            ");
        }

        [TestMethod]
        public void jQuery_dequeue_Example1()
        {
            // Test Block 101.
            // Entry jQuery.dequeue: Execute the next function on the queue for the matched element.
            // Example 1: Use dequeue to end a custom queue function which allows the queue to keep going.
            PerformJQueryTest(@"
                $(""button"").click(function () {
                      $(""div"").animate({left:'+=200px'}, 2000);
                      $(""div"").animate({top:'0px'}, 600);
                      $(""div"").queue(function () {
                        $(this).toggleClass(""red"");
                         $.dequeue( this );
                              });
                      $(""div"").animate({left:'10px', top:'30px'}, 700);
                    });
            ");
        }

        [TestMethod]
        public void jQuery_queue_Example1()
        {
            // Test Block 102.
            // Entry jQuery.queue: Show the queue of functions to be executed on the matched element.
            // Example 1: Show the length of the queue.
            PerformJQueryTest(@"
                $(""#show"").click(function () {
                      var n = jQuery.queue( $(""div"")[0], ""fx"" );
                      $(""span"").text(""Queue length is: "" + n.length);
                    });
                    function runIt() {
                      $(""div"").show(""slow"");
                      $(""div"").animate({left:'+=200'},2000);
                      $(""div"").slideToggle(1000);
                      $(""div"").slideToggle(""fast"");
                      $(""div"").animate({left:'-=200'},1500);
                      $(""div"").hide(""slow"");
                      $(""div"").show(1200);
                      $(""div"").slideUp(""normal"", runIt);
                    }
                    runIt();
            ", "length");
        }

        [TestMethod]
        public void jQuery_queue_1_Example1()
        {
            // Test Block 103.
            // Entry jQuery.queue_1: Manipulate the queue of functions to be executed on the matched element.
            // Example 1: Queue a custom function.
            PerformJQueryTest(@"
                   $(document.body).click(function () {
                      $(""div"").show(""slow"");
                      $(""div"").animate({left:'+=200'},2000);
                      jQuery.queue( $(""div"")[0], ""fx"", function () {
                        $(this).addClass(""newcolor"");
                        jQuery.dequeue( this );
                      });
                      $(""div"").animate({left:'-=200'},500);
                      jQuery.queue( $(""div"")[0], ""fx"", function () {
                        $(this).removeClass(""newcolor"");
                        jQuery.dequeue( this );
                      });
                      $(""div"").slideUp();
                    });
            ");
        }

        [TestMethod]
        public void jQuery_queue_1_Example2()
        {
            // Test Block 104.
            // Entry jQuery.queue_1: Manipulate the queue of functions to be executed on the matched element.
            // Example 2: Set a queue array to delete the queue.
            PerformJQueryTest(@"
                   $(""#start"").click(function () {
                      $(""div"").show(""slow"");
                      $(""div"").animate({left:'+=200'},5000);
                      jQuery.queue( $(""div"")[0], ""fx"", function () {
                        $(this).addClass(""newcolor"");
                        jQuery.dequeue( this );
                      });
                      $(""div"").animate({left:'-=200'},1500);
                      jQuery.queue( $(""div"")[0], ""fx"", function () {
                        $(this).removeClass(""newcolor"");
                        jQuery.dequeue( this );
                      });
                      $(""div"").slideUp();
                    });
                    $(""#stop"").click(function () {
                      jQuery.queue( $(""div"")[0], ""fx"", [] );
                      $(""div"").stop();
                    });
            ");
        }

        [TestMethod]
        public void clearQueue_Example1()
        {
            // Test Block 105.
            // Entry clearQueue: Remove from the queue all items that have not yet been run.
            // Example 1: Empty the queue.
            PerformJQueryTest(@"
                $(""#start"").click(function () {
                  var myDiv = $(""div"");
                  myDiv.show(""slow"");
                  myDiv.animate({left:'+=200'},5000);
                  myDiv.queue(function () {
                    var _this = $(this);
                    _this.addClass(""newcolor"");
                    _this.dequeue();
                  });
                  myDiv.animate({left:'-=200'},1500);
                  myDiv.queue(function () {
                    var _this = $(this);
                    _this.removeClass(""newcolor"");
                    _this.dequeue();
                  });
                  myDiv.slideUp();
                });
                $(""#stop"").click(function () {
                  var myDiv = $(""div"");
                  myDiv.clearQueue();
                  myDiv.stop();
                });
            ");
        }

        [TestMethod]
        public void toArray_Example1()
        {
            // Test Block 106.
            // Entry toArray: Retrieve all the DOM elements contained in the jQuery set, as an array.
            // Example 1: Selects all divs in the document and returns the DOM Elements as an Array, then uses the built-in reverse-method to reverse that array.
            PerformJQueryTest(@"
                    function disp(divs) {
                      var a = [];
                      for (var i = 0; i < divs.length; i++) {
                        a.push(divs[i].innerHTML);
                      }
                      $(""span"").text(a.join("" ""));
                    }
                    disp( $(""div"").toArray().reverse() );
            ");
        }

        [TestMethod]
        public void jQuery_isEmptyObject_Example1()
        {
            // Test Block 107.
            // Entry jQuery.isEmptyObject: Check to see if an object is empty (contains no properties).
            // Example 1: Check an object to see if it's empty.
            PerformJQueryTest(@"
                jQuery.isEmptyObject({}) // true
                jQuery.isEmptyObject({ foo: ""bar"" }) // false
            ");
        }

        [TestMethod]
        public void jQuery_isPlainObject_Example1()
        {
            // Test Block 108.
            // Entry jQuery.isPlainObject: Check to see if an object is a plain object (created using "{}" or "new Object").
            // Example 1: Check an object to see if it's a plain object.
            PerformJQueryTest(@"
                jQuery.isPlainObject({}) // true
                jQuery.isPlainObject(""test"") // false
            ");
        }

        [TestMethod]
        public void keydown_Example1()
        {
            // Test Block 109.
            // Entry keydown: Bind an event handler to the "keydown" JavaScript event, or trigger that event on an element.
            // Example 1: Show the event object for the keydown handler when a key is pressed in the input.
            PerformJQueryTest(@"
                var xTriggered = 0;
                $('#target').keydown(function(event) {
                  if (event.keyCode == '13') {
                     event.preventDefault();
                   }
                   xTriggered++;
                   var msg = 'Handler for .keydown() called ' + xTriggered + ' time(s).';
                  $.print(msg, 'html');
                  $.print(event);
                });
                $('#other').click(function() {
                  $('#target').keydown();
                });
            ", "print", "print");
        }

        [TestMethod]
        public void index_Example1()
        {
            // Test Block 110.
            // Entry index: Search for a given element from among the matched elements.
            // Example 1: On click, returns the index (based zero) of that div in the page.
            PerformJQueryTest(@"
                $(""div"").click(function () {
                  // this is the dom element clicked
                  var index = $(""div"").index(this);
                  $(""span"").text(""That was div index #"" + index);
                });
            ");
        }

        [TestMethod]
        public void index_Example2()
        {
            // Test Block 111.
            // Entry index: Search for a given element from among the matched elements.
            // Example 2: Returns the index for the element with ID bar.
            PerformJQueryTest(@"
                var listItem = $('#bar');
                    $('div').html( 'Index: ' + $('li').index(listItem) );
            ");
        }

        [TestMethod]
        public void index_Example3()
        {
            // Test Block 112.
            // Entry index: Search for a given element from among the matched elements.
            // Example 3: Returns the index for the first item in the jQuery collection.
            PerformJQueryTest(@"
                var listItems = $('li:gt(0)');
                $('div').html( 'Index: ' + $('li').index(listItems) );
            ");
        }

        [TestMethod]
        public void index_Example4()
        {
            // Test Block 113.
            // Entry index: Search for a given element from among the matched elements.
            // Example 4: Returns the index for the element with ID bar in relation to all <li> elements.
            PerformJQueryTest(@"
                $('div').html('Index: ' +  $('#bar').index('li') );
            ");
        }

        [TestMethod]
        public void index_Example5()
        {
            // Test Block 114.
            // Entry index: Search for a given element from among the matched elements.
            // Example 5: Returns the index for the element with ID bar in relation to its siblings.
            PerformJQueryTest(@"
                var barIndex = $('#bar').index();
                $('div').html( 'Index: ' +  barIndex );
            ");
        }

        [TestMethod]
        public void index_Example6()
        {
            // Test Block 115.
            // Entry index: Search for a given element from among the matched elements.
            // Example 6: Returns -1, as there is no element with ID foobaz.
            PerformJQueryTest(@"
                var foobaz = $(""li"").index( $('#foobaz') );
                $('div').html('Index: ' + foobaz);
            ");
        }

        [TestMethod]
        public void removeData_Example1()
        {
            // Test Block 116.
            // Entry removeData: Remove a previously-stored piece of data.
            // Example 1: Set a data store for 2 names then remove one of them.
            PerformJQueryTest(@"
                    $(""span:eq(0)"").text("""" + $(""div"").data(""test1""));
                    $(""div"").data(""test1"", ""VALUE-1"");
                    $(""div"").data(""test2"", ""VALUE-2"");
                    $(""span:eq(1)"").text("""" + $(""div"").data(""test1""));
                    $(""div"").removeData(""test1"");
                    $(""span:eq(2)"").text("""" + $(""div"").data(""test1""));
                    $(""span:eq(3)"").text("""" + $(""div"").data(""test2""));
            ");
        }

        [TestMethod]
        public void data_Example1()
        {
            // Test Block 117.
            // Entry data: Store arbitrary data associated with the matched elements.
            // Example 1: Store then retrieve a value from the div element.
            PerformJQueryTest(@"
                $(""div"").data(""test"", { first: 16, last: ""pizza!"" });
                $(""span:first"").text($(""div"").data(""test"").first);
                $(""span:last"").text($(""div"").data(""test"").last);
            ", "first", "last");
        }

        [TestMethod]
        public void data_1_Example1()
        {
            // Test Block 118.
            // Entry data_1: Returns value at named data store for the first element in the jQuery collection, as set by data(name, value).
            // Example 1: Get the data named "blah" stored at for an element.
            PerformJQueryTest(@"
                $(""button"").click(function(e) {
                  var value;
                  switch ($(""button"").index(this)) {
                    case 0 :
                      value = $(""div"").data(""blah"");
                      break;
                    case 1 :
                      $(""div"").data(""blah"", ""hello"");
                      value = ""Stored!"";
                      break;
                    case 2 :
                      $(""div"").data(""blah"", 86);
                      value = ""Stored!"";
                      break;
                    case 3 :
                      $(""div"").removeData(""blah"");
                      value = ""Removed!"";
                      break;
                  }
                  $(""span"").text("""" + value);
                });
            ");
        }

        [TestMethod]
        public void get_Example1()
        {
            // Test Block 119.
            // Entry get: Retrieve the DOM elements matched by the jQuery object.
            // Example 1: Selects all divs in the document and returns the DOM Elements as an Array, then uses the built-in reverse-method to reverse that array.
            PerformJQueryTest(@"
                    function disp(divs) {
                      var a = [];
                      for (var i = 0; i < divs.length; i++) {
                        a.push(divs[i].innerHTML);
                      }
                      $(""span"").text(a.join("" ""));
                    }
                    disp( $(""div"").get().reverse() );
            ");
        }

        [TestMethod]
        public void get_Example2()
        {
            // Test Block 120.
            // Entry get: Retrieve the DOM elements matched by the jQuery object.
            // Example 2: Gives the tag name of the element clicked on.
            PerformJQueryTest(@"
                    $(""*"", document.body).click(function (e) {
                      e.stopPropagation();
                      var domEl = $(this).get(0);
                      $(""span:first"").text(""Clicked on - "" + domEl.tagName);
                    });
            ");
        }

        [TestMethod]
        public void size_Example1()
        {
            // Test Block 121.
            // Entry size: Return the number of elements in the jQuery object.
            // Example 1: Count the divs. Click to add more.
            PerformJQueryTest(@"
                $(document.body)
                .click(function() { 
                  $(this).append( $(""<div>"") );
                  var n = $(""div"").size();
                  $(""span"").text(""There are "" + n + "" divs. Click to add more."");
                })
                // trigger the click to start
                .click(); 
            ");
        }

        [TestMethod]
        public void jQuery_noConflict_Example1()
        {
            // Test Block 122.
            // Entry jQuery.noConflict: Relinquish jQuery's control of the $ variable.
            // Example 1: Maps the original object that was referenced by $ back to $.
            PerformJQueryTest(@"
                jQuery.noConflict();
                // Do something with jQuery
                jQuery(""div p"").hide();
                // Do something with another library's $()
                $(""content"").style.display = 'none';
            ", "style", "display");
        }

        [TestMethod]
        public void jQuery_noConflict_Example2()
        {
            // Test Block 123.
            // Entry jQuery.noConflict: Relinquish jQuery's control of the $ variable.
            // Example 2: Reverts the $ alias and then creates and executes a function to provide the $ as a jQuery alias inside the functions scope. Inside the function the original $ object is not available. This works well for most plugins that don't rely on any other library.      
            PerformJQueryTest(@"
                jQuery.noConflict();
                (function($) { 
                  $(function() {
                    // more code using $ as alias to jQuery
                  });
                })(jQuery);
                // other code using $ as an alias to the other library
            ");
        }

        [TestMethod]
        public void jQuery_noConflict_Example3()
        {
            // Test Block 124.
            // Entry jQuery.noConflict: Relinquish jQuery's control of the $ variable.
            // Example 3: You can chain the jQuery.noConflict() with the shorthand ready for a compact code.  
            PerformJQueryTest(@"
                jQuery.noConflict()(function(){
                    // code using jQuery
                }); 
                // other code using $ as an alias to the other library
            ");
        }

        [TestMethod]
        public void jQuery_noConflict_Example4()
        {
            // Test Block 125.
            // Entry jQuery.noConflict: Relinquish jQuery's control of the $ variable.
            // Example 4: Creates a different alias instead of jQuery to use in the rest of the script.
            PerformJQueryTest(@"
                var j = jQuery.noConflict();
                // Do something with jQuery
                j(""div p"").hide();
                // Do something with another library's $()
                $(""content"").style.display = 'none';
            ", "style", "display");
        }

        [TestMethod]
        public void jQuery_noConflict_Example5()
        {
            // Test Block 126.
            // Entry jQuery.noConflict: Relinquish jQuery's control of the $ variable.
            // Example 5: Completely move jQuery to a new namespace in another object.
            PerformJQueryTest(@"
                var dom = {};
                dom.query = jQuery.noConflict(true);
            ", "query");
        }

        [TestMethod]
        public void selected_Example1()
        {
            // Test Block 127.
            // Entry selected: Selects all elements that are selected.
            // Example 1: Attaches a change event to the select that gets the text for each selected option and writes them in the div.  It then triggers the event for the initial text draw.
            PerformJQueryTest(@"
                    $(""select"").change(function () {
                          var str = """";
                          $(""select option:selected"").each(function () {
                                str += $(this).text() + "" "";
                              });
                          $(""div"").text(str);
                        })
                        .trigger('change');
            ");
        }

        [TestMethod]
        public void checked_Example1()
        {
            // Test Block 128.
            // Entry checked: Matches all elements that are checked.
            // Example 1: Finds all input elements that are checked.
            PerformJQueryTest(@"
                function countChecked() {
                  var n = $(""input:checked"").length;
                  $(""div"").text(n + (n <= 1 ? "" is"" : "" are"") + "" checked!"");
                }
                countChecked();
                $("":checkbox"").click(countChecked);
            ");
        }

        [TestMethod]
        public void checked_Example2()
        {
            // Test Block 129.
            // Entry checked: Matches all elements that are checked.
            PerformJQueryTest(@"
                $(""input"").click(function() {
                  $(""#log"").html( $("":checked"").val() + "" is checked!"" );
                });
            ");
        }

        [TestMethod]
        public void disabled_Example1()
        {
            // Test Block 130.
            // Entry disabled: Selects all elements that are disabled.
            // Example 1: Finds all input elements that are disabled.
            PerformJQueryTest(@"
                $(""input:disabled"").val(""this is it"");
            ");
        }

        [TestMethod]
        public void enabled_Example1()
        {
            // Test Block 131.
            // Entry enabled: Selects all elements that are enabled.
            // Example 1: Finds all input elements that are enabled.
            PerformJQueryTest(@"
                $(""input:enabled"").val(""this is it"");
            ");
        }

        [TestMethod]
        public void file_Example1()
        {
            // Test Block 132.
            // Entry file: Selects all elements of type file.
            // Example 1: Finds all file inputs.
            PerformJQueryTest(@"
                    var input = $(""input:file"").css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void button_Example1()
        {
            // Test Block 133.
            // Entry button: Selects all button elements and elements of type button.
            // Example 1: Finds all button inputs.
            PerformJQueryTest(@"
                    var input = $("":button"").css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void reset_Example1()
        {
            // Test Block 134.
            // Entry reset: Selects all elements of type reset.
            // Example 1: Finds all reset inputs.
            PerformJQueryTest(@"
                    var input = $(""input:reset"").css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void image_Example1()
        {
            // Test Block 135.
            // Entry image: Selects all elements of type image.
            // Example 1: Finds all image inputs.
            PerformJQueryTest(@"
                    var input = $(""input:image"").css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void submit_Example1()
        {
            // Test Block 136.
            // Entry submit: Selects all elements of type submit.
            // Example 1: Finds all submit elements that are descendants of a td element.
            PerformJQueryTest(@"
                    var submitEl = $(""td :submit"")
                      .parent('td')
                      .css({background:""yellow"", border:""3px red solid""})
                    .end();
                    $('#result').text('jQuery matched ' + submitEl.length + ' elements.');
                    // so it won't submit
                    $(""form"").submit(function () { return false; });
                    // Extra JS to make the HTML easier to edit (None of this is relevant to the ':submit' selector
                    $('#exampleTable').find('td').each(function(i, el) {
                        var inputEl = $(el).children(),
                            inputType = inputEl.attr('type') ? ' type=""' + inputEl.attr('type') + '""' : '';
                        $(el).before('<td>' + inputEl[0].nodeName + inputType + '</td>');
                    })
            ", "nodeName");
        }

        [TestMethod]
        public void checkbox_Example1()
        {
            // Test Block 137.
            // Entry checkbox: Selects all elements of type checkbox.
            // Example 1: Finds all checkbox inputs.
            PerformJQueryTest(@"
                    var input = $(""form input:checkbox"").wrap('<span></span>').parent().css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void radio_Example1()
        {
            // Test Block 138.
            // Entry radio: Selects all  elements of type radio.
            // Example 1: Finds all radio inputs.
            PerformJQueryTest(@"
                    var input = $(""form input:radio"").wrap('<span></span>').parent().css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void password_Example1()
        {
            // Test Block 139.
            // Entry password: Selects all elements of type password.
            // Example 1: Finds all password inputs.
            PerformJQueryTest(@"
                    var input = $(""input:password"").css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void text_Example1()
        {
            // Test Block 140.
            // Entry text: Selects all elements of type text.
            // Example 1: Finds all text inputs.
            PerformJQueryTest(@"
                    var input = $(""form input:text"").css({background:""yellow"", border:""3px red solid""});
                    $(""div"").text(""For this type jQuery found "" + input.length + ""."")
                            .css(""color"", ""red"");
                    $(""form"").submit(function () { return false; }); // so it won't submit
            ");
        }

        [TestMethod]
        public void input_Example1()
        {
            // Test Block 141.
            // Entry input: Selects all input, textarea, select and button elements.
            // Example 1: Finds all input elements.
            PerformJQueryTest(@"
                    var allInputs = $("":input"");
                    var formChildren = $(""form > *"");
                    $(""#messages"").text(""Found "" + allInputs.length + "" inputs and the form has "" +
                                             formChildren.length + "" children."");
                    // so it won't submit
                    $(""form"").submit(function () { return false; }); 
            ");
        }

        [TestMethod]
        public void only_child_Example1()
        {
            // Test Block 142.
            // Entry only-child: Selects all elements that are the only child of their parent.
            // Example 1: Change the text and add a border for each button that is the only child of its parent.
            PerformJQueryTest(@"
                  $(""div button:only-child"").text(""Alone"").css(""border"", ""2px blue solid"");
            ");
        }

        [TestMethod]
        public void last_child_Example1()
        {
            // Test Block 143.
            // Entry last-child: Selects all elements that are the last child of their parent.
            // Example 1: Finds the last span in each matched div and adds some css plus a hover state.
            PerformJQueryTest(@"
                    $(""div span:last-child"")
                        .css({color:""red"", fontSize:""80%""})
                        .hover(function () {
                              $(this).addClass(""solast"");
                            }, function () {
                              $(this).removeClass(""solast"");
                            });
            ");
        }

        [TestMethod]
        public void first_child_Example1()
        {
            // Test Block 144.
            // Entry first-child: Selects all elements that are the first child of their parent.
            // Example 1: Finds the first span in each matched div to underline and add a hover state.
            PerformJQueryTest(@"
                    $(""div span:first-child"")
                        .css(""text-decoration"", ""underline"")
                        .hover(function () {
                              $(this).addClass(""sogreen"");
                            }, function () {
                              $(this).removeClass(""sogreen"");
                            });
            ");
        }

        [TestMethod]
        public void nth_child_Example1()
        {
            // Test Block 145.
            // Entry nth-child: Selects all elements that are the nth-child of their parent.
            // Example 1: Finds the second li in each matched ul and notes it.
            PerformJQueryTest(@"
                $(""ul li:nth-child(2)"").append(""<span> - 2nd!</span>"");
            ");
        }

        [TestMethod]
        public void nth_child_Example2()
        {
            // Test Block 146.
            // Entry nth-child: Selects all elements that are the nth-child of their parent.
            // Example 2: This is a playground to see how the selector works with different strings.  Notice that this is different from the :even and :odd which have no regard for parent and just filter the list of elements to every other one.  The :nth-child, however, counts the index of the child to its particular parent.  In any case, it's easier to see than explain so...
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      var str = $(this).text();
                      $(""tr"").css(""background"", ""white"");
                      $(""tr"" + str).css(""background"", ""#ff0000"");
                      $(""#inner"").text(str);
                    });
            ");
        }

        [TestMethod]
        public void attributeContainsPrefix_Example1()
        {
            // Test Block 147.
            // Entry attributeContainsPrefix: Selects elements that have the specified attribute with a value either equal to a given string or starting with that string followed by a hyphen (-).
            // Example 1: Finds all links with an hreflang attribute that is english.
            PerformJQueryTest(@"
                $('a[hreflang|=""en""]').css('border','3px dotted green');
            ");
        }

        [TestMethod]
        public void attributeContainsWord_Example1()
        {
            // Test Block 148.
            // Entry attributeContainsWord: Selects elements that have the specified attribute with a value containing a given word, delimited by spaces.
            // Example 1: Finds all inputs with a name attribute that contains the word 'man' and sets the value with some text.
            PerformJQueryTest(@"
                $('input[name~=""man""]').val('mr. man is in it!');
            ");
        }

        [TestMethod]
        public void attributeMultiple_Example1()
        {
            // Test Block 149.
            // Entry attributeMultiple: Matches elements that match all of the specified attribute filters.
            // Example 1: Finds all inputs that have an id attribute and whose name attribute ends with man and sets the value.
            PerformJQueryTest(@"
                $('input[id][name$=""man""]').val('only this one');
            ");
        }

        [TestMethod]
        public void attributeContains_Example1()
        {
            // Test Block 150.
            // Entry attributeContains: Selects elements that have the specified attribute with a value containing the a given substring.
            // Example 1: Finds all inputs with a name attribute that contains 'man' and sets the value with some text.
            PerformJQueryTest(@"
                $('input[name*=""man""]').val('has man in it!');
            ");
        }

        [TestMethod]
        public void attributeEndsWith_Example1()
        {
            // Test Block 151.
            // Entry attributeEndsWith: Selects elements that have the specified attribute with a value ending exactly with a given string. The comparison is case sensitive.
            // Example 1: Finds all inputs with an attribute name that ends with 'letter' and puts text in them.
            PerformJQueryTest(@"
                $('input[name$=""letter""]').val('a letter');
            ");
        }

        [TestMethod]
        public void attributeStartsWith_Example1()
        {
            // Test Block 152.
            // Entry attributeStartsWith: Selects elements that have the specified attribute with a value beginning exactly with a given string.
            // Example 1: Finds all inputs with an attribute name that starts with 'news' and puts text in them.
            PerformJQueryTest(@"
                $('input[name^=""news""]').val('news here!');
            ");
        }

        [TestMethod]
        public void attributeNotEqual_Example1()
        {
            // Test Block 153.
            // Entry attributeNotEqual: Select elements that either don't have the specified attribute, or do have the specified attribute but not with a certain value.
            // Example 1: Finds all inputs that don't have the name 'newsletter' and appends text to the span next to it.
            PerformJQueryTest(@"
                $('input[name!=""newsletter""]').next().append('<b>; not newsletter</b>');
            ");
        }

        [TestMethod]
        public void attributeEquals_Example1()
        {
            // Test Block 154.
            // Entry attributeEquals: Selects elements that have the specified attribute with a value exactly equal to a certain value.
            // Example 1: Finds all inputs with a value of "Hot Fuzz" and changes the text of the next sibling span.
            PerformJQueryTest(@"
                $('input[value=""Hot Fuzz""]').next().text("" Hot Fuzz"");
            ");
        }

        [TestMethod]
        public void attributeHas_Example1()
        {
            // Test Block 155.
            // Entry attributeHas: Selects elements that have the specified attribute, with any value. 
            // Example 1: Bind a single click that adds the div id to its text.
            PerformJQueryTest(@"
                    $('div[id]').one('click', function(){
                      var idString = $(this).text() + ' = ' + $(this).attr('id');
                      $(this).text(idString);
                    });
            ");
        }

        [TestMethod]
        public void visible_Example1()
        {
            // Test Block 156.
            // Entry visible: Selects all elements that are visible.
            // Example 1: Make all visible divs turn yellow on click.
            PerformJQueryTest(@"
                    $(""div:visible"").click(function () {
                      $(this).css(""background"", ""yellow"");
                    });
                    $(""button"").click(function () {
                      $(""div:hidden"").show(""fast"");
                    });
            ");
        }

        [TestMethod]
        public void hidden_Example1()
        {
            // Test Block 157.
            // Entry hidden: Selects all elements that are hidden.
            // Example 1: Shows all hidden divs and counts hidden inputs.
            PerformJQueryTest(@"
                // in some browsers :hidden includes head, title, script, etc...
                var hiddenEls = $(""body"").find("":hidden"").not(""script"");
                $(""span:first"").text(""Found "" + hiddenEls.length + "" hidden elements total."");
                $(""div:hidden"").show(3000);
                $(""span:last"").text(""Found "" + $(""input:hidden"").length + "" hidden inputs."");
            ");
        }

        [TestMethod]
        public void parent_Example1()
        {
            // Test Block 158.
            // Entry parent: Select all elements that are the parent of another element, including text nodes.
            // Example 1: Finds all tds with children, including text.
            PerformJQueryTest(@"
                $(""td:parent"").fadeTo(1500, 0.3);
            ");
        }

        [TestMethod]
        public void has_1_Example1()
        {
            // Test Block 159.
            // Entry has_1: Selects elements which contain at least one element that matches the specified selector.
            // Example 1: Adds the class "test" to all divs that have a paragraph inside of them.
            PerformJQueryTest(@"
                $(""div:has(p)"").addClass(""test"");
            ");
        }

        [TestMethod]
        public void empty_Example1()
        {
            // Test Block 160.
            // Entry empty: Select all elements that have no children (including text nodes).
            // Example 1: Finds all elements that are empty - they don't have child elements or text.
            PerformJQueryTest(@"
                $(""td:empty"").text(""Was empty!"").css('background', 'rgb(255,220,200)');
            ");
        }

        [TestMethod]
        public void contains_Example1()
        {
            // Test Block 161.
            // Entry contains: Select all elements that contain the specified text.
            // Example 1: Finds all divs containing "John" and underlines them.
            PerformJQueryTest(@"
                $(""div:contains('John')"").css(""text-decoration"", ""underline"");
            ");
        }

        [TestMethod]
        public void animated_Example1()
        {
            // Test Block 162.
            // Entry animated: Select all elements that are in the progress of an animation at the time the selector is run.
            // Example 1: Change the color of any div that is animated.
            PerformJQueryTest(@"
                    $(""#run"").click(function(){
                      $(""div:animated"").toggleClass(""colored"");
                    });
                    function animateIt() {
                      $(""#mover"").slideToggle(""slow"", animateIt);
                    }
                    animateIt();
            ");
        }

        [TestMethod]
        public void header_Example1()
        {
            // Test Block 163.
            // Entry header: Selects all elements that are headers, like h1, h2, h3 and so on.
            // Example 1: Adds a background and text color to all the headers on the page.
            PerformJQueryTest(@"
                $("":header"").css({ background:'#CCC', color:'blue' });
            ");
        }

        [TestMethod]
        public void lt_Example1()
        {
            // Test Block 164.
            // Entry lt: Select all elements at an index less than index within the matched set.
            // Example 1: Finds TDs less than the one with the 4th index (TD#4).
            PerformJQueryTest(@"
                $(""td:lt(4)"").css(""color"", ""red"");
            ");
        }

        [TestMethod]
        public void gt_Example1()
        {
            // Test Block 165.
            // Entry gt: Select all elements at an index greater than index within the matched set.
            // Example 1: Finds TD #5 and higher. Reminder: the indexing starts at 0.
            PerformJQueryTest(@"
                $(""td:gt(4)"").css(""text-decoration"", ""line-through"");
            ");
        }

        [TestMethod]
        public void eq_Example1()
        {
            // Test Block 166.
            // Entry eq: Select the element at index n within the matched set.
            // Example 1: Finds the third td.
            PerformJQueryTest(@"
                $(""td:eq(2)"").css(""color"", ""red"");
            ");
        }

        [TestMethod]
        public void eq_Example2()
        {
            // Test Block 167.
            // Entry eq: Select the element at index n within the matched set.
            // Example 2: Apply three different styles to list items to demonstrate that :eq() is designed to select a single element while :nth-child() or :eq() within a looping construct such as .each() can select multiple elements.
            PerformJQueryTest(@"
                // applies yellow background color to a single <li>
                $(""ul.nav li:eq(1)"").css( ""backgroundColor"", ""#ff0"" );
                // applies italics to text of the second <li> within each <ul class=""nav"">
                $(""ul.nav"").each(function(index) {
                  $(this).find(""li:eq(1)"").css( ""fontStyle"", ""italic"" );
                });
                // applies red text color to descendants of <ul class=""nav"">
                // for each <li> that is the second child of its parent
                $(""ul.nav li:nth-child(2)"").css( ""color"", ""red"" );
            ");
        }

        [TestMethod]
        public void odd_Example1()
        {
            // Test Block 168.
            // Entry odd: Selects odd elements, zero-indexed.  See also even.
            // Example 1: Finds odd table rows, matching the second, fourth and so on (index 1, 3, 5 etc.).
            PerformJQueryTest(@"
                $(""tr:odd"").css(""background-color"", ""#bbbbff"");
            ");
        }

        [TestMethod]
        public void even_Example1()
        {
            // Test Block 169.
            // Entry even: Selects even elements, zero-indexed.  See also odd.
            // Example 1: Finds even table rows, matching the first, third and so on (index 0, 2, 4 etc.).
            PerformJQueryTest(@"
                $(""tr:even"").css(""background-color"", ""#bbbbff"");
            ");
        }

        [TestMethod]
        public void not_Example1()
        {
            // Test Block 170.
            // Entry not: Selects all elements that do not match the given selector.
            // Example 1: Finds all inputs that are not checked and highlights the next sibling span.  Notice there is no change when clicking the checkboxes since no click events have been linked.
            PerformJQueryTest(@"
                  $(""input:not(:checked) + span"").css(""background-color"", ""yellow"");
                  $(""input"").attr(""disabled"", ""disabled"");
            ");
        }

        [TestMethod]
        public void last_Example1()
        {
            // Test Block 171.
            // Entry last: Selects the last matched element.
            // Example 1: Finds the last table row.
            PerformJQueryTest(@"
                $(""tr:last"").css({backgroundColor: 'yellow', fontWeight: 'bolder'});
            ");
        }

        [TestMethod]
        public void first_Example1()
        {
            // Test Block 172.
            // Entry first: Selects the first matched element.
            // Example 1: Finds the first table row.
            PerformJQueryTest(@"
                $(""tr:first"").css(""font-style"", ""italic"");
            ");
        }

        [TestMethod]
        public void next_siblings_Example1()
        {
            // Test Block 173.
            // Entry next siblings: Selects all sibling elements that follow after the "prev" element, have the same parent, and match the filtering "siblings" selector.
            // Example 1: Finds all divs that are siblings after the element with #prev as its id.  Notice the span isn't selected since it is not a div and the "niece" isn't selected since it is a child of a sibling, not an actual sibling.
            PerformJQueryTest(@"
                $(""#prev ~ div"").css(""border"", ""3px groove blue"");
            ");
        }

        [TestMethod]
        public void next_adjacent_Example1()
        {
            // Test Block 174.
            // Entry next adjacent: Selects all next elements matching "next" that are immediately preceded by a sibling "prev".
            // Example 1: Finds all inputs that are next to a label.
            PerformJQueryTest(@"
                $(""label + input"").css(""color"", ""blue"").val(""Labeled!"")
            ");
        }

        [TestMethod]
        public void child_Example1()
        {
            // Test Block 175.
            // Entry child: Selects all direct child elements specified by "child" of elements specified by "parent".
            // Example 1: Places a border around all list items that are children of <ul class="topnav"> .
            PerformJQueryTest(@"
                $(""ul.topnav > li"").css(""border"", ""3px double red"");
            ");
        }

        [TestMethod]
        public void descendant_Example1()
        {
            // Test Block 176.
            // Entry descendant: Selects all elements that are descendants of a given ancestor.
            // Example 1: Finds all input descendants of forms.
            PerformJQueryTest(@"
                $(""form input"").css(""border"", ""2px dotted blue"");
            ");
        }

        [TestMethod]
        public void multiple_Example1()
        {
            // Test Block 177.
            // Entry multiple: Selects the combined results of all the specified selectors.
            // Example 1: Finds the elements that match any of these three selectors.
            PerformJQueryTest(@"
                $(""div,span,p.myClass"").css(""border"",""3px solid red"");
            ");
        }

        [TestMethod]
        public void multiple_Example2()
        {
            // Test Block 178.
            // Entry multiple: Selects the combined results of all the specified selectors.
            // Example 2: Show the order in the jQuery object.
            PerformJQueryTest(@"
                    var list = $(""div,p,span"").map(function () {
                      return this.tagName;
                    }).get().join("", "");
                    $(""b"").append(document.createTextNode(list));
            ");
        }

        [TestMethod]
        public void all_Example1()
        {
            // Test Block 179.
            // Entry all: Selects all elements.
            // Example 1: Finds every element (including head, body, etc) in the document.
            PerformJQueryTest(@"
                var elementCount = $(""*"").css(""border"",""3px solid red"").length;
                $(""body"").prepend(""<h3>"" + elementCount + "" elements found</h3>"");
            ");
        }

        [TestMethod]
        public void all_Example2()
        {
            // Test Block 180.
            // Entry all: Selects all elements.
            // Example 2: A common way to select all elements is to find within document.body so elements like head, script, etc are left out.
            PerformJQueryTest(@"
                var elementCount = $(""#test"").find(""*"").css(""border"",""3px solid red"").length;
                $(""body"").prepend(""<h3>"" + elementCount + "" elements found</h3>"");
            ");
        }

        [TestMethod]
        public void class_Example1()
        {
            // Test Block 181.
            // Entry class: Selects all elements with the given class. 
            // Example 1: Finds the element with the class "myClass".
            PerformJQueryTest(@"
                $("".myClass"").css(""border"",""3px solid red"");
            ");
        }

        [TestMethod]
        public void class_Example2()
        {
            // Test Block 182.
            // Entry class: Selects all elements with the given class. 
            // Example 2: Finds the element with both "myclass" and "otherclass" classes.
            PerformJQueryTest(@"
                $("".myclass.otherclass"").css(""border"",""13px solid red"");
            ");
        }

        [TestMethod]
        public void element_Example1()
        {
            // Test Block 183.
            // Entry element: Selects all elements with the given tag name.
            // Example 1: Finds every DIV element.
            PerformJQueryTest(@"
                $(""div"").css(""border"",""9px solid red"");
            ");
        }

        [TestMethod]
        public void id_Example1()
        {
            // Test Block 184.
            // Entry id: Selects a single element with the given id attribute. 
            // Example 1: Finds the element with the id "myDiv".
            PerformJQueryTest(@"
                $(""#myDiv"").css(""border"",""3px solid red"");
            ");
        }

        [TestMethod]
        public void id_Example2()
        {
            // Test Block 185.
            // Entry id: Selects a single element with the given id attribute. 
            // Example 2: Finds the element with the id "myID.entry[1]".  See how certain characters must be escaped with backslashes.
            PerformJQueryTest(@"
                $(""#myID\\.entry\\[1\\]"").css(""border"",""3px solid red"");
            ");
        }

        [TestMethod]
        public void scroll_Example1()
        {
            // Test Block 186.
            // Entry scroll: Bind an event handler to the "scroll" JavaScript event, or trigger that event on an element.
            // Example 1: To do something when your page is scrolled:
            PerformJQueryTest(@"
                    $(""p"").clone().appendTo(document.body);
                    $(""p"").clone().appendTo(document.body);
                    $(""p"").clone().appendTo(document.body);
                    $(window).scroll(function () { 
                      $(""span"").css(""display"", ""inline"").fadeOut(""slow""); 
                    });
            ");
        }

        [TestMethod]
        public void resize_Example1()
        {
            // Test Block 187.
            // Entry resize: Bind an event handler to the "resize" JavaScript event, or trigger that event on an element.
            // Example 1: To see the window width while (or after) it is resized, try:
            PerformJQueryTest(@"
                $(window).resize(function() {
                  $('body').prepend('<div>' + $(window).width() + '</div>');
                });
            ");
        }

        [TestMethod]
        public void dequeue_Example1()
        {
            // Test Block 188.
            // Entry dequeue: Execute the next function on the queue for the matched elements.
            // Example 1: Use dequeue to end a custom queue function which allows the queue to keep going.
            PerformJQueryTest(@"
                $(""button"").click(function () {
                  $(""div"").animate({left:'+=200px'}, 2000);
                  $(""div"").animate({top:'0px'}, 600);
                  $(""div"").queue(function () {
                    $(this).toggleClass(""red"");
                    $(this).dequeue();
                  });
                  $(""div"").animate({left:'10px', top:'30px'}, 700);
                });
            ");
        }

        [TestMethod]
        public void queue_Example1()
        {
            // Test Block 189.
            // Entry queue: Show the queue of functions to be executed on the matched elements.
            // Example 1: Show the length of the queue.
            PerformJQueryTest(@"
                var div = $(""div"");
                function runIt() {
                  div.show(""slow"");
                  div.animate({left:'+=200'},2000);
                  div.slideToggle(1000);
                  div.slideToggle(""fast"");
                  div.animate({left:'-=200'},1500);
                  div.hide(""slow"");
                  div.show(1200);
                  div.slideUp(""normal"", runIt);
                }
                function showIt() {
                  var n = div.queue(""fx"");
                  $(""span"").text( n.length );      
                  setTimeout(showIt, 100);
                }
                runIt();
                showIt();
            ");
        }

        [TestMethod]
        public void queue_1_Example1()
        {
            // Test Block 190.
            // Entry queue_1: Manipulate the queue of functions to be executed on the matched elements.
            // Example 1: Queue a custom function.
            PerformJQueryTest(@"
                $(document.body).click(function () {
                      $(""div"").show(""slow"");
                      $(""div"").animate({left:'+=200'},2000);
                      $(""div"").queue(function () {
                        $(this).addClass(""newcolor"");
                        $(this).dequeue();
                      });
                      $(""div"").animate({left:'-=200'},500);
                      $(""div"").queue(function () {
                        $(this).removeClass(""newcolor"");
                        $(this).dequeue();
                      });
                      $(""div"").slideUp();
                    });
            ");
        }

        [TestMethod]
        public void queue_1_Example2()
        {
            // Test Block 191.
            // Entry queue_1: Manipulate the queue of functions to be executed on the matched elements.
            // Example 2: Set a queue array to delete the queue.
            PerformJQueryTest(@"
                $(""#start"").click(function () {
                      $(""div"").show(""slow"");
                      $(""div"").animate({left:'+=200'},5000);
                      $(""div"").queue(function () {
                        $(this).addClass(""newcolor"");
                        $(this).dequeue();
                      });
                      $(""div"").animate({left:'-=200'},1500);
                      $(""div"").queue(function () {
                        $(this).removeClass(""newcolor"");
                        $(this).dequeue();
                      });
                      $(""div"").slideUp();
                    });
                    $(""#stop"").click(function () {
                      $(""div"").queue(""fx"", []);
                      $(""div"").stop();
                    });
            ");
        }

        [TestMethod]
        public void keyup_Example1()
        {
            // Test Block 192.
            // Entry keyup: Bind an event handler to the "keyup" JavaScript event, or trigger that event on an element.
            // Example 1: Show the event object for the keyup handler (using a simple $.print plugin) when a key is released in the input.
            PerformJQueryTest(@"
                var xTriggered = 0;
                $('#target').keyup(function(event) {
                   xTriggered++;
                   var msg = 'Handler for .keyup() called ' + xTriggered + ' time(s).';
                  $.print(msg, 'html');
                  $.print(event);
                }).keydown(function(event) {
                  if (event.which == 13) {
                    event.preventDefault();
                  }  
                });
                $('#other').click(function() {
                  $('#target').keyup();
                });
            ", "print", "print");
        }

        [TestMethod]
        public void keypress_Example1()
        {
            // Test Block 193.
            // Entry keypress: Bind an event handler to the "keypress" JavaScript event, or trigger that event on an element.
            // Example 1: Show the event object when a key is pressed in the input. Note: This demo relies on a simple $.print() plugin (http://api.jquery.com/scripts/events.js) for the event object's output.
            PerformJQueryTest(@"
                var xTriggered = 0;
                $(""#target"").keypress(function(event) {
                  if ( event.which == 13 ) {
                     event.preventDefault();
                   }
                   xTriggered++;
                   var msg = ""Handler for .keypress() called "" + xTriggered + "" time(s)."";
                  $.print( msg, ""html"" );
                  $.print( event );
                });
                $(""#other"").click(function() {
                  $(""#target"").keypress();
                });
            ", "print", "print");
        }

        [TestMethod]
        public void submit_1_Example1()
        {
            // Test Block 194.
            // Entry submit_1: Bind an event handler to the "submit" JavaScript event, or trigger that event on an element.
            // Example 1: If you'd like to prevent forms from being submitted unless a flag variable is set, try:
            PerformJQueryTest(@"
                    $(""form"").submit(function() {
                      if ($(""input:first"").val() == ""correct"") {
                        $(""span"").text(""Validated..."").show();
                        return true;
                      }
                      $(""span"").text(""Not valid!"").show().fadeOut(1000);
                      return false;
                    });
            ");
        }

        [TestMethod]
        public void submit_1_Example2()
        {
            // Test Block 195.
            // Entry submit_1: Bind an event handler to the "submit" JavaScript event, or trigger that event on an element.
            // Example 2: If you'd like to prevent forms from being submitted unless a flag variable is set, try:
            PerformJQueryTest(@"
                $(""form"").submit( function () {
                  return this.some_flag_variable;
                } );
            ", "some_flag_variable");
        }

        [TestMethod]
        public void submit_1_Example3()
        {
            // Test Block 196.
            // Entry submit_1: Bind an event handler to the "submit" JavaScript event, or trigger that event on an element.
            // Example 3: To trigger the submit event on the first form on the page, try:
            PerformJQueryTest(@"
                $(""form:first"").submit();
            ");
        }

        [TestMethod]
        public void select_Example1()
        {
            // Test Block 197.
            // Entry select: Bind an event handler to the "select" JavaScript event, or trigger that event on an element.
            // Example 1: To do something when text in input boxes is selected:
            PerformJQueryTest(@"
                    $("":input"").select( function () { 
                      $(""div"").text(""Something was selected"").show().fadeOut(1000); 
                    });
            ");
        }

        [TestMethod]
        public void select_Example2()
        {
            // Test Block 198.
            // Entry select: Bind an event handler to the "select" JavaScript event, or trigger that event on an element.
            // Example 2: To trigger the select event on all input elements, try:
            PerformJQueryTest(@"
                $(""input"").select();
            ");
        }

        [TestMethod]
        public void change_Example1()
        {
            // Test Block 199.
            // Entry change: Bind an event handler to the "change" JavaScript event, or trigger that event on an element.
            // Example 1: Attaches a change event to the select that gets the text for each selected option and writes them in the div.  It then triggers the event for the initial text draw.
            PerformJQueryTest(@"
                    $(""select"").change(function () {
                          var str = """";
                          $(""select option:selected"").each(function () {
                                str += $(this).text() + "" "";
                              });
                          $(""div"").text(str);
                        })
                        .change();
            ");
        }

        [TestMethod]
        public void change_Example2()
        {
            // Test Block 200.
            // Entry change: Bind an event handler to the "change" JavaScript event, or trigger that event on an element.
            // Example 2: To add a validity test to all text input elements:
            PerformJQueryTest(@"
                $(""input[type='text']"").change( function() {
                  // check input ($(this).val()) for validity here
                });
            ");
        }

        [TestMethod]
        public void blur_Example1()
        {
            // Test Block 201.
            // Entry blur: Bind an event handler to the "blur" JavaScript event, or trigger that event on an element.
            // Example 1: To trigger the blur event on all paragraphs:
            PerformJQueryTest(@"
                $(""p"").blur();
            ");
        }

        [TestMethod]
        public void focus_1_Example1()
        {
            // Test Block 202.
            // Entry focus_1: Bind an event handler to the "focus" JavaScript event, or trigger that event on an element.
            // Example 1: Fire focus.
            PerformJQueryTest(@"
                    $(""input"").focus(function () {
                         $(this).next(""span"").css('display','inline').fadeOut(1000);
                    });
            ");
        }

        [TestMethod]
        public void focus_1_Example2()
        {
            // Test Block 203.
            // Entry focus_1: Bind an event handler to the "focus" JavaScript event, or trigger that event on an element.
            // Example 2: To stop people from writing in text input boxes, try:
            PerformJQueryTest(@"
                $(""input[type=text]"").focus(function(){
                  $(this).blur();
                });
            ");
        }

        [TestMethod]
        public void focus_1_Example3()
        {
            // Test Block 204.
            // Entry focus_1: Bind an event handler to the "focus" JavaScript event, or trigger that event on an element.
            // Example 3: To focus on a login input box with id 'login' on page startup, try:
            PerformJQueryTest(@"
                $(document).ready(function(){
                  $(""#login"").focus();
                });
            ");
        }

        [TestMethod]
        public void mousemove_Example1()
        {
            // Test Block 205.
            // Entry mousemove: Bind an event handler to the "mousemove" JavaScript event, or trigger that event on an element.
            // Example 1: Show the mouse coordinates when the mouse is moved over the yellow div.  Coordinates are relative to the window, which in this case is the iframe.
            PerformJQueryTest(@"
                    $(""div"").mousemove(function(e){
                      var pageCoords = ""( "" + e.pageX + "", "" + e.pageY + "" )"";
                      var clientCoords = ""( "" + e.clientX + "", "" + e.clientY + "" )"";
                      $(""span:first"").text(""( e.pageX, e.pageY ) - "" + pageCoords);
                      $(""span:last"").text(""( e.clientX, e.clientY ) - "" + clientCoords);
                    });
            ");
        }

        [TestMethod]
        public void hover_Example1()
        {
            // Test Block 206.
            // Entry hover: Bind two handlers to the matched elements, to be executed when the mouse pointer enters and leaves the elements.
            // Example 1: To add a special style to list items that are being hovered over, try:
            PerformJQueryTest(@"
                $(""li"").hover(
                  function () {
                    $(this).append($(""<span> ***</span>""));
                  }, 
                  function () {
                    $(this).find(""span:last"").remove();
                  }
                );
                //li with fade class
                $(""li.fade"").hover(function(){$(this).fadeOut(100);$(this).fadeIn(500);});
            ");
        }

        [TestMethod]
        public void hover_Example2()
        {
            // Test Block 207.
            // Entry hover: Bind two handlers to the matched elements, to be executed when the mouse pointer enters and leaves the elements.
            // Example 2: To add a special style to table cells that are being hovered over, try:
            PerformJQueryTest(@"
                $(""td"").hover(
                  function () {
                    $(this).addClass(""hover"");
                  },
                  function () {
                    $(this).removeClass(""hover"");
                  }
                );
            ");
        }

        [TestMethod]
        public void hover_Example3()
        {
            // Test Block 208.
            // Entry hover: Bind two handlers to the matched elements, to be executed when the mouse pointer enters and leaves the elements.
            // Example 3: To unbind the above example use:
            PerformJQueryTest(@"
                $(""td"").unbind('mouseenter mouseleave');
            ");
        }

        [TestMethod]
        public void hover_1_Example1()
        {
            // Test Block 209.
            // Entry hover_1: Bind a single handler to the matched elements, to be executed when the mouse pointer enters or leaves the elements.
            // Example 1: Slide the next sibling LI up or down on hover, and toggle a class.
            PerformJQueryTest(@"
                $(""li"")
                .filter("":odd"")
                .hide()
                 .end()
                .filter("":even"")
                .hover(
                  function () {
                    $(this).toggleClass(""active"")
                      .next().stop(true, true).slideToggle();
                  }
                );
            ");
        }

        [TestMethod]
        public void mouseleave_Example1()
        {
            // Test Block 210.
            // Entry mouseleave: Bind an event handler to be fired when the mouse leaves an element, or trigger that handler on an element.
            // Example 1: Show number of times mouseout and mouseleave events are triggered.  mouseout fires when the pointer moves out of child element as well, while mouseleave fires only when the pointer moves out of the bound element.
            PerformJQueryTest(@"
                    var i = 0;
                    $(""div.overout"").mouseover(function(){
                      $(""p:first"",this).text(""mouse over"");
                    }).mouseout(function(){
                      $(""p:first"",this).text(""mouse out"");
                      $(""p:last"",this).text(++i);
                    });
                    var n = 0;
                    $(""div.enterleave"").mouseenter(function(){
                      $(""p:first"",this).text(""mouse enter"");
                    }).mouseleave(function(){
                      $(""p:first"",this).text(""mouse leave"");
                      $(""p:last"",this).text(++n);
                    });
            ");
        }

        [TestMethod]
        public void mouseenter_Example1()
        {
            // Test Block 211.
            // Entry mouseenter: Bind an event handler to be fired when the mouse enters an element, or trigger that handler on an element.
            // Example 1: Show texts when mouseenter and mouseout event triggering.  mouseover fires when the pointer moves into the child element as well, while mouseenter fires only when the pointer moves into the bound element.
            PerformJQueryTest(@"
                    var i = 0;
                    $(""div.overout"").mouseover(function(){
                      $(""p:first"",this).text(""mouse over"");
                      $(""p:last"",this).text(++i);
                    }).mouseout(function(){
                      $(""p:first"",this).text(""mouse out"");
                    });
                    var n = 0;
                    $(""div.enterleave"").mouseenter(function(){
                      $(""p:first"",this).text(""mouse enter"");
                      $(""p:last"",this).text(++n);
                    }).mouseleave(function(){
                      $(""p:first"",this).text(""mouse leave"");
                    });
            ");
        }

        [TestMethod]
        public void mouseout_Example1()
        {
            // Test Block 212.
            // Entry mouseout: Bind an event handler to the "mouseout" JavaScript event, or trigger that event on an element.
            // Example 1: Show the number of times mouseout and mouseleave events are triggered.  mouseout fires when the pointer moves out of the child element as well, while mouseleave fires only when the pointer moves out of the bound element.
            PerformJQueryTest(@"
                    var i = 0;
                    $(""div.overout"").mouseout(function(){
                      $(""p:first"",this).text(""mouse out"");
                      $(""p:last"",this).text(++i);
                    }).mouseover(function(){
                      $(""p:first"",this).text(""mouse over"");
                    });
                    var n = 0;
                    $(""div.enterleave"").bind(""mouseenter"",function(){
                      $(""p:first"",this).text(""mouse enter"");
                    }).bind(""mouseleave"",function(){
                      $(""p:first"",this).text(""mouse leave"");
                      $(""p:last"",this).text(++n);
                    });
            ");
        }

        [TestMethod]
        public void mouseover_Example1()
        {
            // Test Block 213.
            // Entry mouseover: Bind an event handler to the "mouseover" JavaScript event, or trigger that event on an element.
            // Example 1: Show the number of times mouseover and mouseenter events are triggered.  mouseover fires when the pointer moves into the child element as well, while mouseenter fires only when the pointer moves into the bound element.
            PerformJQueryTest(@"
                  var i = 0;
                  $(""div.overout"").mouseover(function() {
                    i += 1;
                    $(this).find(""span"").text( ""mouse over x "" + i );
                  }).mouseout(function(){
                    $(this).find(""span"").text(""mouse out "");
                  });
                  var n = 0;
                  $(""div.enterleave"").mouseenter(function() {
                    n += 1;
                    $(this).find(""span"").text( ""mouse enter x "" + n );
                  }).mouseleave(function() {
                    $(this).find(""span"").text(""mouse leave"");
                  });
            ");
        }

        [TestMethod]
        public void dblclick_Example1()
        {
            // Test Block 214.
            // Entry dblclick: Bind an event handler to the "dblclick" JavaScript event, or trigger that event on an element.
            // Example 1: To bind a "Hello World!" alert box the dblclick event on every paragraph on the page:
            PerformJQueryTest(@"
                $(""p"").dblclick( function () { alert(""Hello World!""); });
            ");
        }

        [TestMethod]
        public void dblclick_Example2()
        {
            // Test Block 215.
            // Entry dblclick: Bind an event handler to the "dblclick" JavaScript event, or trigger that event on an element.
            // Example 2: Double click to toggle background color.
            PerformJQueryTest(@"
                    var divdbl = $(""div:first"");
                    divdbl.dblclick(function () { 
                      divdbl.toggleClass('dbl'); 
                    });
            ");
        }

        [TestMethod]
        public void click_Example1()
        {
            // Test Block 216.
            // Entry click: Bind an event handler to the "click" JavaScript event, or trigger that event on an element.
            // Example 1: To hide paragraphs on a page when they are clicked:
            PerformJQueryTest(@"
                    $(""p"").click(function () { 
                      $(this).slideUp(); 
                    });
                    $(""p"").hover(function () {
                      $(this).addClass(""hilite"");
                    }, function () {
                      $(this).removeClass(""hilite"");
                    });
            ");
        }

        [TestMethod]
        public void click_Example2()
        {
            // Test Block 217.
            // Entry click: Bind an event handler to the "click" JavaScript event, or trigger that event on an element.
            // Example 2: To trigger the click event on all of the paragraphs on the page:
            PerformJQueryTest(@"
                $(""p"").click();
            ");
        }

        [TestMethod]
        public void mouseup_Example1()
        {
            // Test Block 218.
            // Entry mouseup: Bind an event handler to the "mouseup" JavaScript event, or trigger that event on an element.
            // Example 1: Show texts when mouseup and mousedown event triggering.
            PerformJQueryTest(@"
                    $(""p"").mouseup(function(){
                      $(this).append('<span style=""color:#F00;"">Mouse up.</span>');
                    }).mousedown(function(){
                      $(this).append('<span style=""color:#00F;"">Mouse down.</span>');
                    });
            ");
        }

        [TestMethod]
        public void mousedown_Example1()
        {
            // Test Block 219.
            // Entry mousedown: Bind an event handler to the "mousedown" JavaScript event, or trigger that event on an element.
            // Example 1: Show texts when mouseup and mousedown event triggering.
            PerformJQueryTest(@"
                    $(""p"").mouseup(function(){
                      $(this).append('<span style=""color:#F00;"">Mouse up.</span>');
                    }).mousedown(function(){
                      $(this).append('<span style=""color:#00F;"">Mouse down.</span>');
                    });
            ");
        }

        [TestMethod]
        public void error_Example1()
        {
            // Test Block 220.
            // Entry error: Bind an event handler to the "error" JavaScript event.
            // Example 1: To hide the "broken image" icons for IE users, you can try:
            PerformJQueryTest(@"
                $(""img"")
                  .error(function(){
                    $(this).hide();
                  })
                  .attr(""src"", ""missing.png"");
            ");
        }

        [TestMethod]
        public void unload_Example1()
        {
            // Test Block 221.
            // Entry unload: Bind an event handler to the "unload" JavaScript event.
            // Example 1: To display an alert when a page is unloaded:
            PerformJQueryTest(@"
                $(window).unload( function () { alert(""Bye now!""); } );
            ");
        }

        [TestMethod]
        public void load_Example1()
        {
            // Test Block 222.
            // Entry load: Bind an event handler to the "load" JavaScript event.
            // Example 1: Run a function when the page is fully loaded including graphics.
            PerformJQueryTest(@"
                $(window).load(function () {
                  // run code
                });
            ");
        }

        [TestMethod]
        public void load_Example2()
        {
            // Test Block 223.
            // Entry load: Bind an event handler to the "load" JavaScript event.
            // Example 2: Add the class bigImg to all images with height greater then 100 upon each image load.
            PerformJQueryTest(@"
                $('img.userIcon').load(function(){
                  if($(this).height() > 100) {
                    $(this).addClass('bigImg');
                  }
                });
            ");
        }

        [TestMethod]
        public void ready_Example1()
        {
            // Test Block 224.
            // Entry ready: Specify a function to execute when the DOM is fully loaded.
            // Example 1: Display a message when the DOM is loaded.
            PerformJQueryTest(@"
                $(document).ready(function () {
                  $(""p"").text(""The DOM is now loaded and can be manipulated."");
                });
            ");
        }

        [TestMethod]
        public void die_1_Example1()
        {
            // Test Block 225.
            // Entry die_1: Remove an event handler previously attached using .live() from the elements.
            // Example 1: Can bind and unbind events to the colored button.
            PerformJQueryTest(@"
                function aClick() {
                  $(""div"").show().fadeOut(""slow"");
                }
                $(""#bind"").click(function () {
                  $(""#theone"").live(""click"", aClick)
                              .text(""Can Click!"");
                });
                $(""#unbind"").click(function () {
                  $(""#theone"").die(""click"", aClick)
                              .text(""Does nothing..."");
                });
            ");
        }

        [TestMethod]
        public void die_1_Example2()
        {
            // Test Block 226.
            // Entry die_1: Remove an event handler previously attached using .live() from the elements.
            // Example 2: To unbind all live events from all paragraphs, write:
            PerformJQueryTest(@"
                $(""p"").die()
            ");
        }

        [TestMethod]
        public void die_1_Example3()
        {
            // Test Block 227.
            // Entry die_1: Remove an event handler previously attached using .live() from the elements.
            // Example 3: To unbind all live click events from all paragraphs, write:
            PerformJQueryTest(@"
                $(""p"").die( ""click"" )
            ");
        }

        [TestMethod]
        public void die_1_Example4()
        {
            // Test Block 228.
            // Entry die_1: Remove an event handler previously attached using .live() from the elements.
            // Example 4: To unbind just one previously bound handler, pass the function in as the second argument:
            PerformJQueryTest(@"
                var foo = function () {
                // code to handle some kind of event
                };
                $(""p"").live(""click"", foo); // ... now foo will be called when paragraphs are clicked ...
                $(""p"").die(""click"", foo); // ... foo will no longer be called.
            ");
        }

        [TestMethod]
        public void jQuery_browser_Example1()
        {
            // Test Block 229.
            // Entry jQuery.browser: Contains flags for the useragent, read from navigator.userAgent. We recommend against using this property; please try to use feature detection instead (see jQuery.support). jQuery.browser may be moved to a plugin in a future release of jQuery.
            // Example 1: Show the browser info.
            PerformJQueryTest(@"
                    jQuery.each(jQuery.browser, function(i, val) {
                      $(""<div>"" + i + "" : <span>"" + val + ""</span>"")
                                .appendTo( document.body );
                    });
            ");
        }

        [TestMethod]
        public void jQuery_browser_Example2()
        {
            // Test Block 230.
            // Entry jQuery.browser: Contains flags for the useragent, read from navigator.userAgent. We recommend against using this property; please try to use feature detection instead (see jQuery.support). jQuery.browser may be moved to a plugin in a future release of jQuery.
            // Example 2: Returns true if the current useragent is some version of Microsoft's Internet Explorer.
            PerformJQueryTest(@"
                  $.browser.msie;
            ");
        }

        [TestMethod]
        public void jQuery_browser_Example3()
        {
            // Test Block 231.
            // Entry jQuery.browser: Contains flags for the useragent, read from navigator.userAgent. We recommend against using this property; please try to use feature detection instead (see jQuery.support). jQuery.browser may be moved to a plugin in a future release of jQuery.
            // Example 3: Alerts "this is WebKit!" only for WebKit browsers
            PerformJQueryTest(@"
                  if ($.browser.webkit) {
                    alert( ""this is webkit!"" );
                  }
            ", "webkit");
        }

        [TestMethod]
        public void jQuery_browser_Example4()
        {
            // Test Block 232.
            // Entry jQuery.browser: Contains flags for the useragent, read from navigator.userAgent. We recommend against using this property; please try to use feature detection instead (see jQuery.support). jQuery.browser may be moved to a plugin in a future release of jQuery.
            // Example 4: Alerts "Do stuff for Firefox 3" only for Firefox 3 browsers.
            PerformJQueryTest(@"
                  var ua = $.browser;
                  if ( ua.mozilla && ua.version.slice(0,3) == ""1.9"" ) {
                    alert( ""Do stuff for firefox 3"" );
                  }
            ", "mozilla");
        }

        [TestMethod]
        public void jQuery_browser_Example5()
        {
            // Test Block 233.
            // Entry jQuery.browser: Contains flags for the useragent, read from navigator.userAgent. We recommend against using this property; please try to use feature detection instead (see jQuery.support). jQuery.browser may be moved to a plugin in a future release of jQuery.
            // Example 5: Set a CSS property that's specific to a particular browser.
            PerformJQueryTest(@"
                 if ( $.browser.msie ) {
                    $(""#div ul li"").css( ""display"",""inline"" );
                 } else {
                    $(""#div ul li"").css( ""display"",""inline-table"" );
                 }
            ");
        }

        [TestMethod]
        public void jQuery_browser_version_Example1()
        {
            // Test Block 234.
            // Entry jQuery.browser.version: The version number of the rendering engine for the user's browser.
            // Example 1: Returns the version number of the rendering engine used by the user's current browser. For example, FireFox 4 returns 2.0 (the version of the Gecko rendering engine it utilizes).
            PerformJQueryTest(@"
                $(""p"").html( ""The version number of the rendering engine your browser uses is: <span>"" +
                                $.browser.version + ""</span>"" );
            ");
        }

        [TestMethod]
        public void jQuery_browser_version_Example2()
        {
            // Test Block 235.
            // Entry jQuery.browser.version: The version number of the rendering engine for the user's browser.
            // Example 2: Alerts the version of IE's rendering engine that is being used:
            PerformJQueryTest(@"
                if ( $.browser.msie ) {
                  alert( $.browser.version );
                }
            ");
        }

        [TestMethod]
        public void jQuery_browser_version_Example3()
        {
            // Test Block 236.
            // Entry jQuery.browser.version: The version number of the rendering engine for the user's browser.
            // Example 3: Often you only care about the "major number," the whole number, which you can get by using JavaScript's built-in parseInt() function:
            PerformJQueryTest(@"
                if ( $.browser.msie ) {
                  alert( parseInt($.browser.version, 10) );
                }
            ");
        }

        [TestMethod]
        public void live_Example1()
        {
            // Test Block 237.
            // Entry live: Attach an event handler for all elements which match the current selector, now and in the future.
            // Example 1: Click a paragraph to add another. Note that .live() binds the click event to all paragraphs - even new ones.
            PerformJQueryTest(@"
                $(""p"").live(""click"", function(){
                  $(this).after(""<p>Another paragraph!</p>"");
                });
            ");
        }

        [TestMethod]
        public void live_Example2()
        {
            // Test Block 238.
            // Entry live: Attach an event handler for all elements which match the current selector, now and in the future.
            // Example 2: Cancel a default action and prevent it from bubbling up by returning false.
            PerformJQueryTest(@"
                $(""a"").live(""click"", function() { return false; })
            ");
        }

        [TestMethod]
        public void live_Example3()
        {
            // Test Block 239.
            // Entry live: Attach an event handler for all elements which match the current selector, now and in the future.
            // Example 3: Cancel only the default action by using the preventDefault method.
            PerformJQueryTest(@"
                $(""a"").live(""click"", function(event){
                  event.preventDefault();
                });
            ", "preventDefault");
        }

        [TestMethod]
        public void live_Example4()
        {
            // Test Block 240.
            // Entry live: Attach an event handler for all elements which match the current selector, now and in the future.
            // Example 4: Bind custom events with .live().
            PerformJQueryTest(@"
                $(""p"").live(""myCustomEvent"", function(e, myName, myValue) {
                  $(this).text(""Hi there!"");
                  $(""span"").stop().css(""opacity"", 1)
                           .text(""myName = "" + myName)
                           .fadeIn(30).fadeOut(1000);
                });
                $(""button"").click(function () {
                  $(""p"").trigger(""myCustomEvent"");
                });
            ");
        }

        [TestMethod]
        public void live_Example5()
        {
            // Test Block 241.
            // Entry live: Attach an event handler for all elements which match the current selector, now and in the future.
            // Example 5: Use a map to bind multiple live event handlers. Note that .live() calls the click, mouseover, and mouseout event handlers for all paragraphs--even new ones.
            PerformJQueryTest(@"
                $(""p"").live({
                  click: function() {
                    $(this).after(""<p>Another paragraph!</p>"");
                  },
                  mouseover: function() {
                    $(this).addClass(""over"");
                  },
                  mouseout: function() {
                    $(this).removeClass(""over"");
                  }
                });
            ");
        }

        [TestMethod]
        public void triggerHandler_Example1()
        {
            // Test Block 242.
            // Entry triggerHandler: Execute all handlers attached to an element for an event.
            // Example 1: If you called .triggerHandler() on a focus event - the browser's default focus action would not be triggered, only the event handlers bound to the focus event.
            PerformJQueryTest(@"
                $(""#old"").click(function(){
                $(""input"").trigger(""focus"");
                });
                $(""#new"").click(function(){
                $(""input"").triggerHandler(""focus"");
                });
                $(""input"").focus(function(){
                $(""<span>Focused!</span>"").appendTo(""body"").fadeOut(1000);
                });
            ");
        }

        [TestMethod]
        public void trigger_Example1()
        {
            // Test Block 243.
            // Entry trigger: Execute all handlers and behaviors attached to the matched elements for the given event type.
            // Example 1: Clicks to button #2 also trigger a click for button #1.
            PerformJQueryTest(@"
                $(""button:first"").click(function () {
                update($(""span:first""));
                });
                $(""button:last"").click(function () {
                $(""button:first"").trigger('click');
                update($(""span:last""));
                });
                function update(j) {
                var n = parseInt(j.text(), 10);
                j.text(n + 1);
                }
            ");
        }

        [TestMethod]
        public void trigger_Example2()
        {
            // Test Block 244.
            // Entry trigger: Execute all handlers and behaviors attached to the matched elements for the given event type.
            // Example 2: To submit the first form without using the submit() function, try:
            PerformJQueryTest(@"
                $(""form:first"").trigger(""submit"")
            ");
        }

        [TestMethod]
        public void trigger_Example3()
        {
            // Test Block 245.
            // Entry trigger: Execute all handlers and behaviors attached to the matched elements for the given event type.
            // Example 3: To submit the first form without using the submit() function, try:
            PerformJQueryTest(@"
                var event = jQuery.Event(""submit"");
                $(""form:first"").trigger(event);
                if ( event.isDefaultPrevented() ) {
                // Perform an action...
                }
            ");
        }

        [TestMethod]
        public void trigger_Example4()
        {
            // Test Block 246.
            // Entry trigger: Execute all handlers and behaviors attached to the matched elements for the given event type.
            // Example 4: To pass arbitrary data to an event:
            PerformJQueryTest(@"
                $(""p"").click( function (event, a, b) {
                // when a normal click fires, a and b are undefined
                // for a trigger like below a refers to ""foo"" and b refers to ""bar""
                } ).trigger(""click"", [""foo"", ""bar""]);
            ");
        }

        [TestMethod]
        public void trigger_Example5()
        {
            // Test Block 247.
            // Entry trigger: Execute all handlers and behaviors attached to the matched elements for the given event type.
            // Example 5: To pass arbitrary data through an event object:
            PerformJQueryTest(@"
                var event = jQuery.Event(""logged"");
                event.user = ""foo"";
                event.pass = ""bar"";
                $(""body"").trigger(event);
            ", "user", "pass");
        }

        [TestMethod]
        public void trigger_Example6()
        {
            // Test Block 248.
            // Entry trigger: Execute all handlers and behaviors attached to the matched elements for the given event type.
            // Example 6: Alternative way to pass data through an event object:
            PerformJQueryTest(@"
                $(""body"").trigger({
                type:""logged"",
                user:""foo"",
                pass:""bar""
                });
            ");
        }

        [TestMethod]
        public void ajaxComplete_Example1()
        {
            // Test Block 249.
            // Entry ajaxComplete: Register a handler to be called when Ajax requests complete. This is an Ajax Event.
            // Example 1: Show a message when an Ajax request completes.
            PerformJQueryTest(@"
                $(""#msg"").ajaxComplete(function(event,request, settings){
                   $(this).append(""<li>Request Complete.</li>"");
                 });
            ");
        }

        [TestMethod]
        public void one_Example1()
        {
            // Test Block 250.
            // Entry one: Attach a handler to an event for the elements. The handler is executed at most once per element.
            // Example 1: Tie a one-time click to each div.
            PerformJQueryTest(@"
                var n = 0;
                $(""div"").one(""click"", function() {
                  var index = $(""div"").index(this);
                  $(this).css({ 
                    borderStyle:""inset"",
                    cursor:""auto""
                  });
                  $(""p"").text(""Div at index #"" + index + "" clicked."" +
                      ""  That's "" + ++n + "" total clicks."");
                });
            ");
        }

        [TestMethod]
        public void one_Example2()
        {
            // Test Block 251.
            // Entry one: Attach a handler to an event for the elements. The handler is executed at most once per element.
            // Example 2: To display the text of all paragraphs in an alert box the first time each of them is clicked:
            PerformJQueryTest(@"
                $(""p"").one(""click"", function(){
                alert( $(this).text() );
                });
            ");
        }

        [TestMethod]
        public void serializeArray_Example1()
        {
            // Test Block 252.
            // Entry serializeArray: Encode a set of form elements as an array of names and values.
            // Example 1: Get the values from a form, iterate through them, and append them to a results display.
            PerformJQueryTest(@"
                    function showValues() {
                      var fields = $("":input"").serializeArray();
                      $(""#results"").empty();
                      jQuery.each(fields, function(i, field){
                        $(""#results"").append(field.value + "" "");
                      });
                    }
                    $("":checkbox, :radio"").click(showValues);
                    $(""select"").change(showValues);
                    showValues();
            ", "value");
        }

        [TestMethod]
        public void serialize_Example1()
        {
            // Test Block 253.
            // Entry serialize: Encode a set of form elements as a string for submission.
            // Example 1: Serialize a form to a query string, that could be sent to a server in an Ajax request.
            PerformJQueryTest(@"
                    function showValues() {
                      var str = $(""form"").serialize();
                      $(""#results"").text(str);
                    }
                    $("":checkbox, :radio"").click(showValues);
                    $(""select"").change(showValues);
                    showValues();
            ");
        }

        [TestMethod]
        public void jQuery_ajaxSetup_Example1()
        {
            // Test Block 254.
            // Entry jQuery.ajaxSetup: Set default values for future Ajax requests.
            // Example 1: Sets the defaults for Ajax requests to the url "/xmlhttp/", disables global handlers and uses POST instead of GET. The following Ajax requests then sends some data without having to set anything else.
            PerformJQueryTest(@"
                $.ajaxSetup({
                   url: ""/xmlhttp/"",
                   global: false,
                   type: ""POST""
                 });
                 $.ajax({ data: myData });
            ", "myData");
        }

        [TestMethod]
        public void ajaxSuccess_Example1()
        {
            // Test Block 255.
            // Entry ajaxSuccess: Attach a function to be executed whenever an Ajax request completes successfully. This is an Ajax Event.
            // Example 1: Show a message when an Ajax request completes successfully.
            PerformJQueryTest(@"
                $(""#msg"").ajaxSuccess(function(evt, request, settings){
                      $(this).append(""<li>Successful Request!</li>"");
                      });
            ");
        }

        [TestMethod]
        public void ajaxStop_Example1()
        {
            // Test Block 256.
            // Entry ajaxStop: Register a handler to be called when all Ajax requests have completed. This is an Ajax Event.
            // Example 1: Hide a loading message after all the Ajax requests have stopped.
            PerformJQueryTest(@"
                $(""#loading"").ajaxStop(function(){
                      $(this).hide();
                      });
            ");
        }

        [TestMethod]
        public void ajaxStart_Example1()
        {
            // Test Block 257.
            // Entry ajaxStart: Register a handler to be called when the first Ajax request begins. This is an Ajax Event.
            // Example 1: Show a loading message whenever an Ajax request starts (and none is already active).
            PerformJQueryTest(@"
                $(""#loading"").ajaxStart(function(){
                   $(this).show();
                 });
            ");
        }

        [TestMethod]
        public void ajaxSend_Example1()
        {
            // Test Block 258.
            // Entry ajaxSend: Attach a function to be executed before an Ajax request is sent. This is an Ajax Event.
            // Example 1: Show a message before an Ajax request is sent.
            PerformJQueryTest(@"
                $(""#msg"").ajaxSend(function(evt, request, settings){
                        $(this).append(""<li>Starting request at "" + settings.url + ""</li>"");
                      });
            ", "url");
        }

        [TestMethod]
        public void ajaxError_Example1()
        {
            // Test Block 259.
            // Entry ajaxError: Register a handler to be called when Ajax requests complete with an error. This is an Ajax Event.
            // Example 1: Show a message when an Ajax request fails.
            PerformJQueryTest(@"
                $(""#msg"").ajaxError(function(event, request, settings){
                  $(this).append(""<li>Error requesting page "" + settings.url + ""</li>"");
                });
            ", "url");
        }

        [TestMethod]
        public void unbind_Example1()
        {
            // Test Block 260.
            // Entry unbind: Remove a previously-attached event handler from the elements.
            // Example 1: Can bind and unbind events to the colored button.
            PerformJQueryTest(@"
                function aClick() {
                $(""div"").show().fadeOut(""slow"");
                }
                $(""#bind"").click(function () {
                // could use .bind('click', aClick) instead but for variety...
                $(""#theone"").click(aClick)
                  .text(""Can Click!"");
                });
                $(""#unbind"").click(function () {
                $(""#theone"").unbind('click', aClick)
                  .text(""Does nothing..."");
                });
            ");
        }

        [TestMethod]
        public void unbind_Example2()
        {
            // Test Block 261.
            // Entry unbind: Remove a previously-attached event handler from the elements.
            // Example 2: To unbind all events from all paragraphs, write:
            PerformJQueryTest(@"
                $(""p"").unbind()
            ");
        }

        [TestMethod]
        public void unbind_Example3()
        {
            // Test Block 262.
            // Entry unbind: Remove a previously-attached event handler from the elements.
            // Example 3: To unbind all click events from all paragraphs, write:
            PerformJQueryTest(@"
                $(""p"").unbind( ""click"" )
            ");
        }

        [TestMethod]
        public void unbind_Example4()
        {
            // Test Block 263.
            // Entry unbind: Remove a previously-attached event handler from the elements.
            // Example 4: To unbind just one previously bound handler, pass the function in as the second argument:
            PerformJQueryTest(@"
                var foo = function () {
                // code to handle some kind of event
                };
                $(""p"").bind(""click"", foo); // ... now foo will be called when paragraphs are clicked ...
                $(""p"").unbind(""click"", foo); // ... foo will no longer be called.
            ");
        }

        [TestMethod]
        public void bind_Example1()
        {
            // Test Block 264.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 1: Handle click and double-click for the paragraph.  Note: the coordinates are window relative, so in this case relative to the demo iframe.
            PerformJQueryTest(@"
                $(""p"").bind(""click"", function(event){
                var str = ""( "" + event.pageX + "", "" + event.pageY + "" )"";
                $(""span"").text(""Click happened! "" + str);
                });
                $(""p"").bind(""dblclick"", function(){
                $(""span"").text(""Double-click happened in "" + this.nodeName);
                });
                $(""p"").bind(""mouseenter mouseleave"", function(event){
                $(this).toggleClass(""over"");
                });
            ");
        }

        [TestMethod]
        public void bind_Example2()
        {
            // Test Block 265.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 2: To display each paragraph's text in an alert box whenever it is clicked:
            PerformJQueryTest(@"
                $(""p"").bind(""click"", function(){
                alert( $(this).text() );
                });
            ");
        }

        [TestMethod]
        public void bind_Example3()
        {
            // Test Block 266.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 3: You can pass some extra data before the event handler:
            PerformJQueryTest(@"
                function handler(event) {
                alert(event.data.foo);
                }
                $(""p"").bind(""click"", {foo: ""bar""}, handler)
            ");
        }

        [TestMethod]
        public void bind_Example4()
        {
            // Test Block 267.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 4: Cancel a default action and prevent it from bubbling up by returning false:
            PerformJQueryTest(@"
                $(""form"").bind(""submit"", function() { return false; })
            ");
        }

        [TestMethod]
        public void bind_Example5()
        {
            // Test Block 268.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 5: Cancel only the default action by using the .preventDefault() method.
            PerformJQueryTest(@"
                $(""form"").bind(""submit"", function(event) {
                event.preventDefault();
                });
            ");
        }

        [TestMethod]
        public void bind_Example6()
        {
            // Test Block 269.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 6: Stop an event from bubbling without preventing the default action by using the .stopPropagation() method.
            PerformJQueryTest(@"
                $(""form"").bind(""submit"", function(event) {
                  event.stopPropagation();
                });
            ");
        }

        [TestMethod]
        public void bind_Example7()
        {
            // Test Block 270.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 7: Bind custom events.
            PerformJQueryTest(@"
                $(""p"").bind(""myCustomEvent"", function(e, myName, myValue){
                $(this).text(myName + "", hi there!"");
                $(""span"").stop().css(""opacity"", 1)
                .text(""myName = "" + myName)
                .fadeIn(30).fadeOut(1000);
                });
                $(""button"").click(function () {
                $(""p"").trigger(""myCustomEvent"", [ ""John"" ]);
                });
            ");
        }

        [TestMethod]
        public void bind_Example8()
        {
            // Test Block 271.
            // Entry bind: Attach a handler to an event for the elements.
            // Example 8: Bind multiple events simultaneously.
            PerformJQueryTest(@"
                $(""div.test"").bind({
                  click: function(){
                    $(this).addClass(""active"");
                  },
                  mouseenter: function(){
                    $(this).addClass(""inside"");
                  },
                  mouseleave: function(){
                    $(this).removeClass(""inside"");
                  }
                });
            ");
        }

        [TestMethod]
        public void first_1_Example1()
        {
            // Test Block 272.
            // Entry first_1: Reduce the set of matched elements to the first in the set.
            // Example 1: Highlight the first span in a paragraph.
            PerformJQueryTest(@"
                $(""p span"").first().addClass('highlight');
            ");
        }

        [TestMethod]
        public void last_1_Example1()
        {
            // Test Block 273.
            // Entry last_1: Reduce the set of matched elements to the final one in the set.
            // Example 1: Highlight the last span in a paragraph.
            PerformJQueryTest(@"
                $(""p span"").last().addClass('highlight');
            ");
        }

        [TestMethod]
        public void slice_Example1()
        {
            // Test Block 274.
            // Entry slice: Reduce the set of matched elements to a subset specified by a range of indices.
            // Example 1: Turns divs yellow based on a random slice.
            PerformJQueryTest(@"
                    function colorEm() {
                      var $div = $(""div"");
                      var start = Math.floor(Math.random() *
                                             $div.length);
                      var end = Math.floor(Math.random() *
                                           ($div.length - start)) +
                                           start + 1;
                      if (end == $div.length) end = undefined;
                      $div.css(""background"", """");
                      if (end) 
                        $div.slice(start, end).css(""background"", ""yellow"");   
                       else
                        $div.slice(start).css(""background"", ""yellow"");
                      $(""span"").text('$(""div"").slice(' + start +
                                     (end ? ', ' + end : '') +
                                     ').css(""background"", ""yellow"");');
                    }
                    $(""button"").click(colorEm);
            ");
        }

        [TestMethod]
        public void slice_Example2()
        {
            // Test Block 275.
            // Entry slice: Reduce the set of matched elements to a subset specified by a range of indices.
            // Example 2: Selects all paragraphs, then slices the selection to include only the first element.
            PerformJQueryTest(@"
                $(""p"").slice(0, 1).wrapInner(""<b></b>"");
            ");
        }

        [TestMethod]
        public void slice_Example3()
        {
            // Test Block 276.
            // Entry slice: Reduce the set of matched elements to a subset specified by a range of indices.
            // Example 3: Selects all paragraphs, then slices the selection to include only the first and second element.
            PerformJQueryTest(@"
                $(""p"").slice(0, 2).wrapInner(""<b></b>"");
            ");
        }

        [TestMethod]
        public void slice_Example4()
        {
            // Test Block 277.
            // Entry slice: Reduce the set of matched elements to a subset specified by a range of indices.
            // Example 4: Selects all paragraphs, then slices the selection to include only the second element.
            PerformJQueryTest(@"
                $(""p"").slice(1, 2).wrapInner(""<b></b>"");
            ");
        }

        [TestMethod]
        public void slice_Example5()
        {
            // Test Block 278.
            // Entry slice: Reduce the set of matched elements to a subset specified by a range of indices.
            // Example 5: Selects all paragraphs, then slices the selection to include only the second and third element.
            PerformJQueryTest(@"
                $(""p"").slice(1).wrapInner(""<b></b>"");
            ");
        }

        [TestMethod]
        public void slice_Example6()
        {
            // Test Block 279.
            // Entry slice: Reduce the set of matched elements to a subset specified by a range of indices.
            // Example 6: Selects all paragraphs, then slices the selection to include only the third element.
            PerformJQueryTest(@"
                $(""p"").slice(-1).wrapInner(""<b></b>"");
            ");
        }

        [TestMethod]
        public void jQuery_Example1()
        {
            // Test Block 280.
            // Entry jQuery: Accepts a string containing a CSS selector which is then used to match a set of elements.
            // Example 1: Find all p elements that are children of a div element and apply a border to them.
            PerformJQueryTest(@"
                  $(""div > p"").css(""border"", ""1px solid gray"");
            ");
        }

        [TestMethod]
        public void jQuery_Example2()
        {
            // Test Block 281.
            // Entry jQuery: Accepts a string containing a CSS selector which is then used to match a set of elements.
            // Example 2: Find all inputs of type radio within the first form in the document.
            PerformJQueryTest(@"
                $(""input:radio"", document.forms[0]);
            ");
        }

        [TestMethod]
        public void jQuery_Example3()
        {
            // Test Block 282.
            // Entry jQuery: Accepts a string containing a CSS selector which is then used to match a set of elements.
            // Example 3: Find all div elements within an XML document from an Ajax response.
            PerformJQueryTest(@"
                $(""div"", xml.responseXML);
            ", "xml", "responseXML");
        }

        [TestMethod]
        public void jQuery_Example4()
        {
            // Test Block 283.
            // Entry jQuery: Accepts a string containing a CSS selector which is then used to match a set of elements.
            // Example 4: Set the background color of the page to black.
            PerformJQueryTest(@"
                $(document.body).css( ""background"", ""black"" );
            ");
        }

        [TestMethod]
        public void jQuery_Example5()
        {
            // Test Block 284.
            // Entry jQuery: Accepts a string containing a CSS selector which is then used to match a set of elements.
            // Example 5: Hide all the input elements within a form.
            PerformJQueryTest(@"
                $(myForm.elements).hide()
            ", "myForm", "elements");
        }

        [TestMethod]
        public void jQuery_1_Example1()
        {
            // Test Block 285.
            // Entry jQuery_1: Creates DOM elements on the fly from the provided string of raw HTML.
            // Example 1: Create a div element (and all of its contents) dynamically and append it to the body element. Internally, an element is created and its innerHTML property set to the given markup.
            PerformJQueryTest(@"
                $(""<div><p>Hello</p></div>"").appendTo(""body"")
            ");
        }

        [TestMethod]
        public void jQuery_1_Example2()
        {
            // Test Block 286.
            // Entry jQuery_1: Creates DOM elements on the fly from the provided string of raw HTML.
            // Example 2: Create some DOM elements.
            PerformJQueryTest(@"
                $(""<div/>"", {
                  ""class"": ""test"",
                  text: ""Click me!"",
                  click: function(){
                    $(this).toggleClass(""test"");
                  }
                }).appendTo(""body"");
            ");
        }

        [TestMethod]
        public void jQuery_2_Example1()
        {
            // Test Block 287.
            // Entry jQuery_2: Binds a function to be executed when the DOM has finished loading.
            // Example 1: Execute the function when the DOM is ready to be used.
            PerformJQueryTest(@"
                $(function(){
                   // Document is ready
                 });
            ");
        }

        [TestMethod]
        public void jQuery_2_Example2()
        {
            // Test Block 288.
            // Entry jQuery_2: Binds a function to be executed when the DOM has finished loading.
            // Example 2: Use both the shortcut for $(document).ready() and the argument to write failsafe jQuery code using the $ alias, without relying on the global alias.
            PerformJQueryTest(@"
                jQuery(function($) {
                    // Your code using failsafe $ alias here...
                  });
            ");
        }

        [TestMethod]
        public void stop_Example1()
        {
            // Test Block 289.
            // Entry stop: Stop the currently-running animation on the matched elements.
            // Example 1: Click the Go button once to start the animation, then click the STOP button to stop it where it's currently positioned.  Another option is to click several buttons to queue them up and see that stop just kills the currently playing one.
            PerformJQueryTest(@"
                /* Start animation */
                $(""#go"").click(function(){
                $("".block"").animate({left: '+=100px'}, 2000);
                });
                /* Stop animation when button is clicked */
                $(""#stop"").click(function(){
                $("".block"").stop();
                });
                /* Start animation in the opposite direction */
                $(""#back"").click(function(){
                $("".block"").animate({left: '-=100px'}, 2000);
                });
            ");
        }

        [TestMethod]
        public void stop_Example2()
        {
            // Test Block 290.
            // Entry stop: Stop the currently-running animation on the matched elements.
            // Example 2: Click the slideToggle button to start the animation, then click again before the animation is completed. The animation will toggle the other direction from the saved starting point.
            PerformJQueryTest(@"
                var $block = $('.block');
                /* Toggle a sliding animation animation */
                $('#toggle').on('click', function() {
                    $block.stop().slideToggle(1000);
                });
            ");
        }

        [TestMethod]
        public void end_Example1()
        {
            // Test Block 291.
            // Entry end: End the most recent filtering operation in the current chain and return the set of matched elements to its previous state.
            // Example 1: Selects all paragraphs, finds span elements inside these, and reverts the selection back to the paragraphs.
            PerformJQueryTest(@"
                    jQuery.fn.showTags = function (n) {
                      var tags = this.map(function () { 
                                              return this.tagName; 
                                            })
                                        .get().join("", "");
                      $(""b:eq("" + n + "")"").text(tags);
                      return this;
                    };
                    $(""p"").showTags(0)
                          .find(""span"")
                          .showTags(1)
                          .css(""background"", ""yellow"")
                          .end()
                          .showTags(2)
                          .css(""font-style"", ""italic"");
            ", "showTags");
        }

        [TestMethod]
        public void end_Example2()
        {
            // Test Block 292.
            // Entry end: End the most recent filtering operation in the current chain and return the set of matched elements to its previous state.
            // Example 2: Selects all paragraphs, finds span elements inside these, and reverts the selection back to the paragraphs.
            PerformJQueryTest(@"
                $(""p"").find(""span"").end().css(""border"", ""2px red solid"");
            ");
        }

        [TestMethod]
        public void andSelf_Example1()
        {
            // Test Block 293.
            // Entry andSelf: Add the previous set of elements on the stack to the current set.
            // Example 1: Find all divs, and all the paragraphs inside of them, and give them both class names.  Notice the div doesn't have the yellow background color since it didn't use .andSelf().
            PerformJQueryTest(@"
                    $(""div"").find(""p"").andSelf().addClass(""border"");
                    $(""div"").find(""p"").addClass(""background"");
            ");
        }

        [TestMethod]
        public void siblings_Example1()
        {
            // Test Block 294.
            // Entry siblings: Get the siblings of each element in the set of matched elements, optionally filtered by a selector.
            // Example 1: Find the unique siblings of all yellow li elements in the 3 lists (including other yellow li elements if appropriate).
            PerformJQueryTest(@"
                    var len = $("".hilite"").siblings()
                                          .css(""color"", ""red"")
                                          .length;
                    $(""b"").text(len);
            ");
        }

        [TestMethod]
        public void siblings_Example2()
        {
            // Test Block 295.
            // Entry siblings: Get the siblings of each element in the set of matched elements, optionally filtered by a selector.
            // Example 2: Find all siblings with a class "selected" of each div.
            PerformJQueryTest(@"
                $(""p"").siblings("".selected"").css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void animate_Example1()
        {
            // Test Block 296.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 1: Click the button to animate the div with a number of different properties.
            PerformJQueryTest(@"
                /* Using multiple unit types within one animation. */
                $(""#go"").click(function(){
                  $(""#block"").animate({
                    width: ""70%"",
                    opacity: 0.4,
                    marginLeft: ""0.6in"",
                    fontSize: ""3em"",
                    borderWidth: ""10px""
                  }, 1500 );
                });
            ");
        }

        [TestMethod]
        public void animate_Example2()
        {
            // Test Block 297.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 2: Animates a div's left property with a relative value. Click several times on the buttons to see the relative animations queued up.
            PerformJQueryTest(@"
                $(""#right"").click(function(){
                  $("".block"").animate({""left"": ""+=50px""}, ""slow"");
                });
                $(""#left"").click(function(){
                  $("".block"").animate({""left"": ""-=50px""}, ""slow"");
                });
            ");
        }

        [TestMethod]
        public void animate_Example3()
        {
            // Test Block 298.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 3: The first button shows how an unqueued animation works.  It expands the div out to 90% width while the font-size is increasing. Once the font-size change is complete, the border animation will begin.    The second button starts a traditional chained animation, where each animation will start once the previous animation on the element has completed.
            PerformJQueryTest(@"
                $( ""#go1"" ).click(function(){
                  $( ""#block1"" ).animate( { width: ""90%"" }, { queue: false, duration: 3000 })
                     .animate({ fontSize: ""24px"" }, 1500 )
                     .animate({ borderRightWidth: ""15px"" }, 1500 );
                });
                $( ""#go2"" ).click(function(){
                  $( ""#block2"" ).animate({ width: ""90%"" }, 1000 )
                     .animate({ fontSize: ""24px"" }, 1000 )
                     .animate({ borderLeftWidth: ""15px"" }, 1000 );
                });
                $( ""#go3"" ).click(function(){
                  $( ""#go1"" ).add( ""#go2"" ).click();
                });
                $( ""#go4"" ).click(function(){
                  $( ""div"" ).css({ width: """", fontSize: """", borderWidth: """" });
                });
            ");
        }

        [TestMethod]
        public void animate_Example4()
        {
            // Test Block 299.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 4: Animates the first div's left property and synchronizes the remaining divs, using the step function to set their left properties at each stage of the animation. 
            PerformJQueryTest(@"
                $( ""#go"" ).click(function(){
                  $( "".block:first"" ).animate({
                    left: 100
                  }, {
                    duration: 1000,
                    step: function( now, fx ){
                      $( "".block:gt(0)"" ).css( ""left"", now );
                    }
                  });
                });
            ");
        }

        [TestMethod]
        public void animate_Example5()
        {
            // Test Block 300.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 5: Animates all paragraphs to toggle both height and opacity, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  ""height"": ""toggle"", ""opacity"": ""toggle""
                }, ""slow"" );
            ");
        }

        [TestMethod]
        public void animate_Example6()
        {
            // Test Block 301.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 6: Animates all paragraph to a left style of 50 and opacity of 1 (opaque, visible), completing the animation within 500 milliseconds.
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  ""left"": ""50"", ""opacity"": 1
                }, 500 );
            ");
        }

        [TestMethod]
        public void animate_Example7()
        {
            // Test Block 302.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 7: An example of using an 'easing' function to provide a different style of animation. This will only work if you have a plugin that provides this easing function.  Note, this code will do nothing unless the paragraph element is hidden.
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  ""opacity"": ""show""
                }, ""slow"", ""easein"" );
            ");
        }

        [TestMethod]
        public void animate_Example8()
        {
            // Test Block 303.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 8: Animates all paragraphs to toggle both height and opacity, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  ""height"": ""toggle"", ""opacity"": ""toggle""
                }, { duration: ""slow"" });
            ");
        }

        [TestMethod]
        public void animate_Example9()
        {
            // Test Block 304.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 9: Animates all paragraph to a left style of 50 and opacity of 1 (opaque, visible), completing the animation within 500 milliseconds.  It also will do it outside the queue, meaning it will automatically start without waiting for its turn.
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  left: ""50px"", opacity: 1
                }, { duration: 500, queue: false });
            ");
        }

        [TestMethod]
        public void animate_Example10()
        {
            // Test Block 305.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 10: An example of using an 'easing' function to provide a different style of animation. This will only work if you have a plugin that provides this easing function.
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  ""opacity"": ""show""
                }, { ""duration"": ""slow"", ""easing"": ""easein"" });
            ");
        }

        [TestMethod]
        public void animate_Example11()
        {
            // Test Block 306.
            // Entry animate: Perform a custom animation of a set of CSS properties.
            // Example 11: An example of using a callback function.  The first argument is an array of CSS properties, the second specifies that the animation should take 1000 milliseconds to complete, the third states the easing type, and the fourth argument is an anonymous callback function. 
            PerformJQueryTest(@"
                $( ""p"" ).animate({
                  height:200, width:400, opacity: .5
                }, 1000, ""linear"", function(){ alert(""all done""); });
            ");
        }

        [TestMethod]
        public void prevAll_Example1()
        {
            // Test Block 307.
            // Entry prevAll: Get all preceding siblings of each element in the set of matched elements, optionally filtered by a selector.
            // Example 1: Locate all the divs preceding the last div and give them a class.
            PerformJQueryTest(@"
                $(""div:last"").prevAll().addClass(""before"");
            ");
        }

        [TestMethod]
        public void prev_Example1()
        {
            // Test Block 308.
            // Entry prev: Get the immediately preceding sibling of each element in the set of matched elements, optionally filtered by a selector.
            // Example 1: Find the very previous sibling of each div.
            PerformJQueryTest(@"
                    var $curr = $(""#start"");
                    $curr.css(""background"", ""#f99"");
                    $(""button"").click(function () {
                      $curr = $curr.prev();
                      $(""div"").css(""background"", """");
                      $curr.css(""background"", ""#f99"");
                    });
            ");
        }

        [TestMethod]
        public void prev_Example2()
        {
            // Test Block 309.
            // Entry prev: Get the immediately preceding sibling of each element in the set of matched elements, optionally filtered by a selector.
            // Example 2: For each paragraph, find the very previous sibling that has a class "selected".
            PerformJQueryTest(@"
                $(""p"").prev("".selected"").css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void fadeTo_Example1()
        {
            // Test Block 310.
            // Entry fadeTo: Adjust the opacity of the matched elements.
            // Example 1: Animates first paragraph to fade to an opacity of 0.33 (33%, about one third visible), completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                $(""p:first"").click(function () {
                $(this).fadeTo(""slow"", 0.33);
                });
            ");
        }

        [TestMethod]
        public void fadeTo_Example2()
        {
            // Test Block 311.
            // Entry fadeTo: Adjust the opacity of the matched elements.
            // Example 2: Fade div to a random opacity on each click, completing the animation within 200 milliseconds.
            PerformJQueryTest(@"
                $(""div"").click(function () {
                $(this).fadeTo(""fast"", Math.random());
                });
            ");
        }

        [TestMethod]
        public void fadeTo_Example3()
        {
            // Test Block 312.
            // Entry fadeTo: Adjust the opacity of the matched elements.
            // Example 3: Find the right answer!  The fade will take 250 milliseconds and change various styles when it completes.
            PerformJQueryTest(@"
                var getPos = function (n) {
                return (Math.floor(n) * 90) + ""px"";
                };
                $(""p"").each(function (n) {
                var r = Math.floor(Math.random() * 3);
                var tmp = $(this).text();
                $(this).text($(""p:eq("" + r + "")"").text());
                $(""p:eq("" + r + "")"").text(tmp);
                $(this).css(""left"", getPos(n));
                });
                $(""div"").each(function (n) {
                      $(this).css(""left"", getPos(n));
                    })
                .css(""cursor"", ""pointer"")
                .click(function () {
                      $(this).fadeTo(250, 0.25, function () {
                            $(this).css(""cursor"", """")
                                   .prev().css({""font-weight"": ""bolder"",
                                                ""font-style"": ""italic""});
                          });
                    });
            ");
        }

        [TestMethod]
        public void fadeOut_Example1()
        {
            // Test Block 313.
            // Entry fadeOut: Hide the matched elements by fading them to transparent.
            // Example 1: Animates all paragraphs to fade out, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                  $(""p"").click(function () {
                  $(""p"").fadeOut(""slow"");
                  });
            ");
        }

        [TestMethod]
        public void fadeOut_Example2()
        {
            // Test Block 314.
            // Entry fadeOut: Hide the matched elements by fading them to transparent.
            // Example 2: Fades out spans in one section that you click on.
            PerformJQueryTest(@"
                  $(""span"").click(function () {
                  $(this).fadeOut(1000, function () {
                  $(""div"").text(""'"" + $(this).text() + ""' has faded!"");
                  $(this).remove();
                  });
                  });
                  $(""span"").hover(function () {
                  $(this).addClass(""hilite"");
                  }, function () {
                  $(this).removeClass(""hilite"");
                  });
            ");
        }

        [TestMethod]
        public void fadeOut_Example3()
        {
            // Test Block 315.
            // Entry fadeOut: Hide the matched elements by fading them to transparent.
            // Example 3: Fades out two divs, one with a "linear" easing and one with the default, "swing," easing.
            PerformJQueryTest(@"
                $(""#btn1"").click(function() {
                  function complete() {
                    $(""<div/>"").text(this.id).appendTo(""#log"");
                  }
                  $(""#box1"").fadeOut(1600, ""linear"", complete);
                  $(""#box2"").fadeOut(1600, complete);
                });
                $(""#btn2"").click(function() {
                  $(""div"").show();
                  $(""#log"").empty();
                });
            ");
        }

        [TestMethod]
        public void parents_Example1()
        {
            // Test Block 316.
            // Entry parents: Get the ancestors of each element in the current set of matched elements, optionally filtered by a selector.
            // Example 1: Find all parent elements of each b.
            PerformJQueryTest(@"
                var parentEls = $(""b"").parents()
                            .map(function () { 
                                  return this.tagName; 
                                })
                            .get().join("", "");
                $(""b"").append(""<strong>"" + parentEls + ""</strong>"");
            ");
        }

        [TestMethod]
        public void parents_Example2()
        {
            // Test Block 317.
            // Entry parents: Get the ancestors of each element in the current set of matched elements, optionally filtered by a selector.
            // Example 2: Click to find all unique div parent elements of each span.
            PerformJQueryTest(@"
                function showParents() {
                  $(""div"").css(""border-color"", ""white"");
                  var len = $(""span.selected"")
                                   .parents(""div"")
                                   .css(""border"", ""2px red solid"")
                                   .length;
                  $(""b"").text(""Unique div parents: "" + len);
                }
                $(""span"").click(function () {
                  $(this).toggleClass(""selected"");
                  showParents();
                });
            ");
        }

        [TestMethod]
        public void fadeIn_Example1()
        {
            // Test Block 318.
            // Entry fadeIn: Display the matched elements by fading them to opaque.
            // Example 1: Animates hidden divs to fade in one by one, completing each animation within 600 milliseconds.
            PerformJQueryTest(@"
                      $(document.body).click(function () {
                        $(""div:hidden:first"").fadeIn(""slow"");
                      });
            ");
        }

        [TestMethod]
        public void fadeIn_Example2()
        {
            // Test Block 319.
            // Entry fadeIn: Display the matched elements by fading them to opaque.
            // Example 2: Fades a red block in over the text. Once the animation is done, it quickly fades in more text on top.
            PerformJQueryTest(@"
                        $(""a"").click(function () {
                          $(""div"").fadeIn(3000, function () {
                            $(""span"").fadeIn(100);
                          });
                          return false;
                        }); 
            ");
        }

        [TestMethod]
        public void parent_1_Example1()
        {
            // Test Block 320.
            // Entry parent_1: Get the parent of each element in the current set of matched elements, optionally filtered by a selector.
            // Example 1: Shows the parent of each element as (parent > child).  Check the View Source to see the raw html.
            PerformJQueryTest(@"
                    $(""*"", document.body).each(function () {
                      var parentTag = $(this).parent().get(0).tagName;
                      $(this).prepend(document.createTextNode(parentTag + "" > ""));
                    });
            ");
        }

        [TestMethod]
        public void parent_1_Example2()
        {
            // Test Block 321.
            // Entry parent_1: Get the parent of each element in the current set of matched elements, optionally filtered by a selector.
            // Example 2: Find the parent element of each paragraph with a class "selected".
            PerformJQueryTest(@"
                $(""p"").parent("".selected"").css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void offsetParent_Example1()
        {
            // Test Block 322.
            // Entry offsetParent: Get the closest ancestor element that is positioned.
            // Example 1: Find the offsetParent of item "A."
            PerformJQueryTest(@"
                $('li.item-a').offsetParent().css('background-color', 'red');
            ");
        }

        [TestMethod]
        public void slideToggle_Example1()
        {
            // Test Block 323.
            // Entry slideToggle: Display or hide the matched elements with a sliding motion.
            // Example 1: Animates all paragraphs to slide up or down, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      $(""p"").slideToggle(""slow"");
                    });
            ");
        }

        [TestMethod]
        public void slideToggle_Example2()
        {
            // Test Block 324.
            // Entry slideToggle: Display or hide the matched elements with a sliding motion.
            // Example 2: Animates divs between dividers with a toggle that makes some appear and some disappear.
            PerformJQueryTest(@"
                  $(""#aa"").click(function () {
                    $(""div:not(.still)"").slideToggle(""slow"", function () {
                      var n = parseInt($(""span"").text(), 10);
                      $(""span"").text(n + 1);
                    });
                  });
            ");
        }

        [TestMethod]
        public void jQuery_post_Example1()
        {
            // Test Block 325.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 1: Request the test.php page, but ignore the return results.
            PerformJQueryTest(@"
                $.post(""test.php"");
            ");
        }

        [TestMethod]
        public void jQuery_post_Example2()
        {
            // Test Block 326.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 2: Request the test.php page and send some additional data along (while still ignoring the return results).
            PerformJQueryTest(@"
                $.post(""test.php"", { name: ""John"", time: ""2pm"" } );
            ");
        }

        [TestMethod]
        public void jQuery_post_Example3()
        {
            // Test Block 327.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 3: pass arrays of data to the server (while still ignoring the return results).
            PerformJQueryTest(@"
                $.post(""test.php"", { 'choices[]': [""Jon"", ""Susan""] });
            ");
        }

        [TestMethod]
        public void jQuery_post_Example4()
        {
            // Test Block 328.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 4: send form data using ajax requests
            PerformJQueryTest(@"
                $.post(""test.php"", $(""#testform"").serialize());
            ");
        }

        [TestMethod]
        public void jQuery_post_Example5()
        {
            // Test Block 329.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 5: Alert out the results from requesting test.php (HTML or XML, depending on what was returned).
            PerformJQueryTest(@"
                $.post(""test.php"", function(data) {
                   alert(""Data Loaded: "" + data);
                 });
            ");
        }

        [TestMethod]
        public void jQuery_post_Example6()
        {
            // Test Block 330.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 6: Alert out the results from requesting test.php with an additional payload of data (HTML or XML, depending on what was returned).
            PerformJQueryTest(@"
                $.post(""test.php"", { name: ""John"", time: ""2pm"" },
                   function(data) {
                     alert(""Data Loaded: "" + data);
                   });
            ");
        }

        [TestMethod]
        public void jQuery_post_Example7()
        {
            // Test Block 331.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 7: Gets the test.php page content, store it in a XMLHttpResponse object and applies the process() JavaScript function.
            PerformJQueryTest(@"
                $.post(""test.php"", { name: ""John"", time: ""2pm"" },
                 function(data) {
                   process(data);
                 }, 
                 ""xml""
                );
            ", "process");
        }

        [TestMethod]
        public void jQuery_post_Example8()
        {
            // Test Block 332.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 8: Posts to the test.php page and gets contents which has been returned in json format (<?php echo json_encode(array("name"=>"John","time"=>"2pm")); ?>).
            PerformJQueryTest(@"
                $.post(""test.php"", { ""func"": ""getNameAndTime"" },
                 function(data){
                   console.log(data.name); // John
                   console.log(data.time); //  2pm
                 }, ""json"");
            ", "name", "time");
        }

        [TestMethod]
        public void jQuery_post_Example9()
        {
            // Test Block 333.
            // Entry jQuery.post: Load data from the server using a HTTP POST request.
            // Example 9: Post a form using ajax and put results in a div
            PerformJQueryTest(@"
                  /* attach a submit handler to the form */
                  $(""#searchForm"").submit(function(event) {
                    /* stop form from submitting normally */
                    event.preventDefault(); 
                    /* get some values from elements on the page: */
                    var $form = $( this ),
                        term = $form.find( 'input[name=""s""]' ).val(),
                        url = $form.attr( 'action' );
                    /* Send the data using post and put the results in a div */
                    $.post( url, { s: term },
                      function( data ) {
                          var content = $( data ).find( '#content' );
                          $( ""#result"" ).empty().append( content );
                      }
                    );
                  });
            ");
        }

        [TestMethod]
        public void slideUp_Example1()
        {
            // Test Block 334.
            // Entry slideUp: Hide the matched elements with a sliding motion.
            // Example 1: Animates all divs to slide up, completing the animation within 400 milliseconds.
            PerformJQueryTest(@"
                  $(document.body).click(function () {
                    if ($(""div:first"").is("":hidden"")) {
                      $(""div"").show(""slow"");
                    } else {
                      $(""div"").slideUp();
                    }
                  });
            ");
        }

        [TestMethod]
        public void slideUp_Example2()
        {
            // Test Block 335.
            // Entry slideUp: Hide the matched elements with a sliding motion.
            // Example 2: Animates the parent paragraph to slide up, completing the animation within 200 milliseconds. Once the animation is done, it displays an alert.
            PerformJQueryTest(@"
                  $(""button"").click(function () {
                    $(this).parent().slideUp(""slow"", function () {
                      $(""#msg"").text($(""button"", this).text() + "" has completed."");
                    });
                  });
            ");
        }

        [TestMethod]
        public void nextAll_Example1()
        {
            // Test Block 336.
            // Entry nextAll: Get all following siblings of each element in the set of matched elements, optionally filtered by a selector.
            // Example 1: Locate all the divs after the first and give them a class.
            PerformJQueryTest(@"
                $(""div:first"").nextAll().addClass(""after"");
            ");
        }

        [TestMethod]
        public void nextAll_Example2()
        {
            // Test Block 337.
            // Entry nextAll: Get all following siblings of each element in the set of matched elements, optionally filtered by a selector.
            // Example 2: Locate all the paragraphs after the second child in the body and give them a class.
            PerformJQueryTest(@"
                    $("":nth-child(1)"").nextAll(""p"").addClass(""after"");
            ");
        }

        [TestMethod]
        public void next_Example1()
        {
            // Test Block 338.
            // Entry next: Get the immediately following sibling of each element in the set of matched elements. If a selector is provided, it retrieves the next sibling only if it matches that selector.
            // Example 1: Find the very next sibling of each disabled button and change its text "this button is disabled".
            PerformJQueryTest(@"
                $(""button[disabled]"").next().text(""this button is disabled"");
            ");
        }

        [TestMethod]
        public void next_Example2()
        {
            // Test Block 339.
            // Entry next: Get the immediately following sibling of each element in the set of matched elements. If a selector is provided, it retrieves the next sibling only if it matches that selector.
            // Example 2: Find the very next sibling of each paragraph. Keep only the ones with a class "selected".
            PerformJQueryTest(@"
                $(""p"").next("".selected"").css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void slideDown_Example1()
        {
            // Test Block 340.
            // Entry slideDown: Display the matched elements with a sliding motion.
            // Example 1: Animates all divs to slide down and show themselves over 600 milliseconds.
            PerformJQueryTest(@"
                $(document.body).click(function () {
                if ($(""div:first"").is("":hidden"")) {
                $(""div"").slideDown(""slow"");
                } else {
                $(""div"").hide();
                }
                });
            ");
        }

        [TestMethod]
        public void slideDown_Example2()
        {
            // Test Block 341.
            // Entry slideDown: Display the matched elements with a sliding motion.
            // Example 2: Animates all inputs to slide down, completing the animation within 1000 milliseconds. Once the animation is done, the input look is changed especially if it is the middle input which gets the focus.
            PerformJQueryTest(@"
                $(""div"").click(function () {
                $(this).css({ borderStyle:""inset"", cursor:""wait"" });
                $(""input"").slideDown(1000,function(){
                $(this).css(""border"", ""2px red inset"")
                .filter("".middle"")
                 .css(""background"", ""yellow"")
                 .focus();
                $(""div"").css(""visibility"", ""hidden"");
                });
                });
            ");
        }

        [TestMethod]
        public void find_Example1()
        {
            // Test Block 342.
            // Entry find: Get the descendants of each element in the current set of matched elements, filtered by a selector, jQuery object, or element.
            // Example 1: Starts with all paragraphs and searches for descendant span elements, same as $("p span")
            PerformJQueryTest(@"
                  $(""p"").find(""span"").css('color','red');
            ");
        }

        [TestMethod]
        public void find_Example2()
        {
            // Test Block 343.
            // Entry find: Get the descendants of each element in the current set of matched elements, filtered by a selector, jQuery object, or element.
            // Example 2: A selection using a jQuery collection of all span tags. Only spans within p tags are changed to red while others are left blue.
            PerformJQueryTest(@"
                  var $spans = $('span');
                  $(""p"").find( $spans ).css('color','red');
            ");
        }

        [TestMethod]
        public void find_Example3()
        {
            // Test Block 344.
            // Entry find: Get the descendants of each element in the current set of matched elements, filtered by a selector, jQuery object, or element.
            // Example 3: Add spans around each word then add a hover and italicize words with the letter t.
            PerformJQueryTest(@"
                  var newText = $(""p"").text().split("" "").join(""</span> <span>"");
                  newText = ""<span>"" + newText + ""</span>"";
                  $(""p"").html( newText )
                    .find('span')
                    .hover(function() { 
                      $(this).addClass(""hilite""); 
                    },
                      function() { $(this).removeClass(""hilite""); 
                    })
                  .end()
                    .find("":contains('t')"")
                    .css({""font-style"":""italic"", ""font-weight"":""bolder""});
            ");
        }

        [TestMethod]
        public void jQuery_getScript_Example1()
        {
            // Test Block 345.
            // Entry jQuery.getScript: Load a JavaScript file from the server using a GET HTTP request, then execute it.
            // Example 1: Load the official jQuery Color Animation plugin dynamically and bind some color animations to occur once the new functionality is loaded.
            PerformJQueryTest(@"
                $.getScript(""http://dev.jquery.com/view/trunk/plugins/color/jquery.color.js"", function() {
                  $(""#go"").click(function(){
                    $("".block"").animate( { backgroundColor: ""pink"" }, 1000)
                      .delay(500)
                      .animate( { backgroundColor: ""blue"" }, 1000);
                  });
                });
            ");
        }

        [TestMethod]
        public void contents_Example1()
        {
            // Test Block 346.
            // Entry contents: Get the children of each element in the set of matched elements, including text and comment nodes.
            // Example 1: Find all the text nodes inside a paragraph and wrap them with a bold tag.
            PerformJQueryTest(@"
                $(""p"").contents().filter(function(){ return this.nodeType != 1; }).wrap(""<b/>"");
            ", "nodeType");
        }

        [TestMethod]
        public void contents_Example2()
        {
            // Test Block 347.
            // Entry contents: Get the children of each element in the set of matched elements, including text and comment nodes.
            // Example 2: Change the background colour of links inside of an iframe.
            PerformJQueryTest(@"
                $(""#frameDemo"").contents().find(""a"").css(""background-color"",""#BADA55"");
            ");
        }

        [TestMethod]
        public void closest_Example1()
        {
            // Test Block 348.
            // Entry closest: Get the first element that matches the selector, beginning at the current element and progressing up through the DOM tree.
            // Example 1: Show how event delegation can be done with closest. The closest list element toggles a yellow background when it or its descendent is clicked.
            PerformJQueryTest(@"
                  $( document ).bind(""click"", function( e ) {
                    $( e.target ).closest(""li"").toggleClass(""hilight"");
                  });
            ");
        }

        [TestMethod]
        public void closest_Example2()
        {
            // Test Block 349.
            // Entry closest: Get the first element that matches the selector, beginning at the current element and progressing up through the DOM tree.
            // Example 2: Pass a jQuery object to closest. The closest list element toggles a yellow background when it or its descendent is clicked.
            PerformJQueryTest(@"
                  var $listElements = $(""li"").css(""color"", ""blue"");
                  $( document ).bind(""click"", function( e ) {
                    $( e.target ).closest( $listElements ).toggleClass(""hilight"");
                  });
            ");
        }

        [TestMethod]
        public void closest_1_Example1()
        {
            // Test Block 350.
            // Entry closest_1: Gets an array of all the elements and selectors matched against the current element up through the DOM tree.
            // Example 1: Show how event delegation can be done with closest.
            PerformJQueryTest(@"
                  var close = $(""li:first"").closest([""ul"", ""body""]);
                  $.each(close, function(i){
                  $(""li"").eq(i).html( this.selector + "": "" + this.elem.nodeName );
                  });
            ");
        }

        [TestMethod]
        public void jQuery_getJSON_Example1()
        {
            // Test Block 351.
            // Entry jQuery.getJSON: Load JSON-encoded data from the server using a GET HTTP request.
            // Example 1: Loads the four most recent cat pictures from the Flickr JSONP API.
            PerformJQueryTest(@"
                $.getJSON(""http://api.flickr.com/services/feeds/photos_public.gne?jsoncallback=?"",
                  {
                    tags: ""cat"",
                    tagmode: ""any"",
                    format: ""json""
                  },
                  function(data) {
                    $.each(data.items, function(i,item){
                      $(""<img/>"").attr(""src"", item.media.m).appendTo(""#images"");
                      if ( i == 3 ) return false;
                    });
                  });
            ", "items", "media", "m", "appendTo");
        }

        [TestMethod]
        public void jQuery_getJSON_Example2()
        {
            // Test Block 352.
            // Entry jQuery.getJSON: Load JSON-encoded data from the server using a GET HTTP request.
            // Example 2: Load the JSON data from test.js and access a name from the returned JSON data.
            PerformJQueryTest(@"
                $.getJSON(""test.js"", function(json) {
                   alert(""JSON Data: "" + json.users[3].name);
                 });
            ", "name", "users");
        }

        [TestMethod]
        public void jQuery_getJSON_Example3()
        {
            // Test Block 353.
            // Entry jQuery.getJSON: Load JSON-encoded data from the server using a GET HTTP request.
            // Example 3: Load the JSON data from test.js, passing along additional data, and access a name from the returned JSON data.
            PerformJQueryTest(@"
                $.getJSON(""test.js"", { name: ""John"", time: ""2pm"" }, function(json) {
                    alert(""JSON Data: "" + json.users[3].name);
                    });
            ", "name", "users");
        }

        [TestMethod]
        public void jQuery_get_Example1()
        {
            // Test Block 354.
            // Entry jQuery.get: Load data from the server using a HTTP GET request.
            // Example 1: Request the test.php page, but ignore the return results.
            PerformJQueryTest(@"
                $.get(""test.php"");
            ");
        }

        [TestMethod]
        public void jQuery_get_Example2()
        {
            // Test Block 355.
            // Entry jQuery.get: Load data from the server using a HTTP GET request.
            // Example 2: Request the test.php page and send some additional data along (while still ignoring the return results).
            PerformJQueryTest(@"
                $.get(""test.php"", { name: ""John"", time: ""2pm"" } );
            ");
        }

        [TestMethod]
        public void jQuery_get_Example3()
        {
            // Test Block 356.
            // Entry jQuery.get: Load data from the server using a HTTP GET request.
            // Example 3: pass arrays of data to the server (while still ignoring the return results).
            PerformJQueryTest(@"
                $.get(""test.php"", { 'choices[]': [""Jon"", ""Susan""]} );
            ");
        }

        [TestMethod]
        public void jQuery_get_Example4()
        {
            // Test Block 357.
            // Entry jQuery.get: Load data from the server using a HTTP GET request.
            // Example 4: Alert out the results from requesting test.php (HTML or XML, depending on what was returned).
            PerformJQueryTest(@"
                $.get(""test.php"", function(data){
                alert(""Data Loaded: "" + data);
                });
            ");
        }

        [TestMethod]
        public void jQuery_get_Example5()
        {
            // Test Block 358.
            // Entry jQuery.get: Load data from the server using a HTTP GET request.
            // Example 5: Alert out the results from requesting test.cgi with an additional payload of data (HTML or XML, depending on what was returned).
            PerformJQueryTest(@"
                $.get(""test.cgi"", { name: ""John"", time: ""2pm"" },
                   function(data){
                     alert(""Data Loaded: "" + data);
                   });
            ");
        }

        [TestMethod]
        public void jQuery_get_Example6()
        {
            // Test Block 359.
            // Entry jQuery.get: Load data from the server using a HTTP GET request.
            // Example 6:  Gets the test.php page contents, which has been returned in json format (<?php echo json_encode(array("name"=>"John","time"=>"2pm")); ?>), and adds it to the page.
            PerformJQueryTest(@"
                $.get(""test.php"",
                   function(data){
                     $('body').append( ""Name: "" + data.name ) // John
                              .append( ""Time: "" + data.time ); //  2pm
                   }, ""json"");
            ", "name", "time");
        }

        [TestMethod]
        public void load_1_Example1()
        {
            // Test Block 360.
            // Entry load_1: Load data from the server and place the returned HTML into the matched element.
            // Example 1: Load the main page's footer navigation into an ordered list.
            PerformJQueryTest(@"
                  $(""#new-nav"").load(""/ #jq-footerNavigation li"");
            ");
        }

        [TestMethod]
        public void load_1_Example2()
        {
            // Test Block 361.
            // Entry load_1: Load data from the server and place the returned HTML into the matched element.
            // Example 2: Display a notice if the Ajax request encounters an error.
            PerformJQueryTest(@"
                $(""#success"").load(""/not-here.php"", function(response, status, xhr) {
                  if (status == ""error"") {
                    var msg = ""Sorry but there was an error: "";
                    $(""#error"").html(msg + xhr.status + "" "" + xhr.statusText);
                  }
                });
            ");
        }

        [TestMethod]
        public void load_1_Example3()
        {
            // Test Block 362.
            // Entry load_1: Load data from the server and place the returned HTML into the matched element.
            // Example 3: Load the feeds.html file into the div with the ID of feeds.
            PerformJQueryTest(@"
                $(""#feeds"").load(""feeds.html"");
            ");
        }

        [TestMethod]
        public void load_1_Example4()
        {
            // Test Block 363.
            // Entry load_1: Load data from the server and place the returned HTML into the matched element.
            // Example 4: pass arrays of data to the server.
            PerformJQueryTest(@"
                $(""#objectID"").load(""test.php"", { 'choices[]': [""Jon"", ""Susan""] } );
            ");
        }

        [TestMethod]
        public void load_1_Example5()
        {
            // Test Block 364.
            // Entry load_1: Load data from the server and place the returned HTML into the matched element.
            // Example 5: Same as above, but will POST the additional parameters to the server and a callback that is executed when the server is finished responding.
            PerformJQueryTest(@"
                $(""#feeds"").load(""feeds.php"", {limit: 25}, function(){
                alert(""The last 25 entries in the feed have been loaded"");
                });
            ");
        }

        [TestMethod]
        public void jQuery_ajax_Example1()
        {
            // Test Block 365.
            // Entry jQuery.ajax: Perform an asynchronous HTTP (Ajax) request.
            // Example 1: Save some data to the server and notify the user once it's complete.
            PerformJQueryTest(@"
                $.ajax({
                  type: ""POST"",
                  url: ""some.php"",
                  data: ""name=John&location=Boston"",
                }).done(function( msg ) {
                  alert( ""Data Saved: "" + msg );
                });
            ");
        }

        [TestMethod]
        public void jQuery_ajax_Example2()
        {
            // Test Block 366.
            // Entry jQuery.ajax: Perform an asynchronous HTTP (Ajax) request.
            // Example 2: Retrieve the latest version of an HTML page.
            PerformJQueryTest(@"
                $.ajax({
                  url: ""test.html"",
                  cache: false,
                  success: function(html){
                    $(""#results"").append(html);
                  }
                });
            ");
        }

        [TestMethod]
        public void jQuery_ajax_Example3()
        {
            // Test Block 367.
            // Entry jQuery.ajax: Perform an asynchronous HTTP (Ajax) request.
            // Example 3: Send an xml document as data to the server. By setting the processData      option to false, the automatic conversion of data to strings is prevented.
            PerformJQueryTest(@"
                var xmlDocument = [create xml document];
                var xmlRequest = $.ajax({
                  url: ""page.php"",
                  processData: false,
                  data: xmlDocument
                });
                xmlRequest.done(handleResponse);
            ", "create", "xml", "handleResponse");
        }

        [TestMethod]
        public void jQuery_ajax_Example4()
        {
            // Test Block 368.
            // Entry jQuery.ajax: Perform an asynchronous HTTP (Ajax) request.
            // Example 4: Send an id as data to the server, save some data to the server, and notify the user once it's complete. If the request fails, alert the user.
            PerformJQueryTest(@"
                var menuId = $(""ul.nav"").first().attr(""id"");
                var request = $.ajax({
                  url: ""script.php"",
                  type: ""POST"",
                  data: {id : menuId},
                  dataType: ""html""
                });
                request.done(function(msg) {
                  $(""#log"").html( msg );
                });
                request.fail(function(jqXHR, textStatus) {
                  alert( ""Request failed: "" + textStatus );
                });
            ");
        }

        [TestMethod]
        public void jQuery_ajax_Example5()
        {
            // Test Block 369.
            // Entry jQuery.ajax: Perform an asynchronous HTTP (Ajax) request.
            // Example 5: Load and execute a JavaScript file.
            PerformJQueryTest(@"
                $.ajax({
                  type: ""GET"",
                  url: ""test.js"",
                  dataType: ""script""
                });
            ");
        }

        [TestMethod]
        public void length_Example1()
        {
            // Test Block 370.
            // Entry length: The number of elements in the jQuery object.
            // Example 1: Count the divs.  Click to add more.
            PerformJQueryTest(@"
                $(document.body).click(function () {
                      $(document.body).append($(""<div>""));
                      var n = $(""div"").length;
                      $(""span"").text(""There are "" + n + "" divs."" +
                                     ""Click to add more."");
                    }).trigger('click'); // trigger the click to start
            ");
        }

        [TestMethod]
        public void children_Example1()
        {
            // Test Block 371.
            // Entry children: Get the children of each element in the set of matched elements, optionally filtered by a selector.
            // Example 1: Find all children of the clicked element.
            PerformJQueryTest(@"
                    $(""#container"").click(function (e) {
                      $(""*"").removeClass(""hilite"");
                      var $kids = $(e.target).children();
                      var len = $kids.addClass(""hilite"").length;
                      $(""#results span:first"").text(len);
                      $(""#results span:last"").text(e.target.tagName);
                      e.preventDefault();
                      return false;
                    });
            ");
        }

        [TestMethod]
        public void children_Example2()
        {
            // Test Block 372.
            // Entry children: Get the children of each element in the set of matched elements, optionally filtered by a selector.
            // Example 2: Find all children of each div.
            PerformJQueryTest(@"
                $(""div"").children().css(""border-bottom"", ""3px double red"");
            ");
        }

        [TestMethod]
        public void children_Example3()
        {
            // Test Block 373.
            // Entry children: Get the children of each element in the set of matched elements, optionally filtered by a selector.
            // Example 3: Find all children with a class "selected" of each div.
            PerformJQueryTest(@"
                $(""div"").children("".selected"").css(""color"", ""blue"");
            ");
        }

        [TestMethod]
        public void add_Example1()
        {
            // Test Block 374.
            // Entry add: Add elements to the set of matched elements.
            // Example 1: Finds all divs and makes a border.  Then adds all paragraphs to the jQuery object to set their backgrounds yellow.
            PerformJQueryTest(@"
                $(""div"").css(""border"", ""2px solid red"")
                        .add(""p"")
                        .css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void add_Example2()
        {
            // Test Block 375.
            // Entry add: Add elements to the set of matched elements.
            // Example 2: Adds more elements, matched by the given expression, to the set of matched elements.
            PerformJQueryTest(@"
                $(""p"").add(""span"").css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void add_Example3()
        {
            // Test Block 376.
            // Entry add: Add elements to the set of matched elements.
            // Example 3: Adds more elements, created on the fly, to the set of matched elements.
            PerformJQueryTest(@"
                $(""p"").clone().add(""<span>Again</span>"").appendTo(document.body);
            ");
        }

        [TestMethod]
        public void add_Example4()
        {
            // Test Block 377.
            // Entry add: Add elements to the set of matched elements.
            // Example 4: Adds one or more Elements to the set of matched elements.
            PerformJQueryTest(@"
                $(""p"").add(document.getElementById(""a"")).css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void add_Example5()
        {
            // Test Block 378.
            // Entry add: Add elements to the set of matched elements.
            // Example 5: Demonstrates how to add (or push) elements to an existing collection
            PerformJQueryTest(@"
                var collection = $(""p"");
                // capture the new collection
                collection = collection.add(document.getElementById(""a""));
                collection.css(""background"", ""yellow"");
            ");
        }

        [TestMethod]
        public void context_Example1()
        {
            // Test Block 379.
            // Entry context: The DOM node context originally passed to jQuery(); if none was passed then context will likely be the document.
            // Example 1: Determine the exact context used.
            PerformJQueryTest(@"
                $(""ul"")
                  .append(""<li>"" + $(""ul"").context + ""</li>"")
                  .append(""<li>"" + $(""ul"", document.body).context.nodeName + ""</li>"");
            ");
        }

        [TestMethod]
        public void not_1_Example1()
        {
            // Test Block 380.
            // Entry not_1: Remove elements from the set of matched elements.
            // Example 1: Adds a border to divs that are not green or blue.
            PerformJQueryTest(@"
                    $(""div"").not("".green, #blueone"")
                            .css(""border-color"", ""red"");
            ");
        }

        [TestMethod]
        public void not_1_Example2()
        {
            // Test Block 381.
            // Entry not_1: Remove elements from the set of matched elements.
            // Example 2: Removes the element with the ID "selected" from the set of all paragraphs.
            PerformJQueryTest(@"
                $(""p"").not( $(""#selected"")[0] )
            ");
        }

        [TestMethod]
        public void not_1_Example3()
        {
            // Test Block 382.
            // Entry not_1: Remove elements from the set of matched elements.
            // Example 3: Removes the element with the ID "selected" from the set of all paragraphs.
            PerformJQueryTest(@"
                $(""p"").not(""#selected"")
            ");
        }

        [TestMethod]
        public void not_1_Example4()
        {
            // Test Block 383.
            // Entry not_1: Remove elements from the set of matched elements.
            // Example 4: Removes all elements that match "div p.selected" from the total set of all paragraphs.
            PerformJQueryTest(@"
                $(""p"").not($(""div p.selected""))
            ");
        }

        [TestMethod]
        public void outerWidth_Example1()
        {
            // Test Block 384.
            // Entry outerWidth: Get the current computed width for the first element in the set of matched elements, including padding and border.
            // Example 1: Get the outerWidth of a paragraph.
            PerformJQueryTest(@"
                var p = $(""p:first"");
                $(""p:last"").text( ""outerWidth:"" + p.outerWidth()+ "" , outerWidth(true):"" + p.outerWidth(true) );
            ");
        }

        [TestMethod]
        public void outerHeight_Example1()
        {
            // Test Block 385.
            // Entry outerHeight: Get the current computed height for the first element in the set of matched elements, including padding, border, and optionally margin. Returns an integer (without "px") representation of the value or null if called on an empty set of elements.
            // Example 1: Get the outerHeight of a paragraph.
            PerformJQueryTest(@"
                var p = $(""p:first"");
                $(""p:last"").text( ""outerHeight:"" + p.outerHeight() + "" , outerHeight(true):"" + p.outerHeight(true) );
            ");
        }

        [TestMethod]
        public void toggle_1_Example1()
        {
            // Test Block 386.
            // Entry toggle_1: Display or hide the matched elements.
            // Example 1: Toggles all paragraphs.
            PerformJQueryTest(@"
                $(""button"").click(function () {
                $(""p"").toggle();
                });
            ");
        }

        [TestMethod]
        public void toggle_1_Example2()
        {
            // Test Block 387.
            // Entry toggle_1: Display or hide the matched elements.
            // Example 2: Animates all paragraphs to be shown if they are hidden and hidden if they are visible, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                $(""button"").click(function () {
                $(""p"").toggle(""slow"");
                });    
            ");
        }

        [TestMethod]
        public void toggle_1_Example3()
        {
            // Test Block 388.
            // Entry toggle_1: Display or hide the matched elements.
            // Example 3: Shows all paragraphs, then hides them all, back and forth.
            PerformJQueryTest(@"
                var flip = 0;
                $(""button"").click(function () {
                $(""p"").toggle( flip++ % 2 == 0 );
                });
            ");
        }

        [TestMethod]
        public void innerWidth_Example1()
        {
            // Test Block 389.
            // Entry innerWidth: Get the current computed width for the first element in the set of matched elements, including padding but not border.
            // Example 1: Get the innerWidth of a paragraph.
            PerformJQueryTest(@"
                var p = $(""p:first"");
                $(""p:last"").text( ""innerWidth:"" + p.innerWidth() );
            ");
        }

        [TestMethod]
        public void innerHeight_Example1()
        {
            // Test Block 390.
            // Entry innerHeight: Get the current computed height for the first element in the set of matched elements, including padding but not border.
            // Example 1: Get the innerHeight of a paragraph.
            PerformJQueryTest(@"
                var p = $(""p:first"");
                $(""p:last"").text( ""innerHeight:"" + p.innerHeight() );
            ");
        }

        [TestMethod]
        public void jQuery_param_Example1()
        {
            // Test Block 391.
            // Entry jQuery.param: Create a serialized representation of an array or object, suitable for use in a URL query string or Ajax request. 
            // Example 1: Serialize a key/value object.
            PerformJQueryTest(@"
                    var params = { width:1680, height:1050 };
                    var str = jQuery.param(params);
                    $(""#results"").text(str);
            ");
        }

        [TestMethod]
        public void jQuery_param_Example2()
        {
            // Test Block 392.
            // Entry jQuery.param: Create a serialized representation of an array or object, suitable for use in a URL query string or Ajax request. 
            // Example 2: Serialize a few complex objects
            PerformJQueryTest(@"
                // <=1.3.2: 
                $.param({ a: [2,3,4] }) // ""a=2&a=3&a=4""
                // >=1.4:
                $.param({ a: [2,3,4] }) // ""a[]=2&a[]=3&a[]=4""
                // <=1.3.2: 
                $.param({ a: { b:1,c:2 }, d: [3,4,{ e:5 }] }) // ""a=[object+Object]&d=3&d=4&d=[object+Object]""
                // >=1.4: 
                $.param({ a: { b:1,c:2 }, d: [3,4,{ e:5 }] }) // ""a[b]=1&a[c]=2&d[]=3&d[]=4&d[2][e]=5""
            ");
        }

        [TestMethod]
        public void hide_Example1()
        {
            // Test Block 393.
            // Entry hide: Hide the matched elements.
            // Example 1: Hides all paragraphs then the link on click.
            PerformJQueryTest(@"
                    $(""p"").hide();
                    $(""a"").click(function ( event ) {
                      event.preventDefault();
                      $(this).hide();
                    });
            ");
        }

        [TestMethod]
        public void hide_Example2()
        {
            // Test Block 394.
            // Entry hide: Hide the matched elements.
            // Example 2: Animates all shown paragraphs to hide slowly, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      $(""p"").hide(""slow"");
                    });    
            ");
        }

        [TestMethod]
        public void hide_Example3()
        {
            // Test Block 395.
            // Entry hide: Hide the matched elements.
            // Example 3: Animates all spans (words in this case) to hide fastly, completing each animation within 200 milliseconds. Once each animation is done, it starts the next one.
            PerformJQueryTest(@"
                    $(""#hidr"").click(function () {
                      $(""span:last-child"").hide(""fast"", function () {
                        // use callee so don't have to name the function
                        $(this).prev().hide(""fast"", arguments.callee); 
                      });
                    });
                    $(""#showr"").click(function () {
                      $(""span"").show(2000);
                    });
            ");
        }

        [TestMethod]
        public void hide_Example4()
        {
            // Test Block 396.
            // Entry hide: Hide the matched elements.
            // Example 4: Hides the divs when clicked over 2 seconds, then removes the div element when its hidden.  Try clicking on more than one box at a time.
            PerformJQueryTest(@"
                    for (var i = 0; i < 5; i++) {
                      $(""<div>"").appendTo(document.body);
                    }
                    $(""div"").click(function () {
                      $(this).hide(2000, function () {
                        $(this).remove();
                      });
                    });
            ");
        }

        [TestMethod]
        public void width_Example1()
        {
            // Test Block 397.
            // Entry width: Get the current computed width for the first element in the set of matched elements.
            // Example 1: Show various widths.  Note the values are from the iframe so might be smaller than you expected.  The yellow highlight shows the iframe body.
            PerformJQueryTest(@"
                    function showWidth(ele, w) {
                      $(""div"").text(""The width for the "" + ele + 
                                    "" is "" + w + ""px."");
                    }
                    $(""#getp"").click(function () { 
                      showWidth(""paragraph"", $(""p"").width()); 
                    });
                    $(""#getd"").click(function () { 
                      showWidth(""document"", $(document).width()); 
                    });
                    $(""#getw"").click(function () { 
                      showWidth(""window"", $(window).width()); 
                    });
            ");
        }

        [TestMethod]
        public void width_1_Example1()
        {
            // Test Block 398.
            // Entry width_1: Set the CSS width of each element in the set of matched elements.
            // Example 1: To set the width of each div on click to 30px plus a color change.
            PerformJQueryTest(@"
                    $(""div"").one('click', function () {
                      $(this).width(30)
                             .css({cursor:""auto"", ""background-color"":""blue""});
                    });
            ");
        }

        [TestMethod]
        public void height_Example1()
        {
            // Test Block 399.
            // Entry height: Get the current computed height for the first element in the set of matched elements.
            // Example 1: Show various heights.  Note the values are from the iframe so might be smaller than you expected.  The yellow highlight shows the iframe body.
            PerformJQueryTest(@"
                    function showHeight(ele, h) {
                      $(""div"").text(""The height for the "" + ele + 
                                    "" is "" + h + ""px."");
                    }
                    $(""#getp"").click(function () { 
                      showHeight(""paragraph"", $(""p"").height()); 
                    });
                    $(""#getd"").click(function () { 
                      showHeight(""document"", $(document).height()); 
                    });
                    $(""#getw"").click(function () { 
                      showHeight(""window"", $(window).height()); 
                    });
            ");
        }

        [TestMethod]
        public void height_1_Example1()
        {
            // Test Block 400.
            // Entry height_1: Set the CSS height of every matched element.
            // Example 1: To set the height of each div on click to 30px plus a color change.
            PerformJQueryTest(@"
                $(""div"").one('click', function () {
                      $(this).height(30)
                             .css({cursor:""auto"", backgroundColor:""green""});
                    });
            ");
        }

        [TestMethod]
        public void show_Example1()
        {
            // Test Block 401.
            // Entry show: Display the matched elements.
            // Example 1: Animates all hidden paragraphs to show slowly, completing the animation within 600 milliseconds.
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                    $(""p"").show(""slow"");
                    });
            ");
        }

        [TestMethod]
        public void show_Example2()
        {
            // Test Block 402.
            // Entry show: Display the matched elements.
            // Example 2: Animates all hidden divs to show fastly in order, completing each animation within 200 milliseconds. Once each animation is done, it starts the next one.
            PerformJQueryTest(@"
                $(""#showr"").click(function () {
                  $(""div:eq(0)"").show(""fast"", function () {
                    /* use callee so don't have to name the function */
                    $(this).next(""div"").show(""fast"", arguments.callee);
                  });
                });
                $(""#hidr"").click(function () {
                  $(""div"").hide(2000);
                });
            ");
        }

        [TestMethod]
        public void show_Example3()
        {
            // Test Block 403.
            // Entry show: Display the matched elements.
            // Example 3: Shows all span and input elements with an animation. Once the animation is done, it changes the text.
            PerformJQueryTest(@"
                function doIt() {
                  $(""span,div"").show(""slow"");
                }
                /* can pass in function name */
                $(""button"").click(doIt);
                $(""form"").submit(function () {
                  if ($(""input"").val() == ""yes"") {
                    $(""p"").show(4000, function () {
                      $(this).text(""Ok, DONE! (now showing)"");
                    });
                  }
                  $(""span,div"").hide(""fast"");
                  /* to stop the submit */
                  return false; 
                });
            ");
        }

        [TestMethod]
        public void scrollLeft_Example1()
        {
            // Test Block 404.
            // Entry scrollLeft: Get the current horizontal position of the scroll bar for the first element in the set of matched elements.
            // Example 1: Get the scrollLeft of a paragraph.
            PerformJQueryTest(@"
                var p = $(""p:first"");
                			$(""p:last"").text( ""scrollLeft:"" + p.scrollLeft() );
            ");
        }

        [TestMethod]
        public void scrollLeft_1_Example1()
        {
            // Test Block 405.
            // Entry scrollLeft_1: Set the current horizontal position of the scroll bar for each of the set of matched elements.
            // Example 1: Set the scrollLeft of a div.
            PerformJQueryTest(@"
                $(""div.demo"").scrollLeft(300);
            ");
        }

        [TestMethod]
        public void jQuery_trim_Example1()
        {
            // Test Block 406.
            // Entry jQuery.trim: Remove the whitespace from the beginning and end of a string.
            // Example 1: Remove the two white spaces at the start and at the end of the string.
            PerformJQueryTest(@"
                  var str = ""         lots of spaces before and after         "";
                  $(""#original"").html(""Original String: '"" + str + ""'"");
                  $(""#trimmed"").html(""$.trim()'ed: '"" + $.trim(str) + ""'"");
            ");
        }

        [TestMethod]
        public void jQuery_trim_Example2()
        {
            // Test Block 407.
            // Entry jQuery.trim: Remove the whitespace from the beginning and end of a string.
            // Example 2: Remove the two white spaces at the start and at the end of the string.
            PerformJQueryTest(@"
                $.trim(""    hello, how are you?    "");
            ");
        }

        [TestMethod]
        public void jQuery_isFunction_Example1()
        {
            // Test Block 408.
            // Entry jQuery.isFunction: Determine if the argument passed is a Javascript function object. 
            // Example 1: Test a few parameter examples.
            PerformJQueryTest(@"
                    function stub() {
                    }
                    var objs = [
                          function () {},
                          { x:15, y:20 },
                          null,
                          stub,
                          ""function""
                        ];
                    jQuery.each(objs, function (i) {
                      var isFunc = jQuery.isFunction(objs[i]);
                      $(""span"").eq(i).text(isFunc);
                    });
            ");
        }

        [TestMethod]
        public void jQuery_isFunction_Example2()
        {
            // Test Block 409.
            // Entry jQuery.isFunction: Determine if the argument passed is a Javascript function object. 
            // Example 2: Finds out if the parameter is a function.
            PerformJQueryTest(@"
                $.isFunction(function(){});
            ");
        }

        [TestMethod]
        public void jQuery_isArray_Example1()
        {
            // Test Block 410.
            // Entry jQuery.isArray: Determine whether the argument is an array.
            // Example 1: Finds out if the parameter is an array.
            PerformJQueryTest(@"
                $(""b"").append( """" + $.isArray([]) );
            ");
        }

        [TestMethod]
        public void jQuery_unique_Example1()
        {
            // Test Block 411.
            // Entry jQuery.unique: Sorts an array of DOM elements, in place, with the duplicates removed. Note that this only works on arrays of DOM elements, not strings or numbers.
            // Example 1: Removes any duplicate elements from the array of divs.
            PerformJQueryTest(@"
                    var divs = $(""div"").get(); // unique() must take a native array
                    // add 3 elements of class dup too (they are divs)
                    divs = divs.concat($("".dup"").get());
                    $(""div:eq(1)"").text(""Pre-unique there are "" + divs.length + "" elements."");
                    divs = jQuery.unique(divs);
                    $(""div:eq(2)"").text(""Post-unique there are "" + divs.length + "" elements."")
                                  .css(""color"", ""red"");
            ");
        }

        [TestMethod]
        public void jQuery_merge_Example1()
        {
            // Test Block 412.
            // Entry jQuery.merge: Merge the contents of two arrays together into the first array. 
            // Example 1: Merges two arrays, altering the first argument.
            PerformJQueryTest(@"
                $.merge( [0,1,2], [2,3,4] )
            ");
        }

        [TestMethod]
        public void jQuery_merge_Example2()
        {
            // Test Block 413.
            // Entry jQuery.merge: Merge the contents of two arrays together into the first array. 
            // Example 2: Merges two arrays, altering the first argument.
            PerformJQueryTest(@"
                $.merge( [3,2,1], [4,3,2] )  
            ");
        }

        [TestMethod]
        public void jQuery_merge_Example3()
        {
            // Test Block 414.
            // Entry jQuery.merge: Merge the contents of two arrays together into the first array. 
            // Example 3: Merges two arrays, but uses a copy, so the original isn't altered.
            PerformJQueryTest(@"
                var first = ['a','b','c'];
                var second = ['d','e','f'];
                $.merge( $.merge([],first), second);
            ");
        }

        [TestMethod]
        public void jQuery_inArray_Example1()
        {
            // Test Block 415.
            // Entry jQuery.inArray: Search for a specified value within an array and return its index (or -1 if not found).
            // Example 1: Report the index of some elements in the array.
            PerformJQueryTest(@"
                var arr = [ 4, ""Pete"", 8, ""John"" ];
                var $spans = $(""span"");
                $spans.eq(0).text(jQuery.inArray(""John"", arr));
                $spans.eq(1).text(jQuery.inArray(4, arr));
                $spans.eq(2).text(jQuery.inArray(""Karl"", arr));
                $spans.eq(3).text(jQuery.inArray(""Pete"", arr, 2));
            ");
        }

        [TestMethod]
        public void jQuery_map_Example1()
        {
            // Test Block 416.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 1: A couple examples of using .map()
            PerformJQueryTest(@"
                    var arr = [ ""a"", ""b"", ""c"", ""d"", ""e"" ];
                    $(""div"").text(arr.join("", ""));
                    arr = jQuery.map(arr, function(n, i){
                      return (n.toUpperCase() + i);
                    });
                    $(""p"").text(arr.join("", ""));
                    arr = jQuery.map(arr, function (a) { 
                      return a + a; 
                    });
                    $(""span"").text(arr.join("", ""));
            ");
        }

        [TestMethod]
        public void jQuery_map_Example2()
        {
            // Test Block 417.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 2: Map the original array to a new one and add 4 to each value.
            PerformJQueryTest(@"
                $.map( [0,1,2], function(n){
                   return n + 4;
                 });
            ");
        }

        [TestMethod]
        public void jQuery_map_Example3()
        {
            // Test Block 418.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 3: Maps the original array to a new one and adds 1 to each value if it is bigger then zero, otherwise it's removed.
            PerformJQueryTest(@"
                $.map( [0,1,2], function(n){
                   return n > 0 ? n + 1 : null;
                 });
            ");
        }

        [TestMethod]
        public void jQuery_map_Example4()
        {
            // Test Block 419.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 4: Map the original array to a new one; each element is added with its original value and the value plus one.
            PerformJQueryTest(@"
                $.map( [0,1,2], function(n){
                   return [ n, n + 1 ];
                 });
            ");
        }

        [TestMethod]
        public void jQuery_map_Example5()
        {
            // Test Block 420.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 5: Map the original object to a new array and double each value.
            PerformJQueryTest(@"
                var dimensions = { width: 10, height: 15, length: 20 };
                dimensions = $.map( dimensions, function( value, index ) {
                  return value * 2;
                }); 
            ");
        }

        [TestMethod]
        public void jQuery_map_Example6()
        {
            // Test Block 421.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 6: Map an object's keys to an array.
            PerformJQueryTest(@"
                var dimensions = { width: 10, height: 15, length: 20 },
                    keys = $.map( dimensions, function( value, index ) {
                      return index;
                    }); 
            ");
        }

        [TestMethod]
        public void jQuery_map_Example7()
        {
            // Test Block 422.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 7: Maps the original array to a new one; each element is squared.
            PerformJQueryTest(@"
                $.map( [0,1,2,3], function (a) { 
                  return a * a; 
                });
            ");
        }

        [TestMethod]
        public void jQuery_map_Example8()
        {
            // Test Block 423.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 8: Remove items by returning null from the function. This removes any numbers less than 50, and the rest are decreased by 45.
            PerformJQueryTest(@"
                $.map( [0, 1, 52, 97], function (a) {
                  return (a > 50 ? a - 45 : null); 
                });
            ");
        }

        [TestMethod]
        public void jQuery_map_Example9()
        {
            // Test Block 424.
            // Entry jQuery.map: Translate all items in an array or object to new array of items.
            // Example 9: Augmenting the resulting array by returning an array inside the function.
            PerformJQueryTest(@"
                var array = [0, 1, 52, 97];
                array = $.map(array, function(a, index) {
                  return [a - 45, index];
                }); 
            ");
        }

        [TestMethod]
        public void jQuery_makeArray_Example1()
        {
            // Test Block 425.
            // Entry jQuery.makeArray: Convert an array-like object into a true JavaScript array.
            // Example 1: Turn a collection of HTMLElements into an Array of them.
            PerformJQueryTest(@"
                    var elems = document.getElementsByTagName(""div""); // returns a nodeList
                    var arr = jQuery.makeArray(elems);
                    arr.reverse(); // use an Array method on list of dom elements
                    $(arr).appendTo(document.body);
            ");
        }

        [TestMethod]
        public void jQuery_makeArray_Example2()
        {
            // Test Block 426.
            // Entry jQuery.makeArray: Convert an array-like object into a true JavaScript array.
            // Example 2: Turn a jQuery object into an array
            PerformJQueryTest(@"
                    var obj = $('li');
                    var arr = $.makeArray(obj);
            ");
        }

        [TestMethod]
        public void jQuery_grep_Example1()
        {
            // Test Block 427.
            // Entry jQuery.grep: Finds the elements of an array which satisfy a filter function. The original array is not affected.
            // Example 1: Filters the original array of numbers leaving that are not 5 and have an index greater than 4.  Then it removes all 9s.
            PerformJQueryTest(@"
                var arr = [ 1, 9, 3, 8, 6, 1, 5, 9, 4, 7, 3, 8, 6, 9, 1 ];
                $(""div"").text(arr.join("", ""));
                arr = jQuery.grep(arr, function(n, i){
                  return (n != 5 && i > 4);
                });
                $(""p"").text(arr.join("", ""));
                arr = jQuery.grep(arr, function (a) { return a != 9; });
                $(""span"").text(arr.join("", ""));
            ");
        }

        [TestMethod]
        public void jQuery_grep_Example2()
        {
            // Test Block 428.
            // Entry jQuery.grep: Finds the elements of an array which satisfy a filter function. The original array is not affected.
            // Example 2: Filter an array of numbers to include only numbers bigger then zero.
            PerformJQueryTest(@"
                $.grep( [0,1,2], function(n,i){
                   return n > 0;
                 });
            ");
        }

        [TestMethod]
        public void jQuery_grep_Example3()
        {
            // Test Block 429.
            // Entry jQuery.grep: Finds the elements of an array which satisfy a filter function. The original array is not affected.
            // Example 3: Filter an array of numbers to include numbers that are not bigger than zero.
            PerformJQueryTest(@"
                $.grep( [0,1,2], function(n,i){
                    return n > 0;
                },true);
            ");
        }

        [TestMethod]
        public void jQuery_extend_Example1()
        {
            // Test Block 430.
            // Entry jQuery.extend: Merge the contents of two or more objects together into the first object.
            // Example 1: Merge two objects, modifying the first.
            PerformJQueryTest(@"
                var object1 = {
                  apple: 0,
                  banana: {weight: 52, price: 100},
                  cherry: 97
                };
                var object2 = {
                  banana: {price: 200},
                  durian: 100
                };
                /* merge object2 into object1 */
                $.extend(object1, object2);
                var printObj = function(obj) {
                  var arr = [];
                  $.each(obj, function(key, val) {
                    var next = key + "": "";
                    next += $.isPlainObject(val) ? printObj(val) : val;
                    arr.push( next );
                  });
                  return ""{ "" +  arr.join("", "") + "" }"";
                };
                $(""#log"").append( printObj(object1) );
            ");
        }

        [TestMethod]
        public void jQuery_extend_Example2()
        {
            // Test Block 431.
            // Entry jQuery.extend: Merge the contents of two or more objects together into the first object.
            // Example 2: Merge two objects recursively, modifying the first.
            PerformJQueryTest(@"
                var object1 = {
                  apple: 0,
                  banana: {weight: 52, price: 100},
                  cherry: 97
                };
                var object2 = {
                  banana: {price: 200},
                  durian: 100
                };
                /* merge object2 into object1, recursively */
                $.extend(true, object1, object2);
                var printObj = function(obj) {
                  var arr = [];
                  $.each(obj, function(key, val) {
                    var next = key + "": "";
                    next += $.isPlainObject(val) ? printObj(val) : val;
                    arr.push( next );
                  });
                  return ""{ "" +  arr.join("", "") + "" }"";
                };
                $(""#log"").append( printObj(object1) );
            ");
        }

        [TestMethod]
        public void jQuery_extend_Example3()
        {
            // Test Block 432.
            // Entry jQuery.extend: Merge the contents of two or more objects together into the first object.
            // Example 3: Merge defaults and options, without modifying the defaults. This is a common plugin development pattern.
            PerformJQueryTest(@"
                var defaults = { validate: false, limit: 5, name: ""foo"" };
                var options = { validate: true, name: ""bar"" };
                /* merge defaults and options, without modifying defaults */
                var settings = $.extend({}, defaults, options);
                var printObj = function(obj) {
                  var arr = [];
                  $.each(obj, function(key, val) {
                    var next = key + "": "";
                    next += $.isPlainObject(val) ? printObj(val) : val;
                    arr.push( next );
                  });
                  return ""{ "" +  arr.join("", "") + "" }"";
                };
                $(""#log"").append( ""<div><b>settings -- </b>"" + printObj(settings) + ""</div>"" );
                $(""#log"").append( ""<div><b>options -- </b>"" + printObj(options) + ""</div>"" );
            ");
        }

        [TestMethod]
        public void jQuery_each_Example1()
        {
            // Test Block 433.
            // Entry jQuery.each: A generic iterator function, which can be used to seamlessly iterate over both objects and arrays. Arrays and array-like objects with a length property (such as a function's arguments object) are iterated by numeric index, from 0 to length-1. Other objects are iterated via their named properties.
            // Example 1: Iterates through the array displaying each number as both a word and numeral
            PerformJQueryTest(@"
                    var arr = [ ""one"", ""two"", ""three"", ""four"", ""five"" ];
                    var obj = { one:1, two:2, three:3, four:4, five:5 };
                    jQuery.each(arr, function() {
                      $(""#"" + this).text(""Mine is "" + this + ""."");
                       return (this != ""three""); // will stop running after ""three""
                   });
                    jQuery.each(obj, function(i, val) {
                      $(""#"" + i).append(document.createTextNode("" - "" + val));
                    });
            ");
        }

        [TestMethod]
        public void jQuery_each_Example2()
        {
            // Test Block 434.
            // Entry jQuery.each: A generic iterator function, which can be used to seamlessly iterate over both objects and arrays. Arrays and array-like objects with a length property (such as a function's arguments object) are iterated by numeric index, from 0 to length-1. Other objects are iterated via their named properties.
            // Example 2: Iterates over items in an array, accessing both the current item and its index.
            PerformJQueryTest(@"
                $.each( ['a','b','c'], function(i, l){
                   alert( ""Index #"" + i + "": "" + l );
                 });
            ");
        }

        [TestMethod]
        public void jQuery_each_Example3()
        {
            // Test Block 435.
            // Entry jQuery.each: A generic iterator function, which can be used to seamlessly iterate over both objects and arrays. Arrays and array-like objects with a length property (such as a function's arguments object) are iterated by numeric index, from 0 to length-1. Other objects are iterated via their named properties.
            // Example 3: Iterates over the properties in an object, accessing both the current item and its key.
            PerformJQueryTest(@"
                $.each( { name: ""John"", lang: ""JS"" }, function(k, v){
                   alert( ""Key: "" + k + "", Value: "" + v );
                 });
            ");
        }

        [TestMethod]
        public void jQuery_boxModel_Example1()
        {
            // Test Block 436.
            // Entry jQuery.boxModel: Deprecated in jQuery 1.3 (see jQuery.support). States if the current page, in the user's browser, is being rendered using the W3C CSS Box Model.
            // Example 1: Returns the box model for the iframe.
            PerformJQueryTest(@"
                    $(""p"").html(""The box model for this iframe is: <span>"" +
                                jQuery.boxModel + ""</span>"");
            ");
        }

        [TestMethod]
        public void jQuery_boxModel_Example2()
        {
            // Test Block 437.
            // Entry jQuery.boxModel: Deprecated in jQuery 1.3 (see jQuery.support). States if the current page, in the user's browser, is being rendered using the W3C CSS Box Model.
            // Example 2: Returns false if the page is in Quirks Mode in Internet Explorer
            PerformJQueryTest(@"
                $.boxModel
            ");
        }

        [TestMethod]
        public void scrollTop_Example1()
        {
            // Test Block 438.
            // Entry scrollTop: Get the current vertical position of the scroll bar for the first element in the set of matched elements.
            // Example 1: Get the scrollTop of a paragraph.
            PerformJQueryTest(@"
                var p = $(""p:first"");
                $(""p:last"").text( ""scrollTop:"" + p.scrollTop() );
            ");
        }

        [TestMethod]
        public void scrollTop_1_Example1()
        {
            // Test Block 439.
            // Entry scrollTop_1: Set the current vertical position of the scroll bar for each of the set of matched elements.
            // Example 1: Set the scrollTop of a div.
            PerformJQueryTest(@"
                $(""div.demo"").scrollTop(300);
            ");
        }

        [TestMethod]
        public void jQuery_support_Example1()
        {
            // Test Block 440.
            // Entry jQuery.support: A collection of properties that represent the presence of different browser features or bugs.
            // Example 1: Returns the box model for the iframe.
            PerformJQueryTest(@"
                    $(""p"").html(""This frame uses the W3C box model: <span>"" +
                                jQuery.support.boxModel + ""</span>"");
            ");
        }

        [TestMethod]
        public void jQuery_support_Example2()
        {
            // Test Block 441.
            // Entry jQuery.support: A collection of properties that represent the presence of different browser features or bugs.
            // Example 2: Returns false if the page is in QuirksMode in Internet Explorer
            PerformJQueryTest(@"
                jQuery.support.boxModel
            ");
        }

        [TestMethod]
        public void position_Example1()
        {
            // Test Block 442.
            // Entry position: Get the current coordinates of the first element in the set of matched elements, relative to the offset parent.
            // Example 1: Access the position of the second paragraph:
            PerformJQueryTest(@"
                var p = $(""p:first"");
                var position = p.position();
                $(""p:last"").text( ""left: "" + position.left + "", top: "" + position.top );
            ");
        }

        [TestMethod]
        public void offset_Example1()
        {
            // Test Block 443.
            // Entry offset: Get the current coordinates of the first element in the set of matched elements, relative to the document.
            // Example 1: Access the offset of the second paragraph:
            PerformJQueryTest(@"
                var p = $(""p:last"");
                var offset = p.offset();
                p.html( ""left: "" + offset.left + "", top: "" + offset.top );
            ");
        }

        [TestMethod]
        public void offset_Example2()
        {
            // Test Block 444.
            // Entry offset: Get the current coordinates of the first element in the set of matched elements, relative to the document.
            // Example 2: Click to see the offset.
            PerformJQueryTest(@"
                $(""*"", document.body).click(function (e) {
                  var offset = $(this).offset();
                  e.stopPropagation();
                  $(""#result"").text(this.tagName + "" coords ( "" + offset.left + "", "" +
                                                  offset.top + "" )"");
                });
            ");
        }

        [TestMethod]
        public void offset_1_Example1()
        {
            // Test Block 445.
            // Entry offset_1: Set the current coordinates of every element in the set of matched elements, relative to the document.
            // Example 1: Set the offset of the second paragraph:
            PerformJQueryTest(@"
                $(""p:last"").offset({ top: 10, left: 30 });
            ");
        }

        [TestMethod]
        public void css_Example1()
        {
            // Test Block 446.
            // Entry css: Get the value of a style property for the first element in the set of matched elements.
            // Example 1: To access the background color of a clicked div.
            PerformJQueryTest(@"
                $(""div"").click(function () {
                  var color = $(this).css(""background-color"");
                  $(""#result"").html(""That div is <span style='color:"" +
                                     color + "";'>"" + color + ""</span>."");
                });
            ");
        }

        [TestMethod]
        public void css_1_Example1()
        {
            // Test Block 447.
            // Entry css_1: Set one or more CSS properties for the  set of matched elements.
            // Example 1: To change the color of any paragraph to red on mouseover event.
            PerformJQueryTest(@"
                  $(""p"").mouseover(function () {
                    $(this).css(""color"",""red"");
                  });
            ");
        }

        [TestMethod]
        public void css_1_Example2()
        {
            // Test Block 448.
            // Entry css_1: Set one or more CSS properties for the  set of matched elements.
            // Example 2: Increase the width of #box by 200 pixels
            PerformJQueryTest(@"
                  $(""#box"").one( ""click"", function () {
                    $( this ).css( ""width"",""+=200"" );
                  });
            ");
        }

        [TestMethod]
        public void css_1_Example3()
        {
            // Test Block 449.
            // Entry css_1: Set one or more CSS properties for the  set of matched elements.
            // Example 3: To highlight a clicked word in the paragraph.
            PerformJQueryTest(@"
                  var words = $(""p:first"").text().split("" "");
                  var text = words.join(""</span> <span>"");
                  $(""p:first"").html(""<span>"" + text + ""</span>"");
                  $(""span"").click(function () {
                    $(this).css(""background-color"",""yellow"");
                  });
            ");
        }

        [TestMethod]
        public void css_1_Example4()
        {
            // Test Block 450.
            // Entry css_1: Set one or more CSS properties for the  set of matched elements.
            // Example 4: To set the color of all paragraphs to red and background to blue:
            PerformJQueryTest(@"
                  $(""p"").hover(function () {
                    $(this).css({'background-color' : 'yellow', 'font-weight' : 'bolder'});
                  }, function () {
                    var cssObj = {
                      'background-color' : '#ddd',
                      'font-weight' : '',
                      'color' : 'rgb(0,40,244)'
                    }
                    $(this).css(cssObj);
                  });
            ");
        }

        [TestMethod]
        public void css_1_Example5()
        {
            // Test Block 451.
            // Entry css_1: Set one or more CSS properties for the  set of matched elements.
            // Example 5: Increase the size of a div when you click it:
            PerformJQueryTest(@"
                  $(""div"").click(function() {
                    $(this).css({
                      width: function(index, value) {
                        return parseFloat(value) * 1.2;
                      }, 
                      height: function(index, value) {
                        return parseFloat(value) * 1.2;
                      }
                    });
                  });
            ");
        }

        [TestMethod]
        public void unwrap_Example1()
        {
            // Test Block 452.
            // Entry unwrap: Remove the parents of the set of matched elements from the DOM, leaving the matched elements in their place.
            // Example 1: Wrap/unwrap a div around each of the paragraphs.
            PerformJQueryTest(@"
                $(""button"").toggle(function(){
                  $(""p"").wrap(""<div></div>"");
                }, function(){
                  $(""p"").unwrap();
                });
            ");
        }

        [TestMethod]
        public void detach_Example1()
        {
            // Test Block 453.
            // Entry detach: Remove the set of matched elements from the DOM.
            // Example 1: Detach all paragraphs from the DOM
            PerformJQueryTest(@"
                    $(""p"").click(function(){
                      $(this).toggleClass(""off"");
                    });
                    var p;
                    $(""button"").click(function(){
                      if ( p ) {
                        p.appendTo(""body"");
                        p = null;
                      } else {
                        p = $(""p"").detach();
                      }
                    });
            ");
        }

        [TestMethod]
        public void clone_Example1()
        {
            // Test Block 454.
            // Entry clone: Create a deep copy of the set of matched elements.
            // Example 1: Clones all b elements (and selects the clones) and prepends them to all paragraphs.
            PerformJQueryTest(@"
                  $(""b"").clone().prependTo(""p"");
            ");
        }

        [TestMethod]
        public void clone_Example2()
        {
            // Test Block 455.
            // Entry clone: Create a deep copy of the set of matched elements.
            // Example 2: When using .clone() to clone a collection of elements that are not attached to the DOM, their order when inserted into the DOM is not guaranteed. However, it may be possible to preserve sort order with a workaround, as demonstrated:
            PerformJQueryTest(@"
                // sort order is not guaranteed here and may vary with browser  
                $('#copy').append($('#orig .elem')
                          .clone()
                          .children('a')
                          .prepend('foo - ')
                          .parent()
                          .clone()); 
                // correct way to approach where order is maintained
                $('#copy-correct')
                          .append($('#orig .elem')
                          .clone()
                          .children('a')
                          .prepend('bar - ')
                          .end()); 
            ");
        }

        [TestMethod]
        public void remove_Example1()
        {
            // Test Block 456.
            // Entry remove: Remove the set of matched elements from the DOM.
            // Example 1: Removes all paragraphs from the DOM
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      $(""p"").remove();
                    });
            ");
        }

        [TestMethod]
        public void remove_Example2()
        {
            // Test Block 457.
            // Entry remove: Remove the set of matched elements from the DOM.
            // Example 2: Removes all paragraphs that contain "Hello" from the DOM.  Analogous to doing $("p").filter(":contains('Hello')").remove().
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      $(""p"").remove("":contains('Hello')"");
                    });
            ");
        }

        [TestMethod]
        public void empty_1_Example1()
        {
            // Test Block 458.
            // Entry empty_1: Remove all child nodes of the set of matched elements from the DOM.
            // Example 1: Removes all child nodes (including text nodes) from all paragraphs
            PerformJQueryTest(@"
                  $(""button"").click(function () {
                    $(""p"").empty();
                  });
            ");
        }

        [TestMethod]
        public void replaceAll_Example1()
        {
            // Test Block 459.
            // Entry replaceAll: Replace each target element with the set of matched elements.
            // Example 1: Replace all the paragraphs with bold words.
            PerformJQueryTest(@"
                $(""<b>Paragraph. </b>"").replaceAll(""p""); // check replaceWith() examples
            ");
        }

        [TestMethod]
        public void replaceWith_Example1()
        {
            // Test Block 460.
            // Entry replaceWith: Replace each element in the set of matched elements with the provided new content.
            // Example 1: On click, replace the button with a div containing the same word.
            PerformJQueryTest(@"
                $(""button"").click(function () {
                  $(this).replaceWith( ""<div>"" + $(this).text() + ""</div>"" );
                });
            ");
        }

        [TestMethod]
        public void replaceWith_Example2()
        {
            // Test Block 461.
            // Entry replaceWith: Replace each element in the set of matched elements with the provided new content.
            // Example 2: Replace all paragraphs with bold words.
            PerformJQueryTest(@"
                $(""p"").replaceWith( ""<b>Paragraph. </b>"" );
            ");
        }

        [TestMethod]
        public void replaceWith_Example3()
        {
            // Test Block 462.
            // Entry replaceWith: Replace each element in the set of matched elements with the provided new content.
            // Example 3: On click, replace each paragraph with a div that is already in the DOM and selected with the $() function. Notice it doesn't clone the object but rather moves it to replace the paragraph.
            PerformJQueryTest(@"
                $(""p"").click(function () {
                  $(this).replaceWith( $(""div"") );
                });
            ");
        }

        [TestMethod]
        public void replaceWith_Example4()
        {
            // Test Block 463.
            // Entry replaceWith: Replace each element in the set of matched elements with the provided new content.
            // Example 4: On button click, replace the containing div with its child divs and append the class name of the selected element to the paragraph.
            PerformJQueryTest(@"
                $('button').bind(""click"", function() {
                  var $container = $(""div.container"").replaceWith(function() {
                    return $(this).contents();
                  });
                  $(""p"").append( $container.attr(""class"") );
                });
            ");
        }

        [TestMethod]
        public void wrapInner_Example1()
        {
            // Test Block 464.
            // Entry wrapInner: Wrap an HTML structure around the content of each element in the set of matched elements.
            // Example 1: Selects all paragraphs and wraps a bold tag around each of its contents.
            PerformJQueryTest(@"
                $(""p"").wrapInner(""<b></b>"");
            ");
        }

        [TestMethod]
        public void wrapInner_Example2()
        {
            // Test Block 465.
            // Entry wrapInner: Wrap an HTML structure around the content of each element in the set of matched elements.
            // Example 2: Wraps a newly created tree of objects around the inside of the body.
            PerformJQueryTest(@"
                $(""body"").wrapInner(""<div><div><p><em><b></b></em></p></div></div>"");
            ");
        }

        [TestMethod]
        public void wrapInner_Example3()
        {
            // Test Block 466.
            // Entry wrapInner: Wrap an HTML structure around the content of each element in the set of matched elements.
            // Example 3: Selects all paragraphs and wraps a bold tag around each of its contents.
            PerformJQueryTest(@"
                $(""p"").wrapInner(document.createElement(""b""));
            ");
        }

        [TestMethod]
        public void wrapInner_Example4()
        {
            // Test Block 467.
            // Entry wrapInner: Wrap an HTML structure around the content of each element in the set of matched elements.
            // Example 4: Selects all paragraphs and wraps a jQuery object around each of its contents.
            PerformJQueryTest(@"
                $(""p"").wrapInner($(""<span class='red'></span>""));
            ");
        }

        [TestMethod]
        public void wrapAll_Example1()
        {
            // Test Block 468.
            // Entry wrapAll: Wrap an HTML structure around all elements in the set of matched elements.
            // Example 1: Wrap a new div around all of the paragraphs.
            PerformJQueryTest(@"
                $(""p"").wrapAll(""<div></div>"");
            ");
        }

        [TestMethod]
        public void wrapAll_Example2()
        {
            // Test Block 469.
            // Entry wrapAll: Wrap an HTML structure around all elements in the set of matched elements.
            // Example 2: Wraps a newly created tree of objects around the spans.  Notice anything in between the spans gets left out like the <strong> (red text) in this example.  Even the white space between spans is left out.  Click View Source to see the original html.
            PerformJQueryTest(@"
                $(""span"").wrapAll(""<div><div><p><em><b></b></em></p></div></div>"");
            ");
        }

        [TestMethod]
        public void wrapAll_Example3()
        {
            // Test Block 470.
            // Entry wrapAll: Wrap an HTML structure around all elements in the set of matched elements.
            // Example 3: Wrap a new div around all of the paragraphs.
            PerformJQueryTest(@"
                $(""p"").wrapAll(document.createElement(""div""));
            ");
        }

        [TestMethod]
        public void wrapAll_Example4()
        {
            // Test Block 471.
            // Entry wrapAll: Wrap an HTML structure around all elements in the set of matched elements.
            // Example 4: Wrap a jQuery object double depth div around all of the paragraphs.  Notice it doesn't move the object but just clones it to wrap around its target.
            PerformJQueryTest(@"
                $(""p"").wrapAll($("".doublediv""));
            ");
        }

        [TestMethod]
        public void wrap_Example1()
        {
            // Test Block 472.
            // Entry wrap: Wrap an HTML structure around each element in the set of matched elements.
            // Example 1: Wrap a new div around all of the paragraphs.
            PerformJQueryTest(@"
                $(""p"").wrap(""<div></div>"");
            ");
        }

        [TestMethod]
        public void wrap_Example2()
        {
            // Test Block 473.
            // Entry wrap: Wrap an HTML structure around each element in the set of matched elements.
            // Example 2: Wraps a newly created tree of objects around the spans.  Notice anything in between the spans gets left out like the <strong> (red text) in this example.  Even the white space between spans is left out.  Click View Source to see the original html.
            PerformJQueryTest(@"
                $(""span"").wrap(""<div><div><p><em><b></b></em></p></div></div>"");
            ");
        }

        [TestMethod]
        public void wrap_Example3()
        {
            // Test Block 474.
            // Entry wrap: Wrap an HTML structure around each element in the set of matched elements.
            // Example 3: Wrap a new div around all of the paragraphs.
            PerformJQueryTest(@"
                $(""p"").wrap(document.createElement(""div""));
            ");
        }

        [TestMethod]
        public void wrap_Example4()
        {
            // Test Block 475.
            // Entry wrap: Wrap an HTML structure around each element in the set of matched elements.
            // Example 4: Wrap a jQuery object double depth div around all of the paragraphs.  Notice it doesn't move the object but just clones it to wrap around its target.
            PerformJQueryTest(@"
                $(""p"").wrap($("".doublediv""));
            ");
        }

        [TestMethod]
        public void insertBefore_Example1()
        {
            // Test Block 476.
            // Entry insertBefore: Insert every element in the set of matched elements before the target.
            // Example 1: Inserts all paragraphs before an element with id of "foo". Same as $("#foo").before("p")
            PerformJQueryTest(@"
                $(""p"").insertBefore(""#foo""); // check before() examples
            ");
        }

        [TestMethod]
        public void before_Example1()
        {
            // Test Block 477.
            // Entry before: Insert content, specified by the parameter, before each element in the set of matched elements.
            // Example 1: Inserts some HTML before all paragraphs.
            PerformJQueryTest(@"
                $(""p"").before(""<b>Hello</b>"");
            ");
        }

        [TestMethod]
        public void before_Example2()
        {
            // Test Block 478.
            // Entry before: Insert content, specified by the parameter, before each element in the set of matched elements.
            // Example 2: Inserts a DOM element before all paragraphs.
            PerformJQueryTest(@"
                $(""p"").before( document.createTextNode(""Hello"") );
            ");
        }

        [TestMethod]
        public void before_Example3()
        {
            // Test Block 479.
            // Entry before: Insert content, specified by the parameter, before each element in the set of matched elements.
            // Example 3: Inserts a jQuery object (similar to an Array of DOM Elements) before all paragraphs.
            PerformJQueryTest(@"
                $(""p"").before( $(""b"") );
            ");
        }

        [TestMethod]
        public void insertAfter_Example1()
        {
            // Test Block 480.
            // Entry insertAfter: Insert every element in the set of matched elements after the target.
            // Example 1: Inserts all paragraphs after an element with id of "foo". Same as $("#foo").after("p")
            PerformJQueryTest(@"
                $(""p"").insertAfter(""#foo""); // check after() examples
            ");
        }

        [TestMethod]
        public void after_Example1()
        {
            // Test Block 481.
            // Entry after: Insert content, specified by the parameter, after each element in the set of matched elements.
            // Example 1: Inserts some HTML after all paragraphs.
            PerformJQueryTest(@"
                $(""p"").after(""<b>Hello</b>"");
            ");
        }

        [TestMethod]
        public void after_Example2()
        {
            // Test Block 482.
            // Entry after: Insert content, specified by the parameter, after each element in the set of matched elements.
            // Example 2: Inserts a DOM element after all paragraphs.
            PerformJQueryTest(@"
                $(""p"").after( document.createTextNode(""Hello"") );
            ");
        }

        [TestMethod]
        public void after_Example3()
        {
            // Test Block 483.
            // Entry after: Insert content, specified by the parameter, after each element in the set of matched elements.
            // Example 3: Inserts a jQuery object (similar to an Array of DOM Elements) after all paragraphs.
            PerformJQueryTest(@"
                $(""p"").after( $(""b"") );
            ");
        }

        [TestMethod]
        public void prependTo_Example1()
        {
            // Test Block 484.
            // Entry prependTo: Insert every element in the set of matched elements to the beginning of the target.
            // Example 1: Prepends all spans to the element with the ID "foo"
            PerformJQueryTest(@"
                $(""span"").prependTo(""#foo""); // check prepend() examples
            ");
        }

        [TestMethod]
        public void prepend_Example1()
        {
            // Test Block 485.
            // Entry prepend: Insert content, specified by the parameter, to the beginning of each element in the set of matched elements.
            // Example 1: Prepends some HTML to all paragraphs.
            PerformJQueryTest(@"
                $(""p"").prepend(""<b>Hello </b>"");
            ");
        }

        [TestMethod]
        public void prepend_Example2()
        {
            // Test Block 486.
            // Entry prepend: Insert content, specified by the parameter, to the beginning of each element in the set of matched elements.
            // Example 2: Prepends a DOM Element to all paragraphs.
            PerformJQueryTest(@"
                $(""p"").prepend(document.createTextNode(""Hello ""));
            ");
        }

        [TestMethod]
        public void prepend_Example3()
        {
            // Test Block 487.
            // Entry prepend: Insert content, specified by the parameter, to the beginning of each element in the set of matched elements.
            // Example 3: Prepends a jQuery object (similar to an Array of DOM Elements) to all paragraphs.
            PerformJQueryTest(@"
                $(""p"").prepend( $(""b"") );
            ");
        }

        [TestMethod]
        public void appendTo_Example1()
        {
            // Test Block 488.
            // Entry appendTo: Insert every element in the set of matched elements to the end of the target.
            // Example 1: Appends all spans to the element with the ID "foo"
            PerformJQueryTest(@"
                $(""span"").appendTo(""#foo""); // check append() examples
            ");
        }

        [TestMethod]
        public void append_Example1()
        {
            // Test Block 489.
            // Entry append: Insert content, specified by the parameter, to the end of each element in the set of matched elements.
            // Example 1: Appends some HTML to all paragraphs.
            PerformJQueryTest(@"
                  $(""p"").append(""<strong>Hello</strong>"");
            ");
        }

        [TestMethod]
        public void append_Example2()
        {
            // Test Block 490.
            // Entry append: Insert content, specified by the parameter, to the end of each element in the set of matched elements.
            // Example 2: Appends an Element to all paragraphs.
            PerformJQueryTest(@"
                  $(""p"").append(document.createTextNode(""Hello""));
            ");
        }

        [TestMethod]
        public void append_Example3()
        {
            // Test Block 491.
            // Entry append: Insert content, specified by the parameter, to the end of each element in the set of matched elements.
            // Example 3: Appends a jQuery object (similar to an Array of DOM Elements) to all paragraphs.
            PerformJQueryTest(@"
                  $(""p"").append( $(""strong"") );
            ");
        }

        [TestMethod]
        public void val_Example1()
        {
            // Test Block 492.
            // Entry val: Get the current value of the first element in the set of matched elements.
            // Example 1: Get the single value from a single select and an array of values from a multiple select and display their values.
            PerformJQueryTest(@"
                    function displayVals() {
                      var singleValues = $(""#single"").val();
                      var multipleValues = $(""#multiple"").val() || [];
                      $(""p"").html(""<b>Single:</b> "" + 
                                  singleValues +
                                  "" <b>Multiple:</b> "" + 
                                  multipleValues.join("", ""));
                    }
                    $(""select"").change(displayVals);
                    displayVals();
            ");
        }

        [TestMethod]
        public void val_Example2()
        {
            // Test Block 493.
            // Entry val: Get the current value of the first element in the set of matched elements.
            // Example 2: Find the value of an input box.
            PerformJQueryTest(@"
                    $(""input"").keyup(function () {
                      var value = $(this).val();
                      $(""p"").text(value);
                    }).keyup();
            ");
        }

        [TestMethod]
        public void val_1_Example1()
        {
            // Test Block 494.
            // Entry val_1: Set the value of each element in the set of matched elements.
            // Example 1: Set the value of an input box.
            PerformJQueryTest(@"
                    $(""button"").click(function () {
                      var text = $(this).text();
                      $(""input"").val(text);
                    });
            ");
        }

        [TestMethod]
        public void val_1_Example2()
        {
            // Test Block 495.
            // Entry val_1: Set the value of each element in the set of matched elements.
            // Example 2: Use the function argument to modify the value of an input box.
            PerformJQueryTest(@"
                  $('input').bind('blur', function() {
                    $(this).val(function( i, val ) {
                      return val.toUpperCase();
                    });
                  });
            ");
        }

        [TestMethod]
        public void val_1_Example3()
        {
            // Test Block 496.
            // Entry val_1: Set the value of each element in the set of matched elements.
            // Example 3: Set a single select, a multiple select, checkboxes and a radio button .
            PerformJQueryTest(@"
                    $(""#single"").val(""Single2"");
                    $(""#multiple"").val([""Multiple2"", ""Multiple3""]); 
                    $(""input"").val([""check1"",""check2"", ""radio1"" ]);
            ");
        }

        [TestMethod]
        public void text_1_Example1()
        {
            // Test Block 497.
            // Entry text_1: Get the combined text contents of each element in the set of matched elements, including their descendants.
            // Example 1: Find the text in the first paragraph (stripping out the html), then set the html of the last paragraph to show it is just text (the red bold is gone).
            PerformJQueryTest(@"
                    var str = $(""p:first"").text();
                    $(""p:last"").html(str);
            ");
        }

        [TestMethod]
        public void text_2_Example1()
        {
            // Test Block 498.
            // Entry text_2: Set the content of each element in the set of matched elements to the specified text.
            // Example 1: Add text to the paragraph (notice the bold tag is escaped).
            PerformJQueryTest(@"
                $(""p"").text(""<b>Some</b> new text."");
            ");
        }

        [TestMethod]
        public void html_Example1()
        {
            // Test Block 499.
            // Entry html: Get the HTML contents of the first element in the set of matched elements.
            // Example 1: Click a paragraph to convert it from html to text.
            PerformJQueryTest(@"
                    $(""p"").click(function () {
                      var htmlStr = $(this).html();
                      $(this).text(htmlStr);
                    });
            ");
        }

        [TestMethod]
        public void html_1_Example1()
        {
            // Test Block 500.
            // Entry html_1: Set the HTML contents of each element in the set of matched elements.
            // Example 1: Add some html to each div.
            PerformJQueryTest(@"
                $(""div"").html(""<span class='red'>Hello <b>Again</b></span>"");
            ");
        }

        [TestMethod]
        public void html_1_Example2()
        {
            // Test Block 501.
            // Entry html_1: Set the HTML contents of each element in the set of matched elements.
            // Example 2: Add some html to each div then immediately do further manipulations to the inserted html.
            PerformJQueryTest(@"
                    $(""div"").html(""<b>Wow!</b> Such excitement..."");
                    $(""div b"").append(document.createTextNode(""!!!""))
                              .css(""color"", ""red"");
            ");
        }

        [TestMethod]
        public void map_Example1()
        {
            // Test Block 502.
            // Entry map: Pass each element in the current matched set through a function, producing a new jQuery object containing the return values.
            // Example 1: Build a list of all the values within a form.
            PerformJQueryTest(@"
                    $(""p"").append( $(""input"").map(function(){
                      return $(this).val();
                    }).get().join("", "") );
            ");
        }

        [TestMethod]
        public void map_Example2()
        {
            // Test Block 503.
            // Entry map: Pass each element in the current matched set through a function, producing a new jQuery object containing the return values.
            // Example 2: A contrived example to show some functionality.
            PerformJQueryTest(@"
                var mappedItems = $(""li"").map(function (index) {
                  var replacement = $(""<li>"").text($(this).text()).get(0);
                  if (index == 0) {
                    /* make the first item all caps */
                    $(replacement).text($(replacement).text().toUpperCase());
                  } else if (index == 1 || index == 3) {
                    /* delete the second and fourth items */
                    replacement = null;
                  } else if (index == 2) {
                    /* make two of the third item and add some text */
                    replacement = [replacement,$(""<li>"").get(0)];
                    $(replacement[0]).append(""<b> - A</b>"");
                    $(replacement[1]).append(""Extra <b> - B</b>"");
                  }
                  /* replacement will be a dom element, null, 
                     or an array of dom elements */
                  return replacement;
                });
                $(""#results"").append(mappedItems);
            ");
        }

        [TestMethod]
        public void map_Example3()
        {
            // Test Block 504.
            // Entry map: Pass each element in the current matched set through a function, producing a new jQuery object containing the return values.
            // Example 3: Equalize the heights of the divs.
            PerformJQueryTest(@"
                $.fn.equalizeHeights = function() {
                  var maxHeight = this.map(function(i,e) {
                    return $(e).height();
                  }).get();
                  return this.height( Math.max.apply(this, maxHeight) );
                };
                $('input').click(function(){
                  $('div').equalizeHeights();
                });
            ", "equalizeHeights");
        }

        [TestMethod]
        public void is_Example1()
        {
            // Test Block 505.
            // Entry is: Check the current matched set of elements against a selector, element, or jQuery object and return true if at least one of these elements matches the given arguments.
            // Example 1: Shows a few ways is() can be used inside an event handler.
            PerformJQueryTest(@"
                  $(""div"").one('click', function () {
                    if ($(this).is("":first-child"")) {
                      $(""p"").text(""It's the first div."");
                    } else if ($(this).is("".blue,.red"")) {
                      $(""p"").text(""It's a blue or red div."");
                    } else if ($(this).is("":contains('Peter')"")) {
                      $(""p"").text(""It's Peter!"");
                    } else {
                      $(""p"").html(""It's nothing <em>special</em>."");
                    }
                    $(""p"").hide().slideDown(""slow"");
                    $(this).css({""border-style"": ""inset"", cursor:""default""});
                  });
            ");
        }

        [TestMethod]
        public void is_Example2()
        {
            // Test Block 506.
            // Entry is: Check the current matched set of elements against a selector, element, or jQuery object and return true if at least one of these elements matches the given arguments.
            // Example 2: Returns true, because the parent of the input is a form element.
            PerformJQueryTest(@"
                  var isFormParent = $(""input[type='checkbox']"").parent().is(""form"");
                  $(""div"").text(""isFormParent = "" + isFormParent);
            ");
        }

        [TestMethod]
        public void is_Example3()
        {
            // Test Block 507.
            // Entry is: Check the current matched set of elements against a selector, element, or jQuery object and return true if at least one of these elements matches the given arguments.
            // Example 3: Returns false, because the parent of the input is a p element.
            PerformJQueryTest(@"
                  var isFormParent = $(""input[type='checkbox']"").parent().is(""form"");
                  $(""div"").text(""isFormParent = "" + isFormParent);
            ");
        }

        [TestMethod]
        public void is_Example4()
        {
            // Test Block 508.
            // Entry is: Check the current matched set of elements against a selector, element, or jQuery object and return true if at least one of these elements matches the given arguments.
            // Example 4: Checks against an existing collection of alternating list elements. Blue, alternating list elements slide up while others turn red.
            PerformJQueryTest(@"
                  var $alt = $(""#browsers li:nth-child(2n)"").css(""background"", ""#00FFFF"");
                  $('li').click(function() {
                    var $li = $(this);
                    if ( $li.is( $alt ) ) {
                      $li.slideUp();
                    } else {
                      $li.css(""background"", ""red"");
                    }
                  });
            ");
        }

        [TestMethod]
        public void is_Example5()
        {
            // Test Block 509.
            // Entry is: Check the current matched set of elements against a selector, element, or jQuery object and return true if at least one of these elements matches the given arguments.
            // Example 5: An alternate way to achieve the above example using an element rather than a jQuery object. Checks against an existing collection of alternating list elements. Blue, alternating list elements slide up while others turn red.
            PerformJQueryTest(@"
                  var $alt = $(""#browsers li:nth-child(2n)"").css(""background"", ""#00FFFF"");
                  $('li').click(function() {
                    if ( $alt.is( this ) ) {
                      $(this).slideUp();
                    } else {
                      $(this).css(""background"", ""red"");
                    }
                  });
            ");
        }

        [TestMethod]
        public void eq_1_Example1()
        {
            // Test Block 510.
            // Entry eq_1: Reduce the set of matched elements to the one at the specified index.
            // Example 1: Turn the div with index 2 blue by adding an appropriate class.
            PerformJQueryTest(@"
                    $(""body"").find(""div"").eq(2).addClass(""blue"");
            ");
        }

        [TestMethod]
        public void filter_Example1()
        {
            // Test Block 511.
            // Entry filter: Reduce the set of matched elements to those that match the selector or pass the function's test. 
            // Example 1: Change the color of all divs; then add a border to those with a "middle" class.
            PerformJQueryTest(@"
                    $(""div"").css(""background"", ""#c8ebcc"")
                            .filter("".middle"")
                            .css(""border-color"", ""red"");
            ");
        }

        [TestMethod]
        public void filter_Example2()
        {
            // Test Block 512.
            // Entry filter: Reduce the set of matched elements to those that match the selector or pass the function's test. 
            // Example 2: Change the color of all divs; then add a border to the second one (index == 1) and the div with an id of "fourth."
            PerformJQueryTest(@"
                    $(""div"").css(""background"", ""#b4b0da"")
                            .filter(function (index) {
                                  return index == 1 || $(this).attr(""id"") == ""fourth"";
                                })
                            .css(""border"", ""3px double red"");
            ");
        }

        [TestMethod]
        public void filter_Example3()
        {
            // Test Block 513.
            // Entry filter: Reduce the set of matched elements to those that match the selector or pass the function's test. 
            // Example 3: Select all divs and filter the selection with a DOM element, keeping only the one with an id of "unique".
            PerformJQueryTest(@"
                $(""div"").filter( document.getElementById(""unique"") )
            ");
        }

        [TestMethod]
        public void filter_Example4()
        {
            // Test Block 514.
            // Entry filter: Reduce the set of matched elements to those that match the selector or pass the function's test. 
            // Example 4: Select all divs and filter the selection with a jQuery object, keeping only the one with an id of "unique".
            PerformJQueryTest(@"
                $(""div"").filter( $(""#unique"") )
            ");
        }

        [TestMethod]
        public void toggleClass_Example1()
        {
            // Test Block 515.
            // Entry toggleClass: Add or remove one or more classes from each element in the set of matched elements, depending on either the class's presence or the value of the switch argument.
            // Example 1: Toggle the class 'highlight' when a paragraph is clicked.
            PerformJQueryTest(@"
                    $(""p"").click(function () {
                      $(this).toggleClass(""highlight"");
                    });
            ");
        }

        [TestMethod]
        public void toggleClass_Example2()
        {
            // Test Block 516.
            // Entry toggleClass: Add or remove one or more classes from each element in the set of matched elements, depending on either the class's presence or the value of the switch argument.
            // Example 2: Add the "highlight" class to the clicked paragraph on every third click of that paragraph, remove it every first and second click.
            PerformJQueryTest(@"
                var count = 0;
                $(""p"").each(function() {
                  var $thisParagraph = $(this);
                  var count = 0;
                  $thisParagraph.click(function() {
                    count++;
                    $thisParagraph.find(""span"").text('clicks: ' + count);
                    $thisParagraph.toggleClass(""highlight"", count % 3 == 0);
                  });
                });
            ");
        }

        [TestMethod]
        public void toggleClass_Example3()
        {
            // Test Block 517.
            // Entry toggleClass: Add or remove one or more classes from each element in the set of matched elements, depending on either the class's presence or the value of the switch argument.
            // Example 3: Toggle the class name(s) indicated on the buttons for each div.    
            PerformJQueryTest(@"
                var cls = ['', 'a', 'a b', 'a b c'];
                var divs = $('div.wrap').children();
                var appendClass = function() {
                  divs.append(function() {
                    return '<div>' + (this.className || 'none') + '</div>';
                  });
                };
                appendClass();
                $('button').bind('click', function() {
                  var tc = this.className || undefined;
                  divs.toggleClass(tc);
                  appendClass();
                });
                $('a').bind('click', function(event) {
                  event.preventDefault();
                  divs.empty().each(function(i) {
                    this.className = cls[i];
                  });
                  appendClass();
                });
            ", "className", "className");
        }

        [TestMethod]
        public void removeClass_Example1()
        {
            // Test Block 518.
            // Entry removeClass: Remove a single class, multiple classes, or all classes from each element in the set of matched elements.
            // Example 1: Remove the class 'blue' from the matched elements.
            PerformJQueryTest(@"
                $(""p:even"").removeClass(""blue"");
            ");
        }

        [TestMethod]
        public void removeClass_Example2()
        {
            // Test Block 519.
            // Entry removeClass: Remove a single class, multiple classes, or all classes from each element in the set of matched elements.
            // Example 2: Remove the class 'blue' and 'under' from the matched elements.
            PerformJQueryTest(@"
                $(""p:odd"").removeClass(""blue under"");
            ");
        }

        [TestMethod]
        public void removeClass_Example3()
        {
            // Test Block 520.
            // Entry removeClass: Remove a single class, multiple classes, or all classes from each element in the set of matched elements.
            // Example 3: Remove all the classes from the matched elements.
            PerformJQueryTest(@"
                $(""p:eq(1)"").removeClass();
            ");
        }

        [TestMethod]
        public void hasClass_Example1()
        {
            // Test Block 521.
            // Entry hasClass: Determine whether any of the matched elements are assigned the given class.
            // Example 1: Looks for the paragraph that contains 'selected' as a class.
            PerformJQueryTest(@"
                $(""div#result1"").append($(""p:first"").hasClass(""selected"").toString());
                $(""div#result2"").append($(""p:last"").hasClass(""selected"").toString());
                $(""div#result3"").append($(""p"").hasClass(""selected"").toString());
            ");
        }

        [TestMethod]
        public void removeAttr_Example1()
        {
            // Test Block 522.
            // Entry removeAttr: Remove an attribute from each element in the set of matched elements.
            // Example 1: Clicking the button enables the input next to it.
            PerformJQueryTest(@"
                (function() {
                  var inputTitle = $(""input"").attr(""title"");
                  $(""button"").click(function () {
                    var input = $(this).next();
                    if ( input.attr(""title"") == inputTitle ) {
                      input.removeAttr(""title"")
                    } else {
                      input.attr(""title"", inputTitle);
                    }
                    $(""#log"").html( ""input title is now "" + input.attr(""title"") );
                  });
                })();
            ");
        }

        [TestMethod]
        public void attr_Example1()
        {
            // Test Block 523.
            // Entry attr: Get the value of an attribute for the first element in the set of matched elements.
            // Example 1: Find the title attribute of the first <em> in the page.
            PerformJQueryTest(@"
                var title = $(""em"").attr(""title"");
                  $(""div"").text(title);
            ");
        }

        [TestMethod]
        public void attr_1_Example1()
        {
            // Test Block 524.
            // Entry attr_1: Set one or more attributes for the set of matched elements.
            // Example 1: Set some attributes for all <img>s in the page.
            PerformJQueryTest(@"
                $(""img"").attr({ 
                  src: ""/images/hat.gif"",
                  title: ""jQuery"",
                  alt: ""jQuery Logo""
                });
                $(""div"").text($(""img"").attr(""alt""));
            ");
        }

        [TestMethod]
        public void attr_1_Example2()
        {
            // Test Block 525.
            // Entry attr_1: Set one or more attributes for the set of matched elements.
            // Example 2: Set the id for divs based on the position in the page.
            PerformJQueryTest(@"
                $(""div"").attr(""id"", function (arr) {
                  return ""div-id"" + arr;
                })
                .each(function () {
                  $(""span"", this).html(""(ID = '<b>"" + this.id + ""</b>')"");
                });
            ");
        }

        [TestMethod]
        public void attr_1_Example3()
        {
            // Test Block 526.
            // Entry attr_1: Set one or more attributes for the set of matched elements.
            // Example 3: Set the src attribute from title attribute on the image.
            PerformJQueryTest(@"
                $(""img"").attr(""src"", function() { 
                    return ""/images/"" + this.title; 
                });
            ");
        }

        [TestMethod]
        public void addClass_Example1()
        {
            // Test Block 527.
            // Entry addClass: Adds the specified class(es) to each of the set of matched elements.
            // Example 1: Adds the class "selected" to the matched elements.
            PerformJQueryTest(@"
                  $(""p:last"").addClass(""selected"");
            ");
        }

        [TestMethod]
        public void addClass_Example2()
        {
            // Test Block 528.
            // Entry addClass: Adds the specified class(es) to each of the set of matched elements.
            // Example 2: Adds the classes "selected" and "highlight" to the matched elements.
            PerformJQueryTest(@"
                  $(""p:last"").addClass(""selected highlight"");
            ");
        }

        [TestMethod]
        public void addClass_Example3()
        {
            // Test Block 529.
            // Entry addClass: Adds the specified class(es) to each of the set of matched elements.
            // Example 3: Pass in a function to .addClass() to add the "green" class to a div that already has a "red" class.
            PerformJQueryTest(@"
                  $(""div"").addClass(function(index, currentClass) {
                    var addedClass;
                    if ( currentClass === ""red"" ) {
                      addedClass = ""green"";
                      $(""p"").text(""There is one green div"");
                    }
                    return addedClass;
                  });
            ");
        }

    }
}
