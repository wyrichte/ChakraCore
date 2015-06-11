using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.BPT.Tests.DirectAuthor
{
    class COMAutoRelease : IDisposable
    {
        private List<object> _comObjects = new List<object>();

        public void Add(object o)
        {
            if (o != null && !_comObjects.Contains(o))
                _comObjects.Add(o);
        }

        public void Remove(object o)
        {
            var index = _comObjects.IndexOf(o);
            Assert.IsTrue(index >= 0);
            _comObjects.RemoveAt(index);
        }

        public void Dispose()
        {
            foreach (var obj in _comObjects)
                Marshal.ReleaseComObject(obj);
            _comObjects.Clear();
        }

        public static void Release(ref COMAutoRelease o)
        {
            if (o == null) return;
            o.Dispose();
            o = null;
        }
    }
}
