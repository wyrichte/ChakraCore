using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Collections;
namespace Animals
{
    public sealed class VectorChangedEventArgs : IVectorChangedEventArgs
    {
        private CollectionChange collectionChange;
        private uint index;
        public VectorChangedEventArgs(CollectionChange collectionChange, uint index)
        {
            this.collectionChange = collectionChange;
            this.index = index;
        }
        public CollectionChange CollectionChange
        {
            get
            {
                return this.collectionChange;
            }
        }
        public uint Index
        {
            get
            {
                return this.index;
            }
        }
    }
    public sealed class RCIObservable : IObservableVector<int>
    {
        private List<int> m_intList;
        public RCIObservable()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
        }
        public event VectorChangedEventHandler<int> VectorChanged;
        public int IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        public void Insert(int index, int item)
        {
            m_intList.Insert(index, item);
            if (VectorChanged != null && index >= 0)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)index));
        }
        public void RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
            if (VectorChanged != null && index >= 0)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
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
                if (VectorChanged != null && index >= 0)
                    VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemChanged, (uint)index));
            }
        }
        public void Add(int item)
        {
            m_intList.Add(item);
            if (VectorChanged != null)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)m_intList.Count - 1));
        }
        public void Clear()
        {
            m_intList.Clear();
            if (VectorChanged != null)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.Reset, (uint)m_intList.Count));
        }
        public bool Contains(int item)
        {
            return m_intList.Contains(item);
        }
        public void CopyTo(int[] array, int arrayIndex)
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
            int index = m_intList.IndexOf(item);
            if (index >= 0)
            {
                bool result = m_intList.Remove(index);
                if (VectorChanged != null)
                    VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
                return result;
            }
            return false;
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
    public interface ISingleIObservable : IObservableVector<int>
    {
    }
    public sealed class RCISingleObservable : ISingleIObservable
    {
        private List<int> m_intList;
        public RCISingleObservable()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
        }
        public event VectorChangedEventHandler<int> VectorChanged;
        public int IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        public void Insert(int index, int item)
        {
            m_intList.Insert(index, item);
            if (VectorChanged != null && index >= 0)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)index));
        }
        public void RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
            if (VectorChanged != null && index >= 0)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
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
                if (VectorChanged != null && index >= 0)
                    VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemChanged, (uint)index));
            }
        }
        public void Add(int item)
        {
            m_intList.Add(item);
            if (VectorChanged != null)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)m_intList.Count - 1));
        }
        public void Clear()
        {
            m_intList.Clear();
            if (VectorChanged != null)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.Reset, (uint)m_intList.Count));
        }
        public bool Contains(int item)
        {
            return m_intList.Contains(item);
        }
        public void CopyTo(int[] array, int arrayIndex)
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
            int index = m_intList.IndexOf(item);
            if (index >= 0)
            {
                bool result = m_intList.Remove(index);
                if (VectorChanged != null)
                    VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
                return result;
            }
            return false;
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
    public interface IDoubleIObservable : IObservableVector<int>, IObservableVector<string>
    {
    }
    public sealed class Dummy : IObservableVector<int>
    {
        private List<int> m_intList;
        public Dummy()
        {
            m_intList = new List<int>();
            for (int i = 1; i < 10; i++)
            {
                m_intList.Add(i);
            }
        }
        public event VectorChangedEventHandler<int> VectorChanged;
        public int IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
        public void Insert(int index, int item)
        {
            m_intList.Insert(index, item);
            if (VectorChanged != null && index >= 0)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)index));
        }
        public void RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
            if (VectorChanged != null && index >= 0)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
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
                if (VectorChanged != null && index >= 0)
                    VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemChanged, (uint)index));
            }
        }
        public void Add(int item)
        {
            m_intList.Add(item);
            if (VectorChanged != null)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)m_intList.Count));
        }
        public void Clear()
        {
            m_intList.Clear();
            if (VectorChanged != null)
                VectorChanged(this, new VectorChangedEventArgs(CollectionChange.Reset, (uint)m_intList.Count));
        }
        public bool Contains(int item)
        {
            return m_intList.Contains(item);
        }
        public void CopyTo(int[] array, int arrayIndex)
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
            int index = m_intList.IndexOf(item);
            if (index >= 0)
            {
                bool result = m_intList.Remove(item);
                if (VectorChanged != null)
                    VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
                return result;
            }
            return false;
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
    public sealed class RCIDoubleObservable : IObservableVector<int>, IObservableVector<string>
    {
        private List<int> m_intList;
        private System.Runtime.InteropServices.WindowsRuntime.EventRegistrationTokenTable<VectorChangedEventHandler<int>> m_listIntVectorChangedEvent;
        private System.Runtime.InteropServices.WindowsRuntime.EventRegistrationTokenTable<VectorChangedEventHandler<string>> m_listStringVectorChangedEvent;
        private List<string> m_stringList;
        public RCIDoubleObservable()
        {
            m_listIntVectorChangedEvent = new System.Runtime.InteropServices.WindowsRuntime.EventRegistrationTokenTable<VectorChangedEventHandler<int>>();
            m_listStringVectorChangedEvent = new System.Runtime.InteropServices.WindowsRuntime.EventRegistrationTokenTable<VectorChangedEventHandler<string>>();
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
        event VectorChangedEventHandler<int> IObservableVector<int>.VectorChanged
        {
            add
            {
                return m_listIntVectorChangedEvent.AddEventHandler(value);
            }
            remove
            {
                m_listIntVectorChangedEvent.RemoveEventHandler(value);
            }
        }
        event VectorChangedEventHandler<string> IObservableVector<string>.VectorChanged
        {
            add
            {
                return m_listStringVectorChangedEvent.AddEventHandler(value);
            }
            remove
            {
                m_listStringVectorChangedEvent.RemoveEventHandler(value);
            }
        }

         int IList<int>.IndexOf(int item)
        {
            return m_intList.IndexOf(item);
        }
         void IList<int>.Insert(int index, int item)
        {
            m_intList.Insert(index, item);
            if (m_listIntVectorChangedEvent.InvocationList != null && index >= 0)
                m_listIntVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)index));
        }
        void IList<int>.RemoveAt(int index)
        {
            m_intList.RemoveAt(index);
            if (m_listIntVectorChangedEvent.InvocationList != null && index >= 0)
                m_listIntVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
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
                if (m_listIntVectorChangedEvent.InvocationList != null && index >= 0)
                    m_listIntVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemChanged, (uint)index));
            }
        }
        void ICollection<int>.Add(int item)
        {
            m_intList.Add(item);
            if (m_listIntVectorChangedEvent.InvocationList != null)
                m_listIntVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)m_intList.Count - 1));
        }
        void ICollection<int>.Clear()
        {
            m_intList.Clear();
            if (m_listIntVectorChangedEvent.InvocationList != null)
                m_listIntVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.Reset, (uint)m_intList.Count));
        }
        bool ICollection<int>.Contains(int item)
        {
            return m_intList.Contains(item);
        }
        void ICollection<int>.CopyTo(int[] array, int arrayIndex)
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
            int index = m_intList.IndexOf(item);
            if (index >= 0)
            {
                bool result = m_intList.Remove(item);
                if (m_listIntVectorChangedEvent.InvocationList != null)
                    m_listIntVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
                return result;
            }
            return false;
        }
        IEnumerator<int> IEnumerable<int>.GetEnumerator()
        {
            return m_intList.GetEnumerator();
        }
        

        public int IndexOf(string item)
        {
            return m_stringList.IndexOf(item);
        }
        public void Insert(int index, string item)
        {
            m_stringList.Insert(index, item);
            if (m_listStringVectorChangedEvent.InvocationList != null && index >= 0)
                m_listStringVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)index));
        }
        public void RemoveAt(int index)
        {
            m_stringList.RemoveAt(index);
        }
        public string this[int index]
        {
            get
            {
                return m_stringList[index];
            }
            set
            {
                m_stringList[index] = value;
                if (m_listStringVectorChangedEvent.InvocationList != null && index >= 0)
                    m_listStringVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemChanged, (uint)index));
            }
        }
        public void Add(string item)
        {
            m_stringList.Add(item);
            if (m_listStringVectorChangedEvent.InvocationList != null)
                m_listStringVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)m_stringList.Count-1));
        }
        public void Clear()
        {
            m_stringList.Clear();
        }
        public bool Contains(string item)
        {
            return m_stringList.Contains(item);
        }
        public void CopyTo(string[] array, int arrayIndex)
        {
            m_stringList.CopyTo(array, arrayIndex);
        }
        public int Count
        {
            get { return m_stringList.Count; }
        }
        public bool IsReadOnly
        {
            get { return false; }
        }
        public bool Remove(string item)
        {
            int index = m_stringList.IndexOf(item);
            if (index >= 0)
            {
                bool result = m_stringList.Remove(item);
                if (m_listStringVectorChangedEvent.InvocationList != null)
                    m_listStringVectorChangedEvent.InvocationList(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
                return result;
            }
            return false;
        }
         IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            return m_stringList.GetEnumerator();
        }
         IEnumerator IEnumerable.GetEnumerator()
        {
            return m_stringList.GetEnumerator();
        }
    }
    class MapChangedEventArgs<TKey> : IMapChangedEventArgs<TKey>
    {
        private TKey m_Key;
        private CollectionChange m_CollectionChange;
        public MapChangedEventArgs(TKey key, CollectionChange collectionChange)
        {
            m_Key = key;
            m_CollectionChange = collectionChange;
        }
        public CollectionChange CollectionChange
        {
            get { return m_CollectionChange; }
        }

        public TKey Key
        {
            get { return m_Key; }
        }
    }
    /*
    class ObservableHashMap<TKey, TValue> : IObservableMap<TKey, TValue>
    {
        private Dictionary<TKey, TValue> m_dict;
        public ObservableHashMap()
        {
            m_dict = new Dictionary<TKey, TValue>();
        }
        public event MapChangedEventHandler<TKey, TValue> MapChanged;
        public void Add(TKey key, TValue value)
        {
            m_dict.Add(key, value);
            if (MapChanged != null)
            {
                MapChanged(this, new MapChangedEventArgs<TKey>(key, CollectionChange.ItemInserted));
            }
        }
        public bool ContainsKey(TKey key)
        {
            return m_dict.ContainsKey(key);
        }
        public ICollection<TKey> Keys
        {
            get { return m_dict.Keys; }
        }
        public bool Remove(TKey key)
        {
            bool bResult = m_dict.Remove(key);
            if (bResult && MapChanged != null)
            {
                MapChanged(this, new MapChangedEventArgs<TKey>(key, CollectionChange.ItemRemoved));
            }
            return bResult;
        }
        public bool TryGetValue(TKey key, out TValue value)
        {
            return m_dict.TryGetValue(key, out value);
        }
        public ICollection<TValue> Values
        {
            get { return m_dict.Values; }
        }
        public TValue this[TKey key]
        {
            get
            {
                return m_dict[key];
            }
            set
            {
                CollectionChange cc;
                if (ContainsKey(key))
                {
                    cc = CollectionChange.ItemChanged;
                }
                else
                {
                    cc = CollectionChange.ItemInserted;
                }
                m_dict[key] = value;

                if (MapChanged != null)
                {
                    MapChanged(this, new MapChangedEventArgs<TKey>(key, cc));
                }
            }
        }
        public void Add(KeyValuePair<TKey, TValue> item)
        {
            m_dict.Add(item.Key, item.Value);

            if (MapChanged != null)
            {
                MapChanged(this, new MapChangedEventArgs<TKey>(item.Key, CollectionChange.ItemInserted));
            }
        }
        public void Clear()
        {
            m_dict.Clear();
            
            if (MapChanged != null)
            {
                if (typeof(TKey) == typeof(string))
                {
                    MapChanged(this, new MapChangedEventArgs<TKey>((TKey)string.Empty, CollectionChange.Reset));
                }
                else
                {
                    MapChanged(this, new MapChangedEventArgs<TKey>(default(TKey), CollectionChange.Reset));
                }
            }
        }
        public bool Contains(KeyValuePair<TKey, TValue> item)
        {
            if (m_dict.ContainsKey(item.Key))
            {
                if (item.Value.Equals(m_dict[item.Key]))
                    return true;
            }
            return false;
        }
        public void CopyTo(KeyValuePair<TKey, TValue>[] array, int arrayIndex)
        {

        }
        public int Count
        {
            get { return m_dict.Count; }
        }
        public bool IsReadOnly
        {
            get { return true; }
        }
        public bool Remove(KeyValuePair<TKey, TValue> item)
        {
            if (Contains(item))
            {
                m_dict.Remove(item.Key);
                if (MapChanged != null)
                {
                    MapChanged(this, new MapChangedEventArgs<TKey>(item.Key, CollectionChange.ItemRemoved));
                }
                return true;
            }
            return false;
        }
        public IEnumerator<KeyValuePair<TKey, TValue>> GetEnumerator()
        {
            return (IEnumerator<KeyValuePair<TKey, TValue>>)m_dict.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_dict.GetEnumerator();
        }
    }*/

    class ObservableHashMapStringInt: IObservableMap<string, int>
    {
        private Dictionary<string, int> m_dict;
        public ObservableHashMapStringInt()
        {
            m_dict = new Dictionary<string, int>();
        }
        public event MapChangedEventHandler<string, int> MapChanged;
        public void Add(string key, int value)
        {
            m_dict.Add(key, value);
            if (MapChanged != null)
            {
                MapChanged(this, new MapChangedEventArgs<string>(key, CollectionChange.ItemInserted));
            }
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
            bool bResult = m_dict.Remove(key);
            if (bResult && MapChanged != null)
            {
                MapChanged(this, new MapChangedEventArgs<string>(key, CollectionChange.ItemRemoved));
            }
            return bResult;
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
                CollectionChange cc;
                if (ContainsKey(key))
                {
                    cc = CollectionChange.ItemChanged;
                }
                else
                {
                    cc = CollectionChange.ItemInserted;
                }
                m_dict[key] = value;

                if (MapChanged != null)
                {
                    MapChanged(this, new MapChangedEventArgs<string>(key, cc));
                }
            }
        }
        public void Add(KeyValuePair<string, int> item)
        {
            m_dict.Add(item.Key, item.Value);

            if (MapChanged != null)
            {
                MapChanged(this, new MapChangedEventArgs<string>(item.Key, CollectionChange.ItemInserted));
            }
        }
        public void Clear()
        {
            m_dict.Clear();
            
            if (MapChanged != null)
            {

                    MapChanged(this, new MapChangedEventArgs<string>(string.Empty, CollectionChange.Reset));
                
            }
        }
        public bool Contains(KeyValuePair<string, int> item)
        {
            if (m_dict.ContainsKey(item.Key))
            {
                if (item.Value.Equals(m_dict[item.Key]))
                    return true;
            }
            return false;
        }
        public void CopyTo(KeyValuePair<string, int>[] array, int arrayIndex)
        {

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
            if (Contains(item))
            {
                m_dict.Remove(item.Key);
                if (MapChanged != null)
                {
                    MapChanged(this, new MapChangedEventArgs<string>(item.Key, CollectionChange.ItemRemoved));
                }
                return true;
            }
            return false;
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

    class MyObservableVector<T> : IObservableVector<T>
    {
        public event Windows.Foundation.Collections.VectorChangedEventHandler<T> VectorChanged;
        private List<T> m_list;
        public MyObservableVector()
        {
            m_list = new List<T>();
        }
        public int IndexOf(T item)
        {
            throw new NotImplementedException();
        }
        public void Insert(int index, T item)
        {
            m_list.Insert(index, item);
            if (VectorChanged != null) VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemInserted, (uint)index));
        }
        public void RemoveAt(int index)
        {
            m_list.RemoveAt(index);
            if (VectorChanged != null) VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemRemoved, (uint)index));
        }
        public T this[int index]
        {
            get
            {
                return m_list[index];
            }
            set
            {
                m_list[index] = value;
                if (VectorChanged != null) VectorChanged(this, new VectorChangedEventArgs(CollectionChange.ItemChanged, (uint)index));
            }
        }
        public void Add(T item)
        {
            m_list.Add(item);
        }
        public void Clear()
        {
            m_list.Clear();
        }
        public bool Contains(T item)
        {
            return m_list.Contains(item);
        }
        public void CopyTo(T[] array, int arrayIndex)
        {
            m_list.CopyTo(array, arrayIndex);
        }
        public int Count
        {
            get { return m_list.Count; }
        }
        public bool IsReadOnly
        {
            get { return false; }
        }
        public bool Remove(T item)
        {
            return m_list.Remove(item);
        }
        public IEnumerator<T> GetEnumerator()
        {
            return m_list.GetEnumerator();
        }
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            throw new NotImplementedException();
        }
    }
}
