using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Collections;
using System.Runtime.InteropServices.WindowsRuntime;
namespace Animals
{
    class MyITerator<T> : IIterator<T>
    {
        private List<T> list = new List<T>();
        private int m_CurrentIndex = 0;
        public T Current
        {
            get { return list[m_CurrentIndex]; }
        }
        public uint GetMany([WriteOnlyArray]T[] items)
        {
            int i = 0;
            if (m_CurrentIndex < list.Count)
            {
                for (i = 0; ((i + m_CurrentIndex) < list.Count) && (i < items.Length); i++)
                {
                    items[i] = list[i + m_CurrentIndex];
                }
                m_CurrentIndex = m_CurrentIndex + i;
            }
            return (uint)i;
        }
        public bool HasCurrent
        {
            get { return m_CurrentIndex < list.Count ? true : false; }
        }
        public bool MoveNext()
        {
            m_CurrentIndex = m_CurrentIndex + 1;
            return HasCurrent;
        }
        public void Add(T t)
        {
            list.Add(t);
        }
    }


    class AnimalReadOnlyVector_int : IList<int>
    {
        private IList<int> m_pVector;
        private Animal m_pAnimal;
        public AnimalReadOnlyVector_int(IList<int> vector, Animal animal)
        {
            m_pVector = vector;
            m_pAnimal = animal;
        }
        public int IndexOf(int item)
        {
            return m_pVector.IndexOf(item);
        }
        public void Insert(int index, int item)
        {
            throw new NotImplementedException();
        }
        public void RemoveAt(int index)
        {
            throw new NotImplementedException();
        }
        public int this[int index]
        {
            get
            {
                return m_pVector[index];
            }
            set
            {
                throw new NotImplementedException();
            }
        }
        public void Add(int item)
        {
            throw new NotImplementedException();
        }
        public void Clear()
        {
            throw new NotImplementedException();
        }
        public bool Contains(int item)
        {
            throw new NotImplementedException();
        }
        public void CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }
        public int Count
        {
            get { return m_pVector.Count; }
        }
        public bool IsReadOnly
        {
            get { throw new NotImplementedException(); }
        }
        public bool Remove(int item)
        {
            throw new NotImplementedException();
        }
        public IEnumerator<int> GetEnumerator()
        {
            return (IEnumerator<int>)m_pVector.GetEnumerator(); 
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_pVector.GetEnumerator();
        }
    }
    public sealed class SingleIVector : IList<int>
    {
        private List<int> m_intList;
        public SingleIVector()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
        }
        public int IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        public void Insert(int index, int item)
        {
            m_intList.Insert(index, item);
        }
        public void RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
        }
        public int this[int index]
        {
            get
            {
                return m_intList[index];
            }
            set
            {
                m_intList[index] = value;
            }
        }
        public void Add(int item)
        {
            m_intList.Add(item);
        }
        public void Clear()
        {
            m_intList.Clear();
        }
        public bool Contains(int item)
        {
            return m_intList.Contains(item);
        }
        public void CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            m_intList.CopyTo(array, arrayIndex);
        }
        public int Count
        {
            get { return m_intList.Count; }
        }
        public bool IsReadOnly
        {
            get { return false; }
        }
        public bool Remove(int item)
        {
            return m_intList.Remove(item);
        }
        public IEnumerator<int> GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
    }
    public sealed class DoubleIVector : IList<int>, IList<String>
    {
        private List<int> m_intList;
        private List<string> m_stringList;
        public DoubleIVector()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
            m_stringList = new List<string>();
            for (int i = 1; i < 5; i++)
            {
                m_stringList.Add("String" + i);
            }
        }
        int IList<int>.IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        void IList<int>.Insert(int index, int item)
        {
            m_intList.Insert(index, item);
        }
        void IList<int>.RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
        }
        int IList<int>.this[int index]
        {
            get
            {
                return m_intList[index];
            }
            set
            {
                m_intList[index] = value;
            }
        }
        void ICollection<int>.Add(int item)
        {
            m_intList.Add(item);
        }
        void ICollection<int>.Clear()
        {
            m_intList.Clear();
        }
        bool ICollection<int>.Contains(int item)
        {
            return m_intList.Contains(item);
        }
        void ICollection<int>.CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            m_intList.CopyTo(array, arrayIndex);
        }
        int ICollection<int>.Count
        {
            get { return m_intList.Count; }
        }
        bool ICollection<int>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<int>.Remove(int item)
        {
            return m_intList.Remove(item);
        }
        IEnumerator<int> IEnumerable<int>.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        int IList<string>.IndexOf(string item)
        {
            return m_stringList.IndexOf(item);
        }
        void IList<string>.Insert(int index, string item)
        {
            m_stringList.Insert(index, item);
        }
        void IList<string>.RemoveAt(int index)
        {
            m_stringList.RemoveAt(index);
        }
        string IList<string>.this[int index]
        {
            get
            {
                return m_stringList[index];
            }
            set
            {
                m_stringList[index] = value;
            }
        }
        void ICollection<string>.Add(string item)
        {
            m_stringList.Add(item);
        }
        void ICollection<string>.Clear()
        {
            m_stringList.Clear();
        }
        bool ICollection<string>.Contains(string item)
        {
            return m_stringList.Contains(item);
        }
        void ICollection<string>.CopyTo([WriteOnlyArray]string[] array, int arrayIndex)
        {
            m_stringList.CopyTo(array, arrayIndex);
        }
        int ICollection<string>.Count
        {
            get { return m_stringList.Count; }
        }
        bool ICollection<string>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<string>.Remove(string item)
        {
            return m_stringList.Remove(item);
        }
        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            return m_stringList.GetEnumerator();
        }
    }

    public sealed class MultipleIVector : IList<string>, IList<IAnimal>, IList<int>
    {
        private List<IAnimal> m_animalList;
        private List<int> m_intList;
        private List<string> m_stringList;
        public MultipleIVector()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
            m_animalList = new List<IAnimal>();
            for (int i = 1; i < 4; i++)
            {
                Animal animal = new Animal();
                animal.SetGreeting("Animal" + i);
                m_animalList.Add(animal);
            }
            m_stringList = new List<string>();
            for (int i = 1; i < 5; i++)
            {
                m_stringList.Add("String" + i);
            }
        }
        int IList<IAnimal>.IndexOf(IAnimal item)
        {
            return m_animalList.IndexOf(item);
        }
        void IList<IAnimal>.Insert(int index, IAnimal item)
        {
            m_animalList.Insert(index, item);
        }
        void IList<IAnimal>.RemoveAt(int index)
        {
            m_animalList.RemoveAt(index);
        }
        IAnimal IList<IAnimal>.this[int index]
        {
            get
            {
                return m_animalList[index];
            }
            set
            {
                m_animalList[index] = value;
            }
        }
        void ICollection<IAnimal>.Add(IAnimal item)
        {
            m_animalList.Add(item);
        }
        void ICollection<IAnimal>.Clear()
        {
            m_animalList.Clear();
        }
        bool ICollection<IAnimal>.Contains(IAnimal item)
        {
            return m_animalList.Contains(item);
        }
        void ICollection<IAnimal>.CopyTo([WriteOnlyArray]IAnimal[] array, int arrayIndex)
        {
            m_animalList.CopyTo(array, arrayIndex);
        }
        int ICollection<IAnimal>.Count
        {
            get { return m_animalList.Count; }
        }
        bool ICollection<IAnimal>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<IAnimal>.Remove(IAnimal item)
        {
            return m_animalList.Remove(item);
        }
        IEnumerator<IAnimal> IEnumerable<IAnimal>.GetEnumerator()
        {
            return m_animalList.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_animalList.GetEnumerator();
        }
        int IList<int>.IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        void IList<int>.Insert(int index, int item)
        {
            m_intList.Insert(index, item);
        }
        void IList<int>.RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
        }
        int IList<int>.this[int index]
        {
            get
            {
                return m_intList[index];
            }
            set
            {
                m_intList[index] = value;
            }
        }
        void ICollection<int>.Add(int item)
        {
            m_intList.Add(item);
        }
        void ICollection<int>.Clear()
        {
            m_intList.Clear();
        }
        bool ICollection<int>.Contains(int item)
        {
            return m_intList.Contains(item);
        }
        void ICollection<int>.CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            m_intList.CopyTo(array, arrayIndex);
        }
        int ICollection<int>.Count
        {
            get { return m_intList.Count; }
        }
        bool ICollection<int>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<int>.Remove(int item)
        {
            return m_intList.Remove(item);
        }
        IEnumerator<int> IEnumerable<int>.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        int IList<string>.IndexOf(string item)
        {
            return m_stringList.IndexOf(item);
        }
        void IList<string>.Insert(int index, string item)
        {
            m_stringList.Insert(index, item);
        }
        void IList<string>.RemoveAt(int index)
        {
            m_stringList.RemoveAt(index);
        }
        string IList<string>.this[int index]
        {
            get
            {
                return m_stringList[index];
            }
            set
            {
                m_stringList[index] = value;
            }
        }
        void ICollection<string>.Add(string item)
        {
            m_stringList.Add(item);
        }
        void ICollection<string>.Clear()
        {
            m_stringList.Clear();
        }
        bool ICollection<string>.Contains(string item)
        {
            return m_stringList.Contains(item);
        }
        void ICollection<string>.CopyTo([WriteOnlyArray]string[] array, int arrayIndex)
        {
            m_stringList.CopyTo(array, arrayIndex);
        }
        int ICollection<string>.Count
        {
            get { return m_stringList.Count; }
        }
        bool ICollection<string>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<string>.Remove(string item)
        {
            return m_stringList.Remove(item);
        }
        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            return m_stringList.GetEnumerator();
        }
    }
    public interface ISingleIVector : IList<int>
    {
    }
    public sealed class InterfaceWithSingleIVector : ISingleIVector, IReadOnlyList<float>
    {
        private List<int> m_intList;
        private IReadOnlyList<float> m_floatList;
        public InterfaceWithSingleIVector()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
            List<float> floatList = new List<float>();
            floatList.Add((float)0.25);
            floatList.Add((float)0.50);
            floatList.Add((float)0.75);
            floatList.Add((float)1.25);
            floatList.Add((float)1.50);
            m_floatList = floatList;
        }
        int IList<int>.IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        void IList<int>.Insert(int index, int item)
        {
            m_intList.Insert(index, item);
        }
        void IList<int>.RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
        }
        int IList<int>.this[int index]
        {
            get
            {
                return m_intList[index];
            }
            set
            {
                m_intList[index] = value;
            }
        }
        void ICollection<int>.Add(int item)
        {
            m_intList.Add(item);
        }
        void ICollection<int>.Clear()
        {
            m_intList.Clear();
        }
        bool ICollection<int>.Contains(int item)
        {
            return m_intList.Contains(item);
        }
        void ICollection<int>.CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            m_intList.CopyTo(array, arrayIndex);
        }
        int ICollection<int>.Count
        {
            get { return m_intList.Count; }
        }
        bool ICollection<int>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<int>.Remove(int item)
        {
            return m_intList.Remove(item);
        }
        IEnumerator<int> IEnumerable<int>.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        float IReadOnlyList<float>.this[int index]
        {
            get { return m_floatList[index]; }
        }
        IEnumerator<float> IEnumerable<float>.GetEnumerator()
        {
            return m_floatList.GetEnumerator();
        }
        int IReadOnlyCollection<float>.Count
        {
            get { return m_floatList.Count; }
        }
    }
    public interface IDoubleIVector : IList<int>, IList<IAnimal>
    {
    }
    public sealed class InterfaceWithDoubleIVector : IDoubleIVector, IReadOnlyList<string>
    {
        private List<int> m_intList;
        private List<IAnimal> m_animalList;
        private IReadOnlyList<string> m_stringList;
        public InterfaceWithDoubleIVector()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
            m_animalList = new List<IAnimal>();
            for (int i = 1; i < 4; i++)
            {
                Animal animal = new Animal();
                animal.SetGreeting("Animal" + i);
                m_animalList.Add(animal);
            }
            List<string> stringList = new List<string>();
            for (int i = 1; i < 8; i++)
            {
                stringList.Add("ViewString" + i);
            }
            m_stringList = stringList;
        }
        int IList<int>.IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        void IList<int>.Insert(int index, int item)
        {
            m_intList.Insert(index, item);
        }
        void IList<int>.RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
        }
        int IList<int>.this[int index]
        {
            get
            {
                return m_intList[index];
            }
            set
            {
                m_intList[index] = value;
            }
        }
        void ICollection<int>.Add(int item)
        {
            m_intList.Add(item);
        }
        void ICollection<int>.Clear()
        {
            m_intList.Clear();
        }
        bool ICollection<int>.Contains(int item)
        {
            return m_intList.Contains(item);
        }
        void ICollection<int>.CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            m_intList.CopyTo(array, arrayIndex);
        }
        int ICollection<int>.Count
        {
            get { return m_intList.Count; }
        }
        bool ICollection<int>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<int>.Remove(int item)
        {
            return m_intList.Remove(item);
        }
        IEnumerator<int> IEnumerable<int>.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        int IList<IAnimal>.IndexOf(IAnimal item)
        {
            return m_animalList.IndexOf(item);
        }
        void IList<IAnimal>.Insert(int index, IAnimal item)
        {
            m_animalList.Insert(index, item);
        }
        void IList<IAnimal>.RemoveAt(int index)
        {
            m_animalList.RemoveAt(index);
        }
        IAnimal IList<IAnimal>.this[int index]
        {
            get
            {
                return m_animalList[index];
            }
            set
            {
                m_animalList[index] = value;
            }
        }
        void ICollection<IAnimal>.Add(IAnimal item)
        {
            m_animalList.Add(item);
        }
        void ICollection<IAnimal>.Clear()
        {
            m_animalList.Clear();
        }
        bool ICollection<IAnimal>.Contains(IAnimal item)
        {
            return m_animalList.Contains(item);
        }
        void ICollection<IAnimal>.CopyTo([WriteOnlyArray]IAnimal[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }
        int ICollection<IAnimal>.Count
        {
            get { return m_animalList.Count; }
        }
        bool ICollection<IAnimal>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<IAnimal>.Remove(IAnimal item)
        {
            return m_animalList.Remove(item);
        }
        IEnumerator<IAnimal> IEnumerable<IAnimal>.GetEnumerator()
        {
            return m_animalList.GetEnumerator();
        }
        string IReadOnlyList<string>.this[int index]
        {
            get { return m_stringList[index]; }
        }
        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            return m_stringList.GetEnumerator();
        }
        int IReadOnlyCollection<string>.Count
        {
            get { return m_stringList.Count; }
        }
    }
    public interface IMultipleIVector : IList<int>, IList<IAnimal>, IReadOnlyList<string>, IReadOnlyList<Guid>
    {
    }
    public sealed class InterfaceWithMultipleIVector : IMultipleIVector
    {
        private List<int> m_intList;
        private List<IAnimal> m_animalList;
        private IReadOnlyList<string> m_stringList;
        private IReadOnlyList<Guid> m_guidList;
        public InterfaceWithMultipleIVector()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
            m_animalList = new List<IAnimal>();
            for (int i = 1; i < 4; i++)
            {
                Animal animal = new Animal();
                animal.SetGreeting("Animal" + i);
                m_animalList.Add(animal);
            }
            List<string> stringList = new List<string>();
            for (int i = 1; i < 8; i++)
            {
                stringList.Add("ViewString" + i);
            }
            m_stringList = stringList;
            List<Guid> guidList = new List<Guid>();
            guidList.Add(new Guid("{8EB82CB5-03D6-49D2-80EE-8583E949B5BF}"));
            guidList.Add(new Guid("{B960A7AC-F275-43FC-B154-91E7ADEEE7AA}"));
            guidList.Add(new Guid("{9F1AF037-BAF9-473B-B8C3-183AB3F5B3CE}"));
            guidList.Add(new Guid("{C4A1CC26-EB02-435B-AE67-18A25D86A787}"));
            m_guidList = guidList;
        }
        int IList<int>.IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        void IList<int>.Insert(int index, int item)
        {
            m_intList.Insert(index, item);
        }
        void IList<int>.RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
        }
        int IList<int>.this[int index]
        {
            get
            {
                return m_intList[index];
            }
            set
            {
                m_intList[index] = value;
            }
        }
        void ICollection<int>.Add(int item)
        {
            m_intList.Add(item);
        }
        void ICollection<int>.Clear()
        {
            m_intList.Clear();
        }
        bool ICollection<int>.Contains(int item)
        {
            return m_intList.Contains(item);
        }
        void ICollection<int>.CopyTo([WriteOnlyArray]int[] array, int arrayIndex)
        {
            m_intList.CopyTo(array, arrayIndex);
        }
        int ICollection<int>.Count
        {
            get { return m_intList.Count; }
        }
        bool ICollection<int>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<int>.Remove(int item)
        {
            return m_intList.Remove(item);
        }
        IEnumerator<int> IEnumerable<int>.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        int IList<IAnimal>.IndexOf(IAnimal item)
        {
            return m_animalList.IndexOf(item);
        }
        void IList<IAnimal>.Insert(int index, IAnimal item)
        {
            m_animalList.Insert(index, item);
        }
        void IList<IAnimal>.RemoveAt(int index)
        {
            m_animalList.RemoveAt(index);
        }
        IAnimal IList<IAnimal>.this[int index]
        {
            get
            {
                return m_animalList[index];
            }
            set
            {
                m_animalList[index] = value;
            }
        }
        void ICollection<IAnimal>.Add(IAnimal item)
        {
            m_animalList.Add(item);
        }
        void ICollection<IAnimal>.Clear()
        {
            m_animalList.Clear();
        }
        bool ICollection<IAnimal>.Contains(IAnimal item)
        {
            return m_animalList.Contains(item);
        }
        void ICollection<IAnimal>.CopyTo([WriteOnlyArray]IAnimal[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }
        int ICollection<IAnimal>.Count
        {
            get { return m_animalList.Count; }
        }
        bool ICollection<IAnimal>.IsReadOnly
        {
            get { return false; }
        }
        bool ICollection<IAnimal>.Remove(IAnimal item)
        {
            return m_animalList.Remove(item);
        }
        IEnumerator<IAnimal> IEnumerable<IAnimal>.GetEnumerator()
        {
            return m_animalList.GetEnumerator();
        }
        string IReadOnlyList<string>.this[int index]
        {
            get { return m_stringList[index]; }
        }
        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            return m_stringList.GetEnumerator();
        }
        Guid IReadOnlyList<Guid>.this[int index]
        {
            get { return m_guidList[index]; }
        }
        IEnumerator<Guid> IEnumerable<Guid>.GetEnumerator()
        {
            return m_guidList.GetEnumerator();
        }
        int IReadOnlyCollection<string>.Count
        {
            get { return m_stringList.Count; }
        }
        int IReadOnlyCollection<Guid>.Count
        {
            get { return m_guidList.Count; }
        }
    }

    public sealed class RCStringMap : IDictionary<string, int>
    {
        Dictionary<string, int> m_dict;
        public RCStringMap()
        {
            m_dict = new Dictionary<string, int>();
            m_dict.Add("by", 2);
            m_dict.Add("Hundred", 7);
            m_dict.Add("Hundred And Fifty", 17);
        }
        public void Add(string key, int value)
        {
            m_dict.Add(key, value);
        }

        public bool ContainsKey(string key)
        {
            return m_dict.ContainsKey(key);
        }

        public ICollection<string> Keys
        {
            get { return m_dict.Keys; }
        }

        public bool Remove(string key)
        {
            return m_dict.Remove(key);
        }

        public bool TryGetValue(string key, out int value)
        {
            return m_dict.TryGetValue(key, out value);
        }

        public ICollection<int> Values
        {
            get { return m_dict.Values; }
        }

        public int this[string key]
        {
            get
            {
                return m_dict[key];
            }
            set
            {
                m_dict[key] = value;
            }
        }

        public void Add(KeyValuePair<string, int> item)
        {
            m_dict.Add(item.Key, item.Value);
        }

        public void Clear()
        {
            m_dict.Clear();
        }

        public bool Contains(KeyValuePair<string, int> item)
        {
            return m_dict.Contains(item);
        }

        public void CopyTo([WriteOnlyArray]KeyValuePair<string, int>[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }

        public int Count
        {
            get { return m_dict.Count; }
        }

        public bool IsReadOnly
        {
            get { return true; }
        }

        public bool Remove(KeyValuePair<string, int> item)
        {
            return m_dict.Remove(item.Key);
        }

        public IEnumerator<KeyValuePair<string, int>> GetEnumerator()
        {
            return (IEnumerator<KeyValuePair<string, int>>)m_dict.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_dict.GetEnumerator();
        }
    }

    public sealed class RCStringMapWithIterable : IDictionary<string, int>, IEnumerable<string>
    {
        Dictionary<string, int> m_dict;

        public RCStringMapWithIterable()
        {
            m_dict = new Dictionary<string, int>();
            m_dict.Add("by", 2);
            m_dict.Add("Hundred", 7);
            m_dict.Add("Hundred And Fifty", 17);
        }
        public void Add(string key, int value)
        {
            m_dict.Add(key, value);
        }

        public bool ContainsKey(string key)
        {
            return m_dict.ContainsKey(key);
        }

        public ICollection<string> Keys
        {
            get { return m_dict.Keys; }
        }

        public bool Remove(string key)
        {
            return m_dict.Remove(key);
        }

        public bool TryGetValue(string key, out int value)
        {
            return m_dict.TryGetValue(key, out value);
        }

        public ICollection<int> Values
        {
            get { return m_dict.Values; }
        }

        public int this[string key]
        {
            get
            {
                return m_dict[key];
            }
            set
            {
                m_dict[key] = value;
            }
        }

        public void Add(KeyValuePair<string, int> item)
        {
            m_dict.Add(item.Key, item.Value);
        }

        public void Clear()
        {
            m_dict.Clear();
        }

        public bool Contains(KeyValuePair<string, int> item)
        {
            return m_dict.Contains(item);
        }

        public void CopyTo([WriteOnlyArray]KeyValuePair<string, int>[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }

        public int Count
        {
            get { return m_dict.Count; }
        }

        public bool IsReadOnly
        {
            get { throw new NotImplementedException(); }
        }

        public bool Remove(KeyValuePair<string, int> item)
        {
            throw new NotImplementedException();
        }

        IEnumerator<KeyValuePair<string, int>> IEnumerable<KeyValuePair<string, int>>.GetEnumerator()
        {
            return (IEnumerator<KeyValuePair<string, int>>)m_dict.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_dict.GetEnumerator();
        }

        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            List<string> list = new List<string>();
            list.Add("Monday");
            list.Add("Tuesday");
            list.Add("Wednesday");
            list.Add("Thursday");
            list.Add("Friday");
            list.Add("Saturday");
            list.Add("Sunday");
            return list.GetEnumerator();
        }
    }

    public sealed class RCStringMapWithDefaultIterable :IEnumerable<string>, IDictionary<string, int>
    {
        Dictionary<string, int> m_dict;

        public RCStringMapWithDefaultIterable()
        {
            m_dict = new Dictionary<string, int>();
            m_dict.Add("by", 2);
            m_dict.Add("Hundred", 7);
            m_dict.Add("Hundred And Fifty", 17);
        }
        public void Add(string key, int value)
        {
            m_dict.Add(key, value);
        }

        public bool ContainsKey(string key)
        {
            return m_dict.ContainsKey(key);
        }

        public ICollection<string> Keys
        {
            get { return m_dict.Keys; }
        }

        public bool Remove(string key)
        {
            return m_dict.Remove(key);
        }

        public bool TryGetValue(string key, out int value)
        {
            return m_dict.TryGetValue(key, out value);
        }

        public ICollection<int> Values
        {
            get { return m_dict.Values; }
        }

        public int this[string key]
        {
            get
            {
                return m_dict[key];
            }
            set
            {
                m_dict[key] = value;
            }
        }

        public void Add(KeyValuePair<string, int> item)
        {
            m_dict.Add(item.Key, item.Value);
        }

        public void Clear()
        {
            m_dict.Clear();
        }

        public bool Contains(KeyValuePair<string, int> item)
        {
            return m_dict.Contains(item);
        }

        public void CopyTo([WriteOnlyArray]KeyValuePair<string, int>[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }

        public int Count
        {
            get { return m_dict.Count; }
        }

        public bool IsReadOnly
        {
            get { throw new NotImplementedException(); }
        }

        public bool Remove(KeyValuePair<string, int> item)
        {
            throw new NotImplementedException();
        }

        IEnumerator<KeyValuePair<string, int>> IEnumerable<KeyValuePair<string, int>>.GetEnumerator()
        {
            return (IEnumerator<KeyValuePair<string, int>>)m_dict.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_dict.GetEnumerator();
        }

        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            List<string> list = new List<string>();
            list.Add("Monday");
            list.Add("Tuesday");
            list.Add("Wednesday");
            list.Add("Thursday");
            list.Add("Friday");
            list.Add("Saturday");
            list.Add("Sunday");
            return list.GetEnumerator();
        }
    }

    //Just test whether ngen this winmd success or not
    //Refer to bug 274026 for more details
    public sealed class NGENClass
    {
        interface INestedInterface
        { }
        public int GetZero()
        {
            IList<INestedInterface> list = new List<INestedInterface>();
            return list.Count;
        }
    }

}
