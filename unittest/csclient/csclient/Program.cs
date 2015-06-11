using Microsoft.CSharp.RuntimeBinder;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows;

[assembly: CLSCompliant(true)]

namespace csclient
{
    class Program
    {
        [STAThreadAttribute]
        static void Main(string[] args)
        {
            TestApp testApp = new TestApp();
            testApp.Run();

        }

    }

    class TestApp
    {
        public TestApp()
        {

        }

        void TestCase1()
        {
            Console.WriteLine("testcase 1: create two engine in the same thread, returning a sub object. it works in IE9 but probably not in IE8");
            browser = new WebBrowser();
            browser.DocumentText = "<HTML><BODY>This is a new HTML document.<script>var a = {one:1, two:2, test: function() {return 'test result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>";

            browser1 = new WebBrowser();
            browser1.DocumentText = "<HTML><BODY>This is a new HTML document.<script>var d,e,f;var a = {one:1, two:2, three:3, four:4, test: function() {return 'test result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>";
            // the follow test case (same string for both browser)would make it failed from mshtml for the same reason).
            //            browser1.DocumentText = "<HTML><BODY>This is a new HTML document.<script>var a = 1; var b = 2;function foo(){return window}; function test(){return 'test result0'}</script></BODY></HTML>";

            while (true)
            {
                if (browser1.ReadyState == WebBrowserReadyState.Complete &&
                    browser.ReadyState == WebBrowserReadyState.Complete)
                {
                    break;
                }
                System.Windows.Forms.Application.DoEvents();
            }
            //dynamic obj = browser.Document;
            HtmlDocument doc = browser.Document;
            dynamic result = doc.InvokeScript("foo");

            //dynamic obj1 = browser1.Document;
            HtmlDocument doc1 = browser1.Document;
            dynamic result1 = doc1.InvokeScript("foo");
            bool exceptionThrown = false;

            dynamic test = result.test;
            Console.WriteLine(test());
            try
            {
                dynamic test1 = result1.test;
                Console.Write(test1());
            }
            catch (Exception e)
            {
                Console.WriteLine("result1.test succeeded with exception" + e.Message);
                Console.WriteLine("stack is" + e.StackTrace);
                exceptionThrown = true;
            }
            if (!exceptionThrown)
            {
                Console.WriteLine("testcase 1 succeeded");
            }
            else
            {
                Console.WriteLine("testcase 1 failed");
            }
        }

        void TestCase2()
        {
            Console.WriteLine("testcase 2: create two engine in the same thread, returning a windows. failed in IE9 because mshtml doesn't like the DISPID. probably the same in IE8");
            browser = new WebBrowser();
            browser.DocumentText = "<HTML><BODY>This is a new HTML document.<script>var a = 1; var b = 2;function foo(){return window}; function test(){return 'test result0'}</script></BODY></HTML>";

            browser1 = new WebBrowser();
            browser1.DocumentText = "<HTML><BODY>This is a new HTML document.<script>var a = 1; var b = 2;function foo(){return window}; function test(){return 'test result0'}</script></BODY></HTML>";

            while (true)
            {
                if (browser1.ReadyState == WebBrowserReadyState.Complete &&
                    browser.ReadyState == WebBrowserReadyState.Complete)
                {
                    break;
                }
                System.Windows.Forms.Application.DoEvents();
            }
            //dynamic obj = browser.Document;
            HtmlDocument doc = browser.Document;
            dynamic result = doc.InvokeScript("foo");

            //dynamic obj1 = browser1.Document;
            HtmlDocument doc1 = browser1.Document;
            dynamic result1 = doc1.InvokeScript("foo");
            bool exceptionThrown = false;

            dynamic test = result.test;
            Console.WriteLine(test());
            try
            {
                dynamic test1 = result1.test;
                Console.Write(test1());
            }
            catch (Exception e)
            {
                Console.WriteLine("result1.test succeeded with exception" + e.Message);
                Console.WriteLine("stack is" + e.StackTrace);
                exceptionThrown = true;
            }
            if (!exceptionThrown)
            {
                Console.WriteLine("testcase 2 failed");
            }
            else
            {
                Console.WriteLine("testcase 2 succeeded");
            }
        }

