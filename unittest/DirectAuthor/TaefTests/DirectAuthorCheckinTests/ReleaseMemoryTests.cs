using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class ReleaseMemoryTests : DirectAuthorTest
    {
        [TestMethod]
        public void ReleaseMemoryOnAPoolThread()
        {
            var testDone = new ManualResetEvent(false);

            ThreadPool.QueueUserWorkItem(_ =>
            {
                // Create a session on a non-STA thread.
                var session = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var text = "var a = ''; a.";
                    var primary = session.FileFromText(text);
                    var context = session.OpenContext(primary);

                    var completions = context.GetCompletionsAt(text.Length);

                    // Prevent the normal clean-up of the context.
                    context.TakeOwnership(completions);

                    var done = new ManualResetEvent(false);

                    // Release the memory on a different thread than we allocated it.
                    ThreadPool.QueueUserWorkItem(dummy =>
                    {
                        Marshal.ReleaseComObject(completions);
                        done.Set();
                    }, null);

                    // Wait until we have released the object
                    done.WaitOne();

                    // Call cleanup which triggers the completion list to free.
                    session.Cleanup(true);
                }
                finally
                {
                    session.Close();
                    testDone.Set();
                }
            }, null);

            testDone.WaitOne();
        }
    }
}
