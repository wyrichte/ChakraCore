using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace ArmScriptGenerator
{
    class Program
    {
        static void Main(string[] args)
        {
            //To run:
            //  ArmScriptGenerator rldirs.txt >armUnittests.bat
            if (args.Length == 0)
            {
                Console.WriteLine("Usage:");
                Console.WriteLine("\tArmScriptGenerator rldirs.txt >armUnittests.bat");
            }
            else
            {
                var batchFile = new BatchFile(args[0]);
                batchFile.Dump();
            }
        }

        class BatchFile
        {
            private class Test
            {
                public readonly string Script;
                public readonly string Baseline;
                public readonly string Options;

                public Test(string script, string baseline, string options, string path)
                {
                    this.Script = Path.Combine(path, script);
                    this.Baseline = Path.Combine(path, baseline);
                    this.Options = options;
                }
            }

            private readonly List<Test> _tests = new List<Test>();

            public BatchFile(string dirsfile)
            {
                string rootDirectory = Path.GetDirectoryName(dirsfile);
                using (TextReader reader = new StreamReader(dirsfile))
                {
                    while (true)
                    {
                        string line = reader.ReadLine();
                        if (line == null)
                        {
                            break;
                        }

                        line.Trim();
                        if ((line.Length != 0) && (line[0] != '#'))
                        {
                            this.AddTests(rootDirectory, line);
                        }
                    }
                }

                _tests.Sort(delegate(Test left, Test right) { return string.Compare(left.Baseline, right.Baseline, StringComparison.OrdinalIgnoreCase); });
            }

            public void Dump()
            {
                foreach (var test in _tests)
                {
                    Console.WriteLine(@"jc.exe -bvt -NoNative {0} \release\unittest\{1} >\release\unittest\{2}", test.Options, test.Script, test.Baseline);
                }
            }

            private void AddTests(string rootDirectory, string subDirectory)
            {
                XmlDocument document = new XmlDocument();
                document.Load(Path.Combine(Path.Combine(rootDirectory, subDirectory), "rlexe.xml"));

                var root = document.SelectSingleNode("regress-exe");
                if (root != null)
                {
                    foreach (XmlNode child in root.ChildNodes)
                    {
                        var test = child.SelectSingleNode("default");
                        if (test != null)
                        {
                            string tags = GetText(test, "tags");
                            if (!ContainsAnyWords(tags, "fail", "fails_interpreted", "exclude_interpreted"))
                            {
                                string script = GetText(test, "files");
                                if ((script != null) && (Path.GetExtension(script) == ".js"))
                                {
                                    string baseline = GetText(test, "baseline");
                                    if (baseline.Length == 0)
                                    {
                                        baseline = Path.ChangeExtension(script, ".baseline");
                                    }

                                    string options = GetText(test, "compile-flags");

                                    _tests.Add(new Test(script, baseline, options, subDirectory));
                                }
                            }
                        }
                    }
                }
            }

            private static bool ContainsAnyWords(string text, params string[] words)
            {
                if (text != null)
                {
                    foreach (var word in words)
                    {
                        if (ContainsWord(text, word))
                        {
                            return true;
                        }
                    }
                }

                return false;
            }

            private static bool ContainsWord(string text, string word)
            {
                int index = text.IndexOf(word, StringComparison.OrdinalIgnoreCase);
                if (index >= 0)
                {
                    if ((index == 0) || (text[index - 1] == ','))
                    {
                        index += word.Length;
                        if ((index == text.Length) || (text[index] == ','))
                        {
                            return true;
                        }
                    }
                }

                return false;
            }

            private static string GetText(XmlNode node, string label)
            {
                node = node.SelectSingleNode(label);
                if (node != null)
                {
                    return node.InnerText.Trim();
                }

                return string.Empty;
            }
        }
    }
}