        void TestCase3()
        {
            Console.WriteLine("testcase3, similar to case1, but different process");
            browser = new WebBrowser();
            browser.DocumentText = "<HTML><BODY>This is first HTML document.<script>var a = {done:1, dtwo:2, testtest: function() {return 'testdd result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>";

            HtmlWindow newWindow = browser.Document.Window.Open(new Uri("about:blank"), "displayWindow", "width=400,height=200,location=yes", false);
            newWindow.Document.Write("<HTML><BODY>This is seoncd HTML document.<script>var ddd,eee,ffff;var a = {done:1, dtwo:2, dthree:3, dfour:4, dfive:5, testtest: function() {return 'tttest result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>");

            while (true)
            {
                if (browser.ReadyState == WebBrowserReadyState.Complete)
                {
                    break;
                }
                System.Windows.Forms.Application.DoEvents();
            }
            //dynamic obj = browser.Document;
            HtmlDocument doc = browser.Document;
            dynamic result = doc.InvokeScript("foo");

            //dynamic obj1 = browser1.Document;
            HtmlDocument doc1 = newWindow.Document;
            dynamic result1 = doc1.InvokeScript("foo");
            bool exceptionThrown = false;

            dynamic test = result.testtest;
            Console.WriteLine(test());
            try
            {
                dynamic test1 = result1.testtest;
                Console.Write(test1());
            }
            catch (Exception e)
            {
                Console.WriteLine("result1.test succeeded with exception" + e.Message);
                Console.WriteLine("stack is" + e.StackTrace);
                exceptionThrown = true;
            }
            if (!exceptionThrown)
            {
                Console.WriteLine("testcase 3 failed");
            }
            else
            {
                Console.WriteLine("testcase 3 succeeded");
            }
            newWindow.Close();
        }

        void TestCase4()
        {
            Console.WriteLine("testcase4, similar to case3, but both different process");
            browser = new WebBrowser();
            browser.DocumentText = "<HTML><BODY>This is a new HTML document.<script>var a = {one:1, two:2, testtest: function() {return 'testtt result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>";

            HtmlWindow oneWindow = browser.Document.Window.Open(new Uri("about:blank"), "displayWindow", "width=400,height=200,location=yes", false);
            oneWindow.Document.Write("<HTML><BODY>This is first HTML document.<script>var abcd;var a = {aaa:1, bbb:2, done:1, dtwo:2, tesT4: function() {return 'testdd result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>");

            HtmlWindow newWindow = browser.Document.Window.Open(new Uri("about:blank"), "displayWindow", "width=400,height=200,location=yes", false);
            newWindow.Document.Write("<HTML><BODY>This is seoncd HTML document.<script>var dddd,ffffff;var a = {done:1, ccc:3, ggg:4, dtwo:2, dthree:3, dfour:4, dfive:5, tesT4: function() {return 'tttest result'}}; var b = 2;function foo(){return a}; </script></BODY></HTML>");

            while (true)
            {
                if (browser.ReadyState == WebBrowserReadyState.Complete)
                {
                    break;
                }
                System.Windows.Forms.Application.DoEvents();
            }
            HtmlDocument doc = oneWindow.Document;
            dynamic result = doc.InvokeScript("foo");

            HtmlDocument doc1 = newWindow.Document;
            dynamic result1 = doc1.InvokeScript("foo");
            bool exceptionThrown = false;

            dynamic test = result.tesT4;
            Console.WriteLine(test());
            try
            {
                dynamic test1 = result1.tesT4;
                Console.Write(test1());
            }
            catch (Exception e)
            {
                Console.WriteLine("result1.test succeeded with exception" + e.Message);
                Console.WriteLine("stack is" + e.StackTrace);
                exceptionThrown = true;
            }
            if (!exceptionThrown)
            {
                Console.WriteLine("testcase 4 failed");
            }
            else
            {
                Console.WriteLine("testcase 4 succeeded");
            }
            oneWindow.Close();
            newWindow.Close();
        }

        public void Run()
        {
            TestCase1();
            TestCase2();
            TestCase3();
            TestCase4();

        }


        private WebBrowser browser;
        private WebBrowser browser1;

    }
}
