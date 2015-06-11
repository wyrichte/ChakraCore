using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.JavaScript.LanguageService.Shared;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.BPT.Tests.DirectAuthor
{
    public class AuthorTestSession : IAuthorTestSession
    {
        COMAutoRelease _comObjects = new COMAutoRelease();
        IAuthorServices _services;
        string _basePath;
        Dictionary<string, InMemoryFile> _handleMap = new Dictionary<string, InMemoryFile>();
        List<Context> _contexts = new List<Context>();
        private IntPtr _engine;

        public AuthorTestSession(string basePath)
        {
            _basePath = basePath;

            var scriptEnginePtr = IntPtr.Zero;
            _engine = FakeComActivator.CoCreateFromFile(DirectAuthorCheckinTests.DirectAuthorTest.Paths.JScript9LSFilePath, Guids.CLSID_JScript9LS, Guids.IID_IAuthorServices, out scriptEnginePtr);

            object objectForIUnknown = Marshal.GetObjectForIUnknown(scriptEnginePtr);

            _services = objectForIUnknown as IAuthorServices;
            ReleaseOnClose(_services);

            Marshal.Release(scriptEnginePtr);
        }

        public void Close()
        {
            foreach (var context in _contexts)
                context.Close();
            _contexts.Clear();
            COMAutoRelease.Release(ref _comObjects);
            _services = null;
            _handleMap.Clear();

            NativeMethods.FreeLibrary(_engine);
            //Marshal.Release(_engine);
        }

        public IAuthorTestFile ReadFile(string filename)
        {
            var normalizedName = Path.GetFullPath(Path.Combine(_basePath, filename)).ToLower();
            InMemoryFile file;
            if (_handleMap.TryGetValue(normalizedName, out file))
            {
                return file;
            }
            var text = File.ReadAllText(normalizedName);
            file = new InMemoryFile(filename, text);
            var handle = _services.RegisterFile(file);
            file.Handle = handle;
            ReleaseOnClose(file.Handle);
            _handleMap.Add(normalizedName, file);
            return file;
        }
        
        public IAuthorTestFile FileFromText(string text, string name = null)
        {
            if (text.StartsWith("!!"))
                return ReadFile(text.Substring(2));
            var file = new InMemoryFile(String.IsNullOrEmpty(name) ? null : name, text);
            file.Handle = _services.RegisterFile(file);
            ReleaseOnClose(file.Handle);
            return file;
        }

        public IAuthorTestContext OpenContext(IAuthorTestFile primaryFile, params IAuthorTestFile[] contextFiles)
        {
            return OpenContext(primaryFile, AuthorHostType.ahtBrowser, contextFiles);
        }

        public IAuthorTestContext OpenContext(IAuthorTestFile primaryFile, AuthorHostType hostType, params IAuthorTestFile[] contextFiles)
        {
            return OpenContext(primaryFile, hostType, AuthorScriptVersion.ScriptVersion12, contextFiles);
        }

        public IAuthorTestContext OpenContext(IAuthorTestFile primaryFile, AuthorHostType hostType, AuthorScriptVersion scriptVersion, params IAuthorTestFile[] contextFiles)
        {
            var result = new Context(this, hostType, scriptVersion, primaryFile, contextFiles);
            _contexts.Add(result);
            return result;
        }

        public IAuthorColorizeText GetColorizer()
        {
            return _services.GetColorizer();
        }

        public IAuthorFileHandle TakeOwnershipOf(IAuthorTestFile file)
        {
            var memFile = file as InMemoryFile;
            if (memFile != null)
            {
                _comObjects.Remove(memFile.Handle);
                return memFile.Handle;
            }
            return null;
        }

        public void Cleanup(bool exhaustive)
        {
            _services.Cleanup(exhaustive);
        }

        private T ReleaseOnClose<T>(T o)
        {
            _comObjects.Add(o);
            return o;
        }

        public class InMemoryFile : IAuthorFile, IAuthorTestFile
        {
            static int _instanceCount;
            string _filename;
            StringBuilder _builder;
            string _text;
            AuthorFileStatus _status;

            public InMemoryFile(string filename, string text)
            {
                _instanceCount++;
                _filename = filename;
                _builder = new StringBuilder(text);
                _text = text;
            }

            ~InMemoryFile()
            {
                _instanceCount--;
            }

            public static int InstanceCount 
            { 
                get { return _instanceCount; } 
            }

            public AuthorFileStatus Status { get { return _status; } }

            public string DisplayName { get { return _filename; } }
            public int Length { get { return _builder.Length; } }
            public string Text
            {
                get
                {
                    if (_text == null)
                        _text = _builder.ToString();
                    return _text;
                }
            }
            public IAuthorFileReader GetReader() { return new BuilderReader(_builder); }
            public void StatusChanged(AuthorFileStatus newStatus) { _status = newStatus; }

            internal IAuthorFileHandle Handle { get; set; }

            class BuilderReader : IAuthorFileReader
            {
                StringBuilder _builder;
                int _offset;

                public BuilderReader(StringBuilder builder) { _builder = builder; }

                public int Read(int length, char[] buffer)
                {
                    if (length < 0)
                        throw new ArgumentOutOfRangeException("length");
                    if (_offset + length > _builder.Length)
                    {
                        length = _builder.Length - _offset;
                        if (length < 0) length = 0;
                    }
                    _builder.CopyTo(_offset, buffer, 0, length);
                    _offset += length;
                    return length;
                }

                public void Seek(int offset)
                {
                    if (offset < 0 || offset >= _builder.Length)
                    {
                        throw new ArgumentOutOfRangeException("offset");
                    }
                    _offset = offset;
                }

                public void Close()
                {
                }
            }

            public void InsertText(int offset, string text)
            {
                if (offset >= 0 && offset <= _builder.Length)
                {
                    _builder.Insert(offset, text);
                    _text = null;
                    if (Handle != null)
                        Handle.FileChanged(1, new[] { new AuthorFileChange() { CharactersInserted = text.Length, CharactersRemoved = 0, Offset = offset } });
                }
            }

            public string DeleteText(int offset, int length)
            {
                if (offset >= 0 && offset + length <= _builder.Length)
                {
                    var removedText = _builder.ToString(offset, length);
                    _builder.Remove(offset, length);
                    _text = null;
                    if (Handle != null)
                        Handle.FileChanged(1, new[] { new AuthorFileChange() { CharactersInserted = 0, CharactersRemoved = length, Offset = offset } });
                    return removedText;
                }

                return null;
            }

            public string Replace(int offset, int length, string text)
            {
                if (offset >= 0 && offset + length <= _builder.Length)
                {
                    var removedText = _builder.ToString(offset, length);
                    _builder.Remove(offset, length);
                    _builder.Insert(offset, text);
                    _text = null;
                    if (Handle != null)
                        Handle.FileChanged(1, new[] { new AuthorFileChange() { CharactersInserted = text.Length, CharactersRemoved = length, Offset = offset } });
                    return removedText;
                }

                return null;
            }

            public int OffsetOf(Regex regex)
            {
                var match = Match(regex);
                if (match.Success)
                    return match.Index;
                return -1;
            }

            public int OffsetAfter(Regex regex)
            {
                var match = Match(regex);
                if (match.Success)
                    return match.Index + match.Length;
                return -1;
            }

            public string Name
            {
                get { return DisplayName; }
            }

            public Match Match(Regex regex)
            {
                return regex.Match(Text);
            }

            public Match Match(int offset, Regex regex)
            {
                return regex.Match(Text, offset);
            }

            public int OffsetOf(string text)
            {
                return Text.IndexOf(text);
            }

            public int OffsetAfter(string text)
            {
                var result = Text.IndexOf(text);
                if (result >= 0) result += text.Length;
                return result;
            }

            public IAuthorFileHandle GetHandle()
            {
                return this.Handle;
            }
        }

        class Context : IAuthorTestContext, IAuthorFileContext
        {
            COMAutoRelease _comObjects = new COMAutoRelease();
            AuthorTestSession _session;
            IAuthorFileAuthoring _authoring;
            InMemoryFile _primaryFile;
            List<InMemoryFile> _contextFiles = new List<InMemoryFile>();
            HighResolutionTimer _callTimer = new HighResolutionTimer();
            List<string> _loggedMessages = new List<string>();
            Dictionary<string, InMemoryFile> _asyncScripts = new Dictionary<string, InMemoryFile>();
            AuthorHostType _hostType;
            AuthorScriptVersion _scriptVersion;

            public Context(AuthorTestSession session, AuthorHostType hostType, AuthorScriptVersion scriptVersion, IAuthorTestFile primaryFile, IAuthorTestFile[] contextFiles)
            {
                _session = session;
                _primaryFile = primaryFile as InMemoryFile;
                AddContextFiles(contextFiles);
                _authoring = _session._services.GetFileAuthoring(this);
                ReleaseOnClose(_authoring);
                _hostType = hostType;
                _scriptVersion = scriptVersion;
            }

            public IEnumerable<string> LoggedMessages
            {
                get { return _loggedMessages; }
            }

            public IAuthorTestFile PrimaryFile { get { return _primaryFile; } }

            public void Close()
            {
                COMAutoRelease.Release(ref _comObjects);
                _contextFiles.Clear();
                _asyncScripts.Clear();
            }

            public void AddContextFiles(params IAuthorTestFile[] files)
            {
                bool modified = false;

                if (files == null) return;
                foreach (var f in files)
                {
                    var file = f as InMemoryFile;
                    if (!_contextFiles.Contains(file))
                    {
                        _contextFiles.Add(file);
                        modified = true;
                    }
                }

                if (modified && _authoring != null)
                    _authoring.ContextChanged();
            }

            public void RemoveContextFiles(params IAuthorTestFile[] files)
            {
                bool modified = false;
                if (files == null) return;
                foreach (var f in files)
                {
                    var file = f as InMemoryFile;
                    if (_contextFiles.Contains(file))
                    {
                        _contextFiles.Remove(file);
                        modified = true;
                    }
                }

                if (modified && _authoring != null)
                    _authoring.ContextChanged();
            }

            public void Update(out Int64 callTime)
            {
                _callTimer.Start();
                this.Update();
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;
            }

            public void Update()
            {
                _authoring.Update();
            }

            public void Hurry(int phaseId)
            {
                _authoring.Hurry(phaseId);
            }

            public IAuthorRegionSet GetRegions(out Int64 callTime)
            {
                _callTimer.Start();
                IAuthorRegionSet regions = this.GetRegions();
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;

                return regions;
            }

            public IAuthorRegionSet GetRegions()
            {
                return _authoring.GetRegions();
            }

            public string GetASTAsJSON(out Int64 callTime)
            {
                _callTimer.Start();
                string ast = this.GetASTAsJSON();
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;

                return ast;
            }

            public string GetASTAsJSON()
            {
                return _authoring.GetASTAsJSON();
            }

            public IAuthorParseNodeCursor GetASTCursor(out Int64 callTime)
            {
                _callTimer.Start();
                IAuthorParseNodeCursor cursor = this.GetASTCursor();
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;

                return cursor;
            }

            public IAuthorParseNodeCursor GetASTCursor()
            {
                return ReleaseOnClose(_authoring.GetASTCursor());
            }

            public IAuthorMessageSet GetMessages(out Int64 callTime)
            {
                _callTimer.Start();
                IAuthorMessageSet messages = this.GetMessages();
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;

                return messages;
            }

            public IAuthorMessageSet GetMessages()
            {
                return _primaryFile.Handle.GetMessageSet();
            }

            public IAuthorCompletionSet GetCompletionsAt(int offset, AuthorCompletionFlags flags, out Int64 callTime)
            {
                _callTimer.Start();
                IAuthorCompletionSet completions = this.GetCompletionsAt(offset, flags);
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;

                return completions;
            }

            public IAuthorCompletionSet GetCompletionsAt(int offset, AuthorCompletionFlags flags)
            {
                return ReleaseOnClose(_authoring.GetCompletions(offset, flags));
            }

            public AuthorParameterHelp GetParameterHelpAt(int offset, out Int64 callTime)
            {
                _callTimer.Start();
                AuthorParameterHelp paramHelp = this.GetParameterHelpAt(offset);
                _callTimer.End();
                callTime = _callTimer.ElapseTimeInMilliseconds;

                return paramHelp;
            }

            public AuthorParameterHelp GetParameterHelpAt(int offset)
            {
                var result = new AuthorParameterHelp();
                AuthorDiagStatus status;
                result.FunctionHelp = ReleaseOnClose(_authoring.GetFunctionHelp(offset, AuthorFunctionHelpFlags.afhfDefault, out result.ParameterIndex, out result.Region, out status));
                return result;
            }

            public AuthorParameterHelp GetImplicitParameterHelpAt(int offset)
            {
                var result = new AuthorParameterHelp();
                AuthorDiagStatus status;
                result.FunctionHelp = ReleaseOnClose(_authoring.GetFunctionHelp(offset, AuthorFunctionHelpFlags.afhfImplicitRequest, out result.ParameterIndex, out result.Region, out status));
                return result;
            }

            public IAuthorSymbolHelp GetQuickInfoAt(int offset)
            {
                AuthorFileRegion extent;
                return ReleaseOnClose(_authoring.GetQuickInfo(offset, out extent)); 
            }

            public IAuthorFileHandle GetPrimaryFile()
            {
                return _primaryFile.Handle;
            }

            public int ContextFileCount
            {
                get { return _contextFiles.Count; }
            }

            public void GetContextFiles(int startIndex, int count, IAuthorFileHandle[] files)
            {
                if (startIndex < 0) return;
                for (int i = 0; i < Math.Min(files.Length, count); i++)
                {
                    var index = i + startIndex;
                    if (index < _contextFiles.Count)
                    {
                        var handle = _contextFiles[startIndex + index].Handle;
                        files[i] = handle;
                    }
                    else
                        files[i] = null;
                }
            }

            public AuthorHostType HostType
            {
                get { return _hostType; }
            }

            public AuthorScriptVersion GetScriptVersion()
            {
                return _scriptVersion;
            }

            public void GetDefinitionLocation(int offset, out IAuthorTestFile file, out int location)
            {
                IAuthorFileHandle handle;
                _authoring.GetDefinitionLocation(offset, out handle, out location);
                if (handle != null)
                {
                    if (handle == _primaryFile.Handle)
                        file = _primaryFile;
                    else
                        file = _contextFiles.Where(f => f.Handle == handle).FirstOrDefault();
                }
                else
                    file = null;
            }

            public IAuthorReferenceSet GetReferences(int offset)
            {
                return ReleaseOnClose(_authoring.GetReferences(offset));
            }

            public void FileAuthoringPhaseChanged(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle executingFile)
            {
                if (PhaseChanged != null)
                    PhaseChanged(phase, phaseId);
                if (executingFile != null)
                    Marshal.ReleaseComObject(executingFile);
            }

            public event Action<AuthorFileAuthoringPhase, int> PhaseChanged;

            public void LogMessage(string message)
            {
                _loggedMessages.Add(message);
                System.Diagnostics.Debug.WriteLine(message);
            }

            public AuthorFileAsynchronousScriptStatus VerifyScriptInContext(
                [MarshalAs(UnmanagedType.BStr)] string url,
                [MarshalAs(UnmanagedType.BStr)] string charset,
                [MarshalAs(UnmanagedType.BStr)] string type,
                IAuthorFileHandle sourceFile,
                [MarshalAs(UnmanagedType.VariantBool)] bool insertBeforeSourceFile
            )
            {
                var source = GetFileByHandle(sourceFile);
                System.Diagnostics.Debug.Write(String.Format("Recieved async request for '{0}'.  In file '{1}' ...  ", url, source.Name));

                InMemoryFile asyncFile = null;

                if (!String.IsNullOrEmpty(url))
                {
                    if (_asyncScripts.ContainsKey(url))
                    {
                        asyncFile = _asyncScripts[url];
                    }
                    else
                    {
                        // file does not exist, ignore it
                        System.Diagnostics.Debug.WriteLine("Status: Missing form asyncList");
                        return AuthorFileAsynchronousScriptStatus.afassIgnored;
                    }
                }
                else
                { 
                    // both url and text are empty
                    Assert.Fail("url is empty");
                    return AuthorFileAsynchronousScriptStatus.afassLoaded;
                }

                // findout if the file is already loaded or not
                foreach (var contextFile in _contextFiles)
                {
                    if (asyncFile == contextFile)
                    {
                        /// TODO: handle cases where the requested file is loaded but in the wrong order
                        System.Diagnostics.Debug.WriteLine("Status: Loaded");
                        return AuthorFileAsynchronousScriptStatus.afassLoaded;
                    }
                }

                // find the location to insert the new file
                int index = 0;
                if (sourceFile == _primaryFile.Handle)
                {
                    index = _contextFiles.Count;
                }
                else
                {
                    // the source file is one of the context files, find it.
                    foreach (var contextFile in _contextFiles)
                    {
                        if (contextFile.Handle == sourceFile)
                        {
                            break;
                        }
                        index++;
                    }
                    if (index == _contextFiles.Count)
                    {
                        Assert.Fail("Counld not find the source file in the context list");
                        return AuthorFileAsynchronousScriptStatus.afassIgnored;
                    }
                }

                if (!insertBeforeSourceFile && index < _contextFiles.Count)
                    index++;

                // add the new async file in the expected location
                var file = asyncFile as InMemoryFile;
                _contextFiles.Insert(index, file);
                if (_authoring != null)
                {
                    _authoring.ContextChanged();
                }

                System.Diagnostics.Debug.WriteLine("Status: Added");
                // notify the engin that the file was found and will be added to the context
                return AuthorFileAsynchronousScriptStatus.afassAdded;
            }

            public void AddAsyncScript(string url, IAuthorTestFile testFile)
            {
                if (!String.IsNullOrEmpty(url) && !_asyncScripts.ContainsKey(url))
                {
                    var file = testFile as InMemoryFile;
                    _asyncScripts.Add(url, file);
                }
            }

            public void TakeOwnership(object o)
            {
                _comObjects.Remove(o);
            }

            public IAuthorDiagnostics GetDiagnostics()
            {
                return ReleaseOnClose(_authoring.GetDiagnostics());
            }

            class StructureWrapper : IAuthorStructure
            {
                private Context _context;
                private IAuthorStructure _structure;

                public StructureWrapper(Context context, IAuthorStructure structure)
                {
                    _context = context;
                    _structure = structure;
                }

                public IAuthorStructureNodeSet GetAllNodes()
                {
                    return _context.ReleaseOnClose(_structure.GetAllNodes());
                }

                public IAuthorStructureNodeSet GetContainerNodes()
                {
                    return _context.ReleaseOnClose(_structure.GetContainerNodes());
                }

                public IAuthorStructureNodeSet GetChildrenOf(int key)
                {
                    return _context.ReleaseOnClose(_structure.GetChildrenOf(key));
                }
            }

            public IAuthorStructure GetStructure()
            {
                return new StructureWrapper(this, ReleaseOnClose(_authoring.GetStructure()));
            }

            private class CompletionCallback : IAuthorCompletionDiagnosticCallback
            {
                Action _routine;

                public CompletionCallback(Action routine)
                {
                    _routine = routine;
                }

                public void Invoke()
                {
                    if (_routine != null)
                    {
                        _routine();
                    }
                }
            }

            public void SetCompletionDiagnosticCallback(Action callback)
            {
                _authoring.SetCompletionDiagnosticCallback(new CompletionCallback(callback));
            }

            private T ReleaseOnClose<T>(T o)
            {
                _comObjects.Add(o);
                return o;
            }

            public IAuthorTestFile GetFileByText(string text)
            {
                if (text == PrimaryFile.Text)
                    return PrimaryFile;

                foreach (var contextFile in this._contextFiles)
                {
                    if (text == contextFile.Text)
                        return contextFile;
                }

                return null;
            }

            private IAuthorTestFile GetFileByHandle(IAuthorFileHandle fileHandle)
            {
                if (fileHandle == PrimaryFile.GetHandle())
                    return PrimaryFile;

                foreach (var contextFile in this._contextFiles)
                {
                    if (fileHandle == contextFile.GetHandle())
                        return contextFile;
                }

                return null;
            } 
        }
    }

    public static class AuthorTestContextExtensions
    {
        public static void GetAllocations(this IAuthorTestContext context, Action<IEnumerable<IAuthorAllocInfo>> handler)
        {
#if DEBUG
            try
            {
                var diagnostics = context.GetDiagnostics();
                var allocs = diagnostics.AllocStats;
                var allocsArray = allocs.ToEnumerable().ToArray();

                handler(allocsArray);

                //
                // Make sure to release and null all COM interfaces before leaving this method.
                //

                for (int a = 0; a < allocsArray.Length; a++)
                {
                    Marshal.ReleaseComObject(allocsArray[a]);
                    allocsArray[a] = null;
                }

                Marshal.ReleaseComObject(allocs);
                allocs = null;

                context.TakeOwnership(diagnostics);
                Marshal.ReleaseComObject(diagnostics);
                diagnostics = null;
            }
            catch (NotImplementedException)
            {
                // LS bits are not CHK
            }
 #endif
        }
    };
}
