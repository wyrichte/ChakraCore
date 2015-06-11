using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Windows.Foundation;
using Windows.Foundation.Collections;
using System.Runtime.InteropServices.WindowsRuntime;
namespace Animals
{
    public sealed class PropertyValueTests
    {
        private object m_empty;
        public void IsSameDelegate(DelegateWithOutParam_HSTRING inValue1, DelegateWithOutParam_HSTRING inValue2, out bool isSame)
        {
            isSame = inValue1.Equals(inValue2);
        }
        public void ReceiveAnimalArray(out Animal[] outValue)
        {
            outValue = new Animal[3];
            outValue[0] = new Animal();
            outValue[0].SetGreeting("Animal1");
            outValue[1] = null;
            outValue[2] = new Animal();
            outValue[2].SetGreeting("Animal3");
        }
        public void ReceiveBooleanArray(out bool[] outValue)
        {
            outValue = new bool[4];
            outValue[0] = true;
            outValue[1] = false;
            outValue[2] = true;
            outValue[3] = true;
        }
        public void ReceiveChar16Array(out char[] outValue)
        {
            outValue = new char[10];
            outValue[0] = 'P';
            outValue[1] = 'r';
            outValue[2] = 'o';
            outValue[3] = 'j';
            outValue[4] = 'e';
            outValue[5] = 'c';
            outValue[6] = 't';
            outValue[7] = 'i';
            outValue[8] = 'o';
            outValue[9] = 'n';
        }
        public void ReceiveDateArray(out DateTimeOffset[] outValue)
        {
            outValue = new DateTimeOffset[2];
            outValue[0] = new DateTimeOffset(1641, 2, 1, 0, 0, 0, new TimeSpan());
            outValue[1] = new DateTimeOffset(1600, 12, 31, 0, 0, 0, new TimeSpan());
        }
        public void ReceiveDoubleArray(out double[] outValue)
        {
            outValue = new double[2];
            outValue[0] = 13.4;
            outValue[1] = 56.8;
        }
        public void ReceiveEnumArray(out Phylum[] outValue)
        {
            outValue = new Phylum[4];
            outValue[0] = Phylum.Entoprocta;
            outValue[1] = Phylum.Mollusca;
            outValue[2] = Phylum.Arthropoda;
            outValue[3] = Phylum.Orthonectida;
        }
        public void ReceiveFishArray(out IFish[] outValue)
        {
            outValue = new Fish[2];
            outValue[0] = new Fish();
            outValue[0].Name = "Nemo";
            outValue[1] = new Fish();
            outValue[1].Name = "Dori";
        }
        public void ReceiveFloatArray(out float[] outValue)
        {
            outValue = new float[4];
            outValue[0] = 78.3f;
            outValue[1] = 67.9f;
            outValue[2] = 99.4f;
            outValue[3] = -32.2f;
        }
        public void ReceiveGuidArray(out Guid[] outValue)
        {
            outValue = new Guid[3];
            outValue[0] = new Guid("3463F772-274F-449D-8B25-822742C2B3FF");
            outValue[1] = new Guid("3B3B41BC-96E3-43FE-8EC1-7E3DDE4F776C");
            outValue[2] = new Guid("C1A5F085-740C-4991-9342-60B1E471BEB9");
        }
        public void ReceiveInspectableArray(out object[] outValue)
        {
            Animals.Animal[] animalArray = new Animals.Animal[3];
            animalArray[0] = new Animals.Animal();
            animalArray[0].SetGreeting("Animal1");
            animalArray[1] = null;
            animalArray[2] = new Animals.Animal();
            animalArray[2].SetGreeting("Animal3");
            outValue = animalArray;
        }
        public void ReceiveInt16Array(out System.Int16[] outValue)
        {
            outValue = new System.Int16[4];
            outValue[0] = 10;
            outValue[1] = -20;
            outValue[2] = 30;
            outValue[3] = -40;
        }
        public void ReceiveInt32Array(out int[] outValue)
        {
            outValue = new int[4];
            outValue[0] = 1000;
            outValue[1] = -2000;
            outValue[2] = 3000;
            outValue[3] = -4000;
        }
        public void ReceiveInt64Array(out long[] outValue)
        {
            outValue = new long[4];
            outValue[0] = 100000;
            outValue[1] = -200000;
            outValue[2] = 300000;
            outValue[3] = -400000;
        }
        public void ReceiveJSDelegateArray(DelegateWithOutParam_HSTRING delegate1, DelegateWithOutParam_HSTRING delegate2, out DelegateWithOutParam_HSTRING[] outValue)
        {
            outValue = new DelegateWithOutParam_HSTRING[2];
            outValue[0] = delegate1;
            outValue[1] = delegate2;
        }
        public void ReceiveMapOfStructAndVector(out IDictionary<Dimensions, IList<string>> outValue)
        {
            Dimensions dimension1 = new Dimensions();
            dimension1.Length = 100;
            dimension1.Width = 100;
            List<string> list1 = new List<string>();
            list1.Add("Hundred");
            list1.Add("by");
            list1.Add("Hundred");
            Dimensions dimension2 = new Dimensions();
            dimension2.Length = 150;
            dimension2.Width = 100;
            List<string> list2 = new List<string>();
            list2.Add("Hundred And Fifty");
            list2.Add("by");
            list2.Add("Hundred");
            outValue = new Dictionary<Dimensions, IList<string>>();
            outValue.Add(dimension1, list1);
            outValue.Add(dimension2, list2);
        }
        public void ReceiveMapOfStructAndVector_InspectableOut(out object outValue)
        {
            IDictionary<Dimensions, IList<string>> dict;
            ReceiveMapOfStructAndVector(out dict);
            outValue = dict;
        }
        public void ReceivePointArray(out Windows.Foundation.Point[] outValue)
        {
            outValue = new Windows.Foundation.Point[3];
            outValue[0] = new Windows.Foundation.Point(10, 40);
            outValue[1] = new Windows.Foundation.Point(30, 50);
            outValue[2] = new Windows.Foundation.Point(100, 50);
        }
        public void ReceiveRectArray(out Windows.Foundation.Rect[] outValue)
        {
            outValue = new Windows.Foundation.Rect[3];
            outValue[0] = new Windows.Foundation.Rect(10, 40, 10, 40);
            outValue[1] = new Windows.Foundation.Rect(30, 50, 30, 50);
            outValue[2] = new Windows.Foundation.Rect(100, 50, 100, 50);
        }
        public void ReceiveSizeArray(out Windows.Foundation.Size[] outValue)
        {
            outValue = new Windows.Foundation.Size[3];
            outValue[0] = new Windows.Foundation.Size(10, 40);
            outValue[1] = new Windows.Foundation.Size(30, 50);
            outValue[2] = new Windows.Foundation.Size(100, 50);
        }
        public void ReceiveStringArray(out string[] outValue)
        {
            outValue = new string[5];
            outValue[0] = "Javascript";
            outValue[1] = "is";
            outValue[2] = "present";
            outValue[3] = "and";
            outValue[4] = "future";
        }
        public void ReceiveStructArray(out Dimensions[] outValue)
        {
            outValue = new Dimensions[2];
            Dimensions d1;
            d1.Length = 40;
            d1.Width = 40;
            outValue[0] = d1;
            Dimensions d2;
            d2.Length = 100;
            d2.Width = 40;
            outValue[0] = d2;
        }
        public void ReceiveTimeSpanArray(out TimeSpan[] outValue)
        {
            outValue = new TimeSpan[3];
            outValue[0] = new TimeSpan(1265068800000 * 10000 + 5);
            outValue[1] = new TimeSpan(0);
            outValue[2] = new TimeSpan(60);
        }
        public void ReceiveUInt16Array(out System.UInt16[] outValue)
        {
            outValue = new System.UInt16[2];
            outValue[0] = 10;
            outValue[1] = 20;
        }
        public void ReceiveUInt32Array(out uint[] outValue)
        {
            outValue = new UInt32[2];
            outValue[0] = 1000;
            outValue[1] = 2000;
        }
        public void ReceiveUInt64Array(out ulong[] outValue)
        {
            outValue = new ulong[2];
            outValue[0] = 100000;
            outValue[1] = 200000;
        }
        public void ReceiveUInt8Array(out byte[] outValue)
        {
            outValue = new byte[4];
            outValue[0] = 0x00;
            outValue[1] = 0x02;
            outValue[2] = 0x20;
            outValue[3] = 0x22;
        }
        public void ReceiveVectorArray(out IList<int>[] outValue)
        {
            outValue = new List<int>[2];
            outValue[0] = new List<int>();
            outValue[1] = new List<int>();
            for (int i = 1; i <= 4; i++)
            {
                outValue[0].Add(i);
                outValue[1].Add(i);
            }
            outValue[0].Add(5);
        }
        public void ReceiveVectorOfDate(out IList<DateTimeOffset> outValue)
        {
            outValue = new List<DateTimeOffset>();
            outValue.Add(new DateTimeOffset(1265068800000 * 10000 + 5, new TimeSpan()));
            outValue.Add(new DateTimeOffset(0, new TimeSpan()));
        }
        public void ReceiveVectorOfDate_InspectableOut(out object outValue)
        {
            IList<DateTimeOffset> datetime;
            ReceiveVectorOfDate(out datetime);
            outValue = datetime;
        }
        public void ReceiveVectorOfDelegate(out IList<DelegateWithOutParam_HSTRING> outValue)
        {
            outValue = new List<DelegateWithOutParam_HSTRING>();
            outValue.Add(CallBackMethods.AnimalDelegateWithOutParamString);
            outValue.Add(CallBackMethods.AnimalDelegateWithOutParamString);
        }
        public void ReceiveVectorOfDelegate_InspectableOut(out object outValue)
        {
            IList<DelegateWithOutParam_HSTRING> listOfDelegate;
            ReceiveVectorOfDelegate(out listOfDelegate);
            outValue = listOfDelegate;
        }
        public void ReceiveVectorOfEnum(out IList<Phylum> outValue)
        {
            outValue = new List<Phylum>();
            outValue.Add(Phylum.Entoprocta);
            outValue.Add(Phylum.Mollusca);
            outValue.Add(Phylum.Arthropoda);
            outValue.Add(Phylum.Orthonectida);
        }
        public void ReceiveVectorOfEnum_InspectableOut(out object outValue)
        {
            IList<Phylum> listOfPhylum;
            ReceiveVectorOfEnum(out listOfPhylum);
            outValue = listOfPhylum;
        }
        public void ReceiveVectorOfEventRegistration(out IList<System.Runtime.InteropServices.WindowsRuntime.EventRegistrationToken> outValue)
        {
            outValue = new List<System.Runtime.InteropServices.WindowsRuntime.EventRegistrationToken>();
            outValue.Add(new System.Runtime.InteropServices.WindowsRuntime.EventRegistrationToken());
        }
        public void ReceiveVectorOfEventRegistration_InspectableOut(out object outValue)
        {
            IList<System.Runtime.InteropServices.WindowsRuntime.EventRegistrationToken> listOfEventRegistration;
            ReceiveVectorOfEventRegistration(out listOfEventRegistration);
            outValue = listOfEventRegistration;
        }
        public void ReceiveVectorOfGuid(out IList<Guid> outValue)
        {
            outValue = new List<Guid>();
            outValue.Add(new Guid("3463F772-274F-449D-8B25-822742C2B3FF"));
            outValue.Add(new Guid("3463F772-274F-449D-8B25-822742C2B3FF"));
            outValue.Add(new Guid("3463F772-274F-449D-8B25-822742C2B3FF"));
        }
        public void ReceiveVectorOfGuid_InspectableOut(out object outValue)
        {
            IList<Guid> listOfGuid;
            ReceiveVectorOfGuid(out listOfGuid);
            outValue = listOfGuid;
        }
        public void ReceiveVectorOfRCObservableVector(out IList<RCIObservable> outValue)
        {
            outValue = new List<RCIObservable>();
            outValue.Add(new RCIObservable());
            outValue.Add(new RCIObservable());
        }
        public void ReceiveVectorOfRCObservableVector_InspectableOut(out object outValue)
        {
            IList<RCIObservable> listOfRCIObservable;
            ReceiveVectorOfRCObservableVector(out listOfRCIObservable);
            outValue = listOfRCIObservable;
        }
        public void ReceiveVectorOfStruct(out IList<Dimensions> outValue)
        {
            outValue = new List<Dimensions>();
            Dimensions d1;
            d1.Length = 100;
            d1.Width = 100;
            outValue.Add(d1);
            Dimensions d2;
            d2.Length = 150;
            d2.Width = 100;
            outValue.Add(d2);
        }
        public void ReceiveVectorOfStruct_InspectableOut(out object outValue)
        {
            IList<Dimensions> list;
            ReceiveVectorOfStruct(out list);
            outValue = list;
        }
        public void ReceiveVectorOfTimeSpan(out IList<TimeSpan> outValue)
        {
            outValue = new List<TimeSpan>();
            outValue.Add(new TimeSpan(1265068800000 * 10000 + 5));
            outValue.Add(new TimeSpan(0));
            outValue.Add(new TimeSpan(60));
        }
        public void ReceiveVectorOfTimeSpan_InspectableOut(out object outValue)
        {
            IList<TimeSpan> list;
            ReceiveVectorOfTimeSpan(out list);
            outValue = list;
        }
        public void ReceiveVectorOfVector(out IList<IList<int>> outValue)
        {
            outValue = new List<IList<int>>();
            List<int> list1 = new List<int>();
            List<int> list2 = new List<int>();
            for (int i = 1; i < 5; i++)
            {
                list1.Add(i);
            }
            list1.Add(5);
            outValue.Add(list1);
            outValue.Add(list2);
        }
        public void ReceiveVectorOfVector_InspectableOut(out object outValue)
        {
            IList<IList<int>> list;
            ReceiveVectorOfVector(out list);
            outValue = list;
        }
        internal class MyAsync : Windows.Foundation.IAsyncOperation<bool>
        {
            public AsyncOperationCompletedHandler<bool> Completed
            {
                get
                {
                    throw new NotImplementedException();
                }
                set
                {
                    throw new NotImplementedException();
                }
            }

            public bool GetResults()
            {
                return false;
            }

            public void Cancel()
            {

            }

            public void Close()
            {

            }

            public Exception ErrorCode
            {
                get { throw new NotImplementedException(); }
            }

            public uint Id
            {
                get { throw new NotImplementedException(); }
            }

            public AsyncStatus Status
            {
                get { throw new NotImplementedException(); }
            }
        }
        public void ReceiveVectorOfAsyncInfo(out object outValue)
        {
            IList<IAsyncInfo> list = new List<IAsyncInfo>();
            list.Add(new MyAsync());
            outValue = list;

        }
        public void ReceiveVectorOfAsyncInfo_InspectableOut(out object outValue)
        {
            IList<IAsyncInfo> list = new List<IAsyncInfo>();
            list.Add(new MyAsync());
            outValue = list;
        }

        public void ReceiveWinrtDelegateArray(out DelegateWithOutParam_HSTRING[] outValue)
        {
            outValue = new DelegateWithOutParam_HSTRING[2];
            outValue[0] = CallBackMethods.AnimalDelegateWithOutParamString;
            outValue[1] = CallBackMethods.AnimalDelegateWithOutParamString;
        }
        public void TestArray_InspectableIn(object inValue, out bool isValidType, out object[] outValue)
        {
            if (inValue != null)
                isValidType = true;
            else
                isValidType = false;
            if (inValue.GetType().IsArray)
            {
                Array arr = (Array)inValue;
                outValue = new object[arr.Length];
                for (int i = 0; i < arr.Length; i++)
                {
                    outValue[i] = arr.GetValue(i);
                }
            }
            else
                outValue = null;
        }
        public void TestArray_InspectableOut([ReadOnlyArray]object[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestAnimalArray_InspectableOut([ReadOnlyArray] Animal[] inValue, out object outValue)
        {
            outValue = inValue;

        }
        public void TestFishArray_InspectableOut([ReadOnlyArray] IFish[] inValue, out object outValue)
        {
            outValue = inValue;

        }
        public void TestVectorArray_InspectableOut([ReadOnlyArray]IList<int>[] inValue, out object outValue)
        {
            outValue = inValue;

        }

        public void TestBooleanArray_InspectableIn(object inValue, out bool isValidType, out bool[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(bool) && inValue.GetType().IsArray)
            {
                isValidType = true;
            }
            else
            {
                isValidType = false;
            }
            outValue = (bool[])inValue;
        }
        public void TestBooleanArray_InspectableOut([ReadOnlyArray]bool[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoolean_InspectableIn(object inValue, out bool isValidType, out bool outValue)
        {
            if (inValue.GetType() == typeof(bool))
            {
                isValidType = true;
            }
            else
            {
                isValidType = false;
            }
            outValue = (bool)inValue;
        }
        public void TestBoolean_InspectableOut(bool inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxIVector_InspectableOut(IList<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxInspectable_InspectableOut(object inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxIterable_InspectableOut(IEnumerable<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxedNull_InspectableOut(out object outValue)
        {
            outValue = null;
        }
        public void TestChar16Array_InspectableIn(object inValue, out bool isValidType, out char[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(char) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (char[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestChar16Array_InspectableOut([ReadOnlyArray]char[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestChar16_InspectableOut(char inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDateArray_InspectableIn(object inValue, out bool isValidType, out DateTimeOffset[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(DateTimeOffset) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (DateTimeOffset[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestDateArray_InspectableOut([ReadOnlyArray]DateTimeOffset[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDate_InspectableIn(object inValue, out bool isValidType, out DateTimeOffset outValue)
        {
            if (inValue.GetType() == typeof(DateTimeOffset))
            {
                isValidType = true;
                outValue = (DateTimeOffset)inValue;
            }
            else
            {
                isValidType = false;
                outValue = new DateTimeOffset();
            }
        }
        public void TestDate_InspectableOut(DateTimeOffset inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDelegateArray_InspectableIn(object inValue, out bool isValidType, out DelegateWithOutParam_HSTRING[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(DelegateWithOutParam_HSTRING) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (DelegateWithOutParam_HSTRING[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestDimensionsArray_InspectableIn(object inValue, out bool isValidType, out Dimensions[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(Dimensions) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (Dimensions[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestDimensions_InspectableOut(Dimensions? inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDoubleArray_InspectableIn(object inValue, out bool isValidType, out double[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(double) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (double[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestDoubleArray_InspectableOut([ReadOnlyArray]double[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDouble_InspectableOut(double inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestFloatArray_InspectableIn(object inValue, out bool isValidType, out float[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(float) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (float[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestFloatArray_InspectableOut([ReadOnlyArray]float[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestFloat_InspectableOut(float inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestGuidArray_InspectableIn(object inValue, out bool isValidType, out Guid[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(Guid) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (Guid[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestGuidArray_InspectableOut([ReadOnlyArray]Guid[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestGuid_InspectableOut(Guid inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestIVectorView_InspectableOut(IReadOnlyList<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestIVector_InspectableOut(IList<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInspectable_InspectableIn(object inValue, out bool isValidType, out object outValue)
        {
            if (inValue != null)
                isValidType = true;
            else
                isValidType = false;
            outValue = inValue;
        }
        public void TestInspectable_InspectableOut(object inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt16Array_InspectableIn(object inValue, out bool isValidType, out System.Int16[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(System.Int16) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (System.Int16[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestInt16Array_InspectableOut([ReadOnlyArray]System.Int16[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt16_InspectableOut(System.Int16 inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt32Array_InspectableIn(object inValue, out bool isValidType, out int[] outValue)
        {
            if (inValue.GetType().GetElementType() == typeof(int) && inValue.GetType().IsArray)
            {
                isValidType = true;
                outValue = (int[])inValue;
            }
            else
            {
                isValidType = false;
                outValue = null;
            }
        }
        public void TestInt32Array_InspectableOut([ReadOnlyArray]int[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt32_InspectableOut(int inValue, out object outValue)
        {
            outValue = inValue;
        }
        private void Test_InspectableHelper<T>(object inValue, out bool isValidType, out T outValue)
        {
            if (inValue.GetType() == typeof(T))
            {
                isValidType = true;
                outValue = (T)inValue;
            }
            else
            {
                isValidType = false;
                outValue = default(T);
            }
        }
        public void TestInt64Array_InspectableIn(object inValue, out bool isValidType, out long[] outValue)
        {
            Test_InspectableHelper<long[]>(inValue, out isValidType, out outValue);
        }
        public void TestInt64Array_InspectableOut([ReadOnlyArray]long[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt64_InspectableOut(long inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestIterable_InspectableOut(IEnumerable<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestNull_InspectableIn(object inValue, out bool isValidType, out object outValue)
        {
            if (inValue == null)
                isValidType = true;
            else
                isValidType = false;
            outValue = null;
        }
        public void TestNull_InspectableOut(out object outValue)
        {
            outValue = null;
        }
        public void TestNumber_InspectableIn(object inValue, out bool isValidType, out double outValue)
        {
            Test_InspectableHelper<double>(inValue, out isValidType, out outValue);
        }
        public void TestPointArray_InspectableIn(object inValue, out bool isValidType, out Windows.Foundation.Point[] outValue)
        {
            Test_InspectableHelper<Windows.Foundation.Point[]>(inValue, out isValidType, out outValue);
        }
        public void TestPointArray_InspectableOut([ReadOnlyArray]Windows.Foundation.Point[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestPoint_InspectableOut(Windows.Foundation.Point inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRectArray_InspectableIn(object inValue, out bool isValidType, out Windows.Foundation.Rect[] outValue)
        {
            Test_InspectableHelper<Windows.Foundation.Rect[]>(inValue, out isValidType, out outValue);
        }
        public void TestEnumArray_InspectableIn(object inValue, out bool isValidType, out Phylum[] outValue)
        {
            Test_InspectableHelper<Phylum[]>(inValue, out isValidType, out outValue);
        }
        public void TestRectArray_InspectableOut([ReadOnlyArray]Windows.Foundation.Rect[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRect_InspectableOut(Windows.Foundation.Rect inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestEnum_InspectableOut(Phylum inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRCPV1_InspectableOut(out object outValue)
        {
            Animals.Dimensions dimension = new Dimensions();
            dimension.Length = 100;
            dimension.Width = 20;
            outValue = dimension;
        }
        public void TestRCPV2_InspectableOut(out object outValue)
        {
            Animals.Dimensions dimension = new Dimensions();
            dimension.Length = 100;
            dimension.Width = 20;
            outValue = dimension;
        }
        public void TestRCPV3_InspectableOut(out object outValue)
        {
            Animals.Dimensions dimension = new Dimensions();
            dimension.Length = 100;
            dimension.Width = 20;
            outValue = dimension;
        }
        public void TestRCPV4_InspectableOut(out object outValue)
        {
            char? d = 'D';
            outValue = d;
        }
        public void TestRCPV5_InspectableOut(out object outValue)
        {
            char? e = 'E';
            outValue = e;
        }
        public void TestRCPV6_InspectableOut(out object outValue)
        {
            char? f = 'F';
            outValue = f;
        }
        public void TestSizeArray_InspectableIn(object inValue, out bool isValidType, out Windows.Foundation.Size[] outValue)
        {
            Test_InspectableHelper<Windows.Foundation.Size[]>(inValue, out isValidType, out outValue);
        }
        public void TestSizeArray_InspectableOut([ReadOnlyArray]Windows.Foundation.Size[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestSize_InspectableOut(Windows.Foundation.Size inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestStringArray_InspectableIn(object inValue, out bool isValidType, out string[] outValue)
        {
            Test_InspectableHelper<string[]>(inValue, out isValidType, out outValue);
        }
        public void TestStringArray_InspectableOut([ReadOnlyArray]string[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestString_InspectableIn(object inValue, out bool isValidType, out string outValue)
        {
            Test_InspectableHelper<string>(inValue, out isValidType, out outValue);
        }
        public void TestString_InspectableOut(string inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpanArray_InspectableIn(object inValue, out bool isValidType, out TimeSpan[] outValue)
        {
            Test_InspectableHelper<TimeSpan[]>(inValue, out isValidType, out outValue);
        }
        public void TestTimeSpanArray_InspectableOut([ReadOnlyArray]TimeSpan[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpan_InspectableOut(TimeSpan inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt16Array_InspectableIn(object inValue, out bool isValidType, out System.UInt16[] outValue)
        {
            Test_InspectableHelper<System.UInt16[]>(inValue, out isValidType, out outValue);
        }
        public void TestUInt16Array_InspectableOut([ReadOnlyArray]System.UInt16[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt16_InspectableOut(System.UInt16 inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt32Array_InspectableIn(object inValue, out bool isValidType, out uint[] outValue)
        {
            Test_InspectableHelper<uint[]>(inValue, out isValidType, out outValue);
        }
        public void TestUInt32Array_InspectableOut([ReadOnlyArray]uint[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt32_InspectableOut(uint inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt64Array_InspectableIn(object inValue, out bool isValidType, out ulong[] outValue)
        {
            Test_InspectableHelper<ulong[]>(inValue, out isValidType, out outValue);
        }
        public void TestUInt64Array_InspectableOut([ReadOnlyArray]ulong[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt64_InspectableOut(ulong inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt8Array_InspectableIn(object inValue, out bool isValidType, out byte[] outValue)
        {
            Test_InspectableHelper<byte[]>(inValue, out isValidType, out outValue);
        }
        public void TestUInt8Array_InspectableOut([ReadOnlyArray]byte[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt8_InspectableOut(byte inValue, out object outValue)
        {
            outValue = inValue;
        }
        #region "PropertyValue"
        public void TestArray_IPropertyValueIn(object inValue, out bool isValidType, out object[] outValue)
        {
            isValidType = true;
            outValue = (object[])inValue;
        }
        public void TestArray_IPropertyValueOut([ReadOnlyArray] object[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBooleanArray_IPropertyValueIn(object inValue, out bool isValidType, out bool[] outValue)
        {
            isValidType = true;
            outValue = (bool[])inValue;
        }
        public void TestBooleanArray_IPropertyValueOut([ReadOnlyArray]bool[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoolean_IPropertyValueIn(object inValue, out bool isValidType, out bool outValue)
        {
            isValidType = true;
            outValue = (bool)inValue;
        }
        public void TestBoolean_IPropertyValueOut(bool inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxInspectable_IPropertyValueOut(object inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxedNull_IPropertyValueOut(out object outValue)
        {
            outValue = null;
        }
        public void TestChar16Array_IPropertyValueIn(object inValue, out bool isValidType, out char[] outValue)
        {
            isValidType = true;
            outValue = (char[])inValue;
        }
        public void TestChar16Array_IPropertyValueOut([ReadOnlyArray]char[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestChar16_IPropertyValueOut(char inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDateArray_IPropertyValueIn(object inValue, out bool isValidType, out DateTimeOffset[] outValue)
        {
            isValidType = true;
            outValue = (DateTimeOffset[])inValue;
        }
        public void TestDateArray_IPropertyValueOut([ReadOnlyArray]DateTimeOffset[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDate_IPropertyValueIn(object inValue, out bool isValidType, out DateTimeOffset outValue)
        {
            isValidType = true;
            outValue = (DateTimeOffset)inValue;
        }
        public void TestDate_IPropertyValueOut(DateTimeOffset inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDelegateArray_IPropertyValueIn(object inValue, out bool isValidType, out DelegateWithOutParam_HSTRING[] outValue)
        {
            isValidType = true;
            outValue = (DelegateWithOutParam_HSTRING[])inValue;
        }
        public void TestDimensionsArray_IPropertyValueIn(object inValue, out bool isValidType, out Dimensions[] outValue)
        {
            isValidType = true;
            outValue = (Dimensions[])inValue;
        }
        public void TestDimensions_IPropertyValueOut(Dimensions? inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDoubleArray_IPropertyValueIn(object inValue, out bool isValidType, out double[] outValue)
        {
            isValidType = true;
            outValue = (double[])inValue;
        }
        public void TestDoubleArray_IPropertyValueOut([ReadOnlyArray]double[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDouble_IPropertyValueOut(double inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestEnumArray_IPropertyValueIn(object inValue, out bool isValidType, out Phylum[] outValue)
        {
            isValidType = true;
            outValue = (Phylum[])inValue;
        }
        public void TestEnum_IPropertyValueOut(Phylum inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestFloatArray_IPropertyValueIn(object inValue, out bool isValidType, out float[] outValue)
        {
            isValidType = true;
            outValue = (float[])inValue;
        }
        public void TestFloatArray_IPropertyValueOut([ReadOnlyArray]float[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestFloat_IPropertyValueOut(float inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestGuidArray_IPropertyValueIn(object inValue, out bool isValidType, out Guid[] outValue)
        {
            isValidType = true;
            outValue = (Guid[])inValue;
        }
        public void TestGuidArray_IPropertyValueOut([ReadOnlyArray]Guid[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestGuid_IPropertyValueOut(Guid inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInspectable_IPropertyValueIn(object inValue, out bool isValidType, out object outValue)
        {
            isValidType = true;
            outValue = inValue;
        }
        public void TestInspectable_IPropertyValueOut(object inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt16Array_IPropertyValueIn(object inValue, out bool isValidType, out System.Int16[] outValue)
        {
            isValidType = true;
            outValue = (System.Int16[])inValue;
        }
        public void TestInt16Array_IPropertyValueOut([ReadOnlyArray]System.Int16[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt16_IPropertyValueOut(System.Int16 inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt32Array_IPropertyValueIn(object inValue, out bool isValidType, out int[] outValue)
        {
            isValidType = true;
            outValue = (int[])inValue;
        }
        public void TestInt32Array_IPropertyValueOut([ReadOnlyArray]int[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt32_IPropertyValueOut(int inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt64Array_IPropertyValueIn(object inValue, out bool isValidType, out long[] outValue)
        {
            isValidType = true;
            outValue = (long[])inValue;
        }
        public void TestInt64Array_IPropertyValueOut([ReadOnlyArray]long[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt64_IPropertyValueOut(long inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestNull_IPropertyValueIn(object inValue, out bool isValidType, out object outValue)
        {
            isValidType = true;
            outValue = inValue;
        }
        public void TestNull_IPropertyValueOut(out object outValue)
        {
            outValue = null;
        }
        public void TestNumber_IPropertyValueIn(object inValue, out bool isValidType, out double outValue)
        {
            isValidType = true;
            outValue = (double)inValue;
        }
        public void TestPointArray_IPropertyValueIn(object inValue, out bool isValidType, out Windows.Foundation.Point[] outValue)
        {
            isValidType = true;
            outValue = (Windows.Foundation.Point[])inValue;
        }
        public void TestPointArray_IPropertyValueOut([ReadOnlyArray]Windows.Foundation.Point[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestPoint_IPropertyValueOut(Windows.Foundation.Point inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRectArray_IPropertyValueIn(object inValue, out bool isValidType, out Windows.Foundation.Rect[] outValue)
        {
            isValidType = true;
            outValue = (Windows.Foundation.Rect[])inValue;
        }
        public void TestRectArray_IPropertyValueOut([ReadOnlyArray]Windows.Foundation.Rect[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRect_IPropertyValueOut(Windows.Foundation.Rect inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestSizeArray_IPropertyValueIn(object inValue, out bool isValidType, out Windows.Foundation.Size[] outValue)
        {
            isValidType = true;
            outValue = (Windows.Foundation.Size[])inValue;
        }
        public void TestSizeArray_IPropertyValueOut([ReadOnlyArray]Windows.Foundation.Size[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestSize_IPropertyValueOut(Windows.Foundation.Size inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestStringArray_IPropertyValueIn(object inValue, out bool isValidType, out string[] outValue)
        {
            isValidType = true;
            outValue = (string[])inValue;
        }
        public void TestStringArray_IPropertyValueOut([ReadOnlyArray]string[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestString_IPropertyValueIn(object inValue, out bool isValidType, out string outValue)
        {
            isValidType = true;
            outValue = (string)inValue;
        }
        public void TestString_IPropertyValueOut(string inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpanArray_IPropertyValueIn(object inValue, out bool isValidType, out TimeSpan[] outValue)
        {
            isValidType = true;
            outValue = (TimeSpan[])inValue;
        }
        public void TestTimeSpanArray_IPropertyValueOut([ReadOnlyArray]TimeSpan[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpan_IPropertyValueOut(TimeSpan inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt16Array_IPropertyValueIn(object inValue, out bool isValidType, out System.UInt16[] outValue)
        {
            isValidType = true;
            outValue = (System.UInt16[])inValue;
        }
        public void TestUInt16Array_IPropertyValueOut([ReadOnlyArray]System.UInt16[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt16_IPropertyValueOut(System.UInt16 inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt32Array_IPropertyValueIn(object inValue, out bool isValidType, out uint[] outValue)
        {
            isValidType = true;
            outValue = (uint[])inValue;
        }
        public void TestUInt32Array_IPropertyValueOut([ReadOnlyArray]uint[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt32_IPropertyValueOut(uint inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt64Array_IPropertyValueIn(object inValue, out bool isValidType, out ulong[] outValue)
        {
            isValidType = true;
            outValue = (ulong[])inValue;
        }
        public void TestUInt64Array_IPropertyValueOut([ReadOnlyArray]ulong[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt64_IPropertyValueOut(ulong inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt8Array_IPropertyValueIn(object inValue, out bool isValidType, out byte[] outValue)
        {
            isValidType = true;
            outValue = (byte[])inValue;
        }
        public void TestUInt8Array_IPropertyValueOut([ReadOnlyArray]byte[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt8_IPropertyValueOut(byte inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestArray_PropertyValueIn(object inValue, out bool isValidType, out object[] outValue)
        {
            isValidType = true;
            outValue = (object[])inValue;
        }
        public void TestArray_PropertyValueOut([ReadOnlyArray]object[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBooleanArray_PropertyValueIn(object inValue, out bool isValidType, out bool[] outValue)
        {
            isValidType = true;
            outValue = (bool[])inValue;
        }
        public void TestBooleanArray_PropertyValueOut([ReadOnlyArray]bool[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoolean_PropertyValueIn(object inValue, out bool isValidType, out bool outValue)
        {
            isValidType = true;
            outValue = (bool)inValue;
        }
        public void TestBoolean_PropertyValueOut(bool inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxInspectable_PropertyValueOut(object inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxedNull_PropertyValueOut(out object outValue)
        {
            outValue = null;
        }
        public void TestChar16Array_PropertyValueIn(object inValue, out bool isValidType, out char[] outValue)
        {
            isValidType = true;
            outValue = (char[])inValue;
        }
        public void TestChar16Array_PropertyValueOut([ReadOnlyArray]char[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestChar16_PropertyValueOut(char inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDateArray_PropertyValueIn(object inValue, out bool isValidType, out DateTimeOffset[] outValue)
        {
            isValidType = true;
            outValue = (DateTimeOffset[])inValue;
        }
        public void TestDateArray_PropertyValueOut([ReadOnlyArray]DateTimeOffset[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDate_PropertyValueIn(object inValue, out bool isValidType, out DateTimeOffset outValue)
        {
            isValidType = true;
            outValue = (DateTimeOffset)inValue;
        }
        public void TestDate_PropertyValueOut(DateTimeOffset inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDelegateArray_PropertyValueIn(object inValue, out bool isValidType, out DelegateWithOutParam_HSTRING[] outValue)
        {
            isValidType = true;
            outValue = (DelegateWithOutParam_HSTRING[])inValue;
        }
        public void TestDimensionsArray_PropertyValueIn(object inValue, out bool isValidType, out Dimensions[] outValue)
        {
            isValidType = true;
            outValue = (Dimensions[])inValue;
        }
        public void TestDimensions_PropertyValueOut(Dimensions? inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDoubleArray_PropertyValueIn(object inValue, out bool isValidType, out double[] outValue)
        {
            isValidType = true;
            outValue = (double[])inValue;
        }
        public void TestDoubleArray_PropertyValueOut([ReadOnlyArray]double[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestDouble_PropertyValueOut(double inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestEnumArray_PropertyValueIn(object inValue, out bool isValidType, out Phylum[] outValue)
        {
            isValidType = true;
            outValue = (Phylum[])inValue;
        }
        public void TestFloatArray_PropertyValueIn(object inValue, out bool isValidType, out float[] outValue)
        {
            isValidType = true;
            outValue = (float[])inValue;
        }
        public void TestFloatArray_PropertyValueOut([ReadOnlyArray]float[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestFloat_PropertyValueOut(float inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestGuidArray_PropertyValueIn(object inValue, out bool isValidType, out Guid[] outValue)
        {
            isValidType = true;
            outValue = (Guid[])inValue;
        }
        public void TestGuidArray_PropertyValueOut([ReadOnlyArray]Guid[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestGuid_PropertyValueOut(Guid inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInspectable_PropertyValueIn(object inValue, out bool isValidType, out object outValue)
        {
            isValidType = true;
            outValue = inValue;
        }
        public void TestInspectable_PropertyValueOut(object inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt16Array_PropertyValueIn(object inValue, out bool isValidType, out System.Int16[] outValue)
        {
            isValidType = true;
            outValue = (System.Int16[])inValue;
        }
        public void TestInt16Array_PropertyValueOut([ReadOnlyArray]System.Int16[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt16_PropertyValueOut(System.Int16 inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt32Array_PropertyValueIn(object inValue, out bool isValidType, out int[] outValue)
        {
            isValidType = true;
            outValue = (System.Int32[])inValue;
        }
        public void TestInt32Array_PropertyValueOut([ReadOnlyArray]int[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt32_PropertyValueOut(int inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt64Array_PropertyValueIn(object inValue, out bool isValidType, out long[] outValue)
        {
            isValidType = true;
            outValue = (long[])inValue;
        }
        public void TestInt64Array_PropertyValueOut([ReadOnlyArray]long[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestInt64_PropertyValueOut(long inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestNull_PropertyValueIn(object inValue, out bool isValidType, out object outValue)
        {
            isValidType = true;
            outValue = inValue;
        }
        public void TestNull_PropertyValueOut(out object outValue)
        {
            outValue = null;
        }
        public void TestNumber_PropertyValueIn(object inValue, out bool isValidType, out double outValue)
        {
            isValidType = true;
            outValue = (double)inValue;
        }
        public void TestPointArray_PropertyValueIn(object inValue, out bool isValidType, out Windows.Foundation.Point[] outValue)
        {
            isValidType = true;
            outValue = (Windows.Foundation.Point[])inValue;
        }
        public void TestPointArray_PropertyValueOut([ReadOnlyArray]Windows.Foundation.Point[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestPoint_PropertyValueOut(Windows.Foundation.Point inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRectArray_PropertyValueIn(object inValue, out bool isValidType, out Windows.Foundation.Rect[] outValue)
        {
            isValidType = true;
            outValue = (Windows.Foundation.Rect[])inValue;
        }
        public void TestRectArray_PropertyValueOut([ReadOnlyArray]Windows.Foundation.Rect[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRect_PropertyValueOut(Windows.Foundation.Rect inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestSizeArray_PropertyValueIn(object inValue, out bool isValidType, out Windows.Foundation.Size[] outValue)
        {
            isValidType = true;
            outValue = (Windows.Foundation.Size[])inValue;
        }
        public void TestSizeArray_PropertyValueOut([ReadOnlyArray]Windows.Foundation.Size[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestSize_PropertyValueOut(Windows.Foundation.Size inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestStringArray_PropertyValueIn(object inValue, out bool isValidType, out string[] outValue)
        {
            isValidType = true;
            outValue = (string[])inValue;
        }
        public void TestStringArray_PropertyValueOut([ReadOnlyArray]string[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestString_PropertyValueIn(object inValue, out bool isValidType, out string outValue)
        {
            isValidType = true;
            outValue = (string)inValue;
        }
        public void TestString_PropertyValueOut(string inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpanArray_PropertyValueIn(object inValue, out bool isValidType, out TimeSpan[] outValue)
        {
            isValidType = true;
            outValue = (TimeSpan[])inValue;
        }
        public void TestTimeSpanArray_PropertyValueOut([ReadOnlyArray]TimeSpan[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpan_PropertyValueOut(TimeSpan inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt16Array_PropertyValueIn(object inValue, out bool isValidType, out System.UInt16[] outValue)
        {
            isValidType = true;
            outValue = (System.UInt16[])inValue;
        }
        public void TestUInt16Array_PropertyValueOut([ReadOnlyArray]System.UInt16[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt16_PropertyValueOut(System.UInt16 inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt32Array_PropertyValueIn(object inValue, out bool isValidType, out uint[] outValue)
        {
            isValidType = true;
            outValue = (uint[])inValue;
        }
        public void TestUInt32Array_PropertyValueOut([ReadOnlyArray]uint[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt32_PropertyValueOut(uint inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt64Array_PropertyValueIn(object inValue, out bool isValidType, out ulong[] outValue)
        {
            isValidType = true;
            outValue = (ulong[])inValue;
        }
        public void TestUInt64Array_PropertyValueOut([ReadOnlyArray]ulong[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt64_PropertyValueOut(ulong inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt8Array_PropertyValueIn(object inValue, out bool isValidType, out byte[] outValue)
        {
            isValidType = true;
            outValue = (byte[])inValue;
        }
        public void TestUInt8Array_PropertyValueOut([ReadOnlyArray]byte[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestUInt8_PropertyValueOut(byte inValue, out object outValue)
        {
            outValue = inValue;
        }
        #endregion
        private void Test_ReferenceHelper<T>(T? inValue, out bool isNull, out bool isValidType, out T outValue) where T : struct
        {
            if (inValue == null)
            {
                isNull = true;
                isValidType = true;
                outValue = default(T);
            }
            else
            {
                isNull = false;
                isValidType = true;
                outValue = (T)inValue;
            }
        }
        public void TestBoolean_ReferenceIn(bool? inValue, out bool isNull, out bool isValidType, out bool outValue)
        {
            Test_ReferenceHelper<bool>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestBoolean_ReferenceOut(out bool? outValue, bool inValue)
        {
            outValue = inValue;
        }
        public void TestChar16_ReferenceIn(char? inValue, out bool isNull, out bool isValidType, out char outValue)
        {
            Test_ReferenceHelper<char>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestChar16_ReferenceOut(out char? outValue, char inValue)
        {
            outValue = inValue;
        }
        public void TestDate_ReferenceIn(DateTimeOffset? inValue, out bool isNull, out bool isValidType, out DateTimeOffset outValue)
        {
            Test_ReferenceHelper<DateTimeOffset>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestDate_ReferenceOut(out DateTimeOffset? outValue, DateTimeOffset inValue)
        {
            outValue = inValue;
        }
        public void TestDimensions_ReferenceIn(Dimensions? inValue, out bool isNull, out bool isValidType, out Dimensions outValue)
        {
            Test_ReferenceHelper<Dimensions>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestDimensions_ReferenceOut(out Dimensions? outValue, Dimensions? inValue)
        {
            outValue = inValue;
        }
        public void TestDouble_ReferenceIn(double? inValue, out bool isNull, out bool isValidType, out double outValue)
        {
            Test_ReferenceHelper<double>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestDouble_ReferenceOut(out double? outValue, double inValue)
        {
            outValue = inValue;
        }
        public void TestFloat_ReferenceIn(float? inValue, out bool isNull, out bool isValidType, out float outValue)
        {
            Test_ReferenceHelper<float>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestFloat_ReferenceOut(out float? outValue, float inValue)
        {
            outValue = inValue;
        }
        public void TestGuid_ReferenceIn(Guid? inValue, out bool isNull, out bool isValidType, out Guid outValue)
        {
            Test_ReferenceHelper<Guid>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestGuid_ReferenceOut(out Guid? outValue, Guid inValue)
        {
            outValue = inValue;
        }
        public void TestInt16_ReferenceIn(System.Int16? inValue, out bool isNull, out bool isValidType, out System.Int16 outValue)
        {
            Test_ReferenceHelper<System.Int16>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestInt16_ReferenceOut(out System.Int16? outValue, System.Int16 inValue)
        {
            outValue = inValue;
        }
        public void TestInt32_ReferenceIn(int? inValue, out bool isNull, out bool isValidType, out int outValue)
        {
            Test_ReferenceHelper<int>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestInt32_ReferenceOut(out int? outValue, int inValue)
        {
            outValue = inValue;
        }
        public void TestInt64_ReferenceIn(long? inValue, out bool isNull, out bool isValidType, out long outValue)
        {
            Test_ReferenceHelper<long>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestInt64_ReferenceOut(out long? outValue, long inValue)
        {
            outValue = inValue;
        }
        public void TestPoint_ReferenceIn(Windows.Foundation.Point? inValue, out bool isNull, out bool isValidType, out Windows.Foundation.Point outValue)
        {
            Test_ReferenceHelper<Windows.Foundation.Point>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestPoint_ReferenceOut(out Windows.Foundation.Point? outValue, Windows.Foundation.Point inValue)
        {
            outValue = inValue;
        }
        public void TestRect_ReferenceIn(Windows.Foundation.Rect? inValue, out bool isNull, out bool isValidType, out Windows.Foundation.Rect outValue)
        {
            Test_ReferenceHelper<Windows.Foundation.Rect>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestRect_ReferenceOut(out Windows.Foundation.Rect? outValue, Windows.Foundation.Rect inValue)
        {
            outValue = inValue;
        }
        public void TestEnum_ReferenceIn(Phylum? inValue, out bool isNull, out bool isValidType, out Phylum outValue)
        {
            Test_ReferenceHelper<Phylum>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestEnum_ReferenceOut(out Phylum? outValue, Phylum inValue)
        {
            outValue = inValue;
        }
        public void TestSize_ReferenceIn(Windows.Foundation.Size? inValue, out bool isNull, out bool isValidType, out Windows.Foundation.Size outValue)
        {
            Test_ReferenceHelper<Windows.Foundation.Size>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestSize_ReferenceOut(out Windows.Foundation.Size? outValue, Windows.Foundation.Size inValue)
        {
            outValue = inValue;
        }
        public void TestTimeSpan_ReferenceIn(TimeSpan? inValue, out bool isNull, out bool isValidType, out TimeSpan outValue)
        {
            Test_ReferenceHelper<TimeSpan>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestTimeSpan_ReferenceOut(out TimeSpan? outValue, TimeSpan inValue)
        {
            outValue = inValue;
        }
        public void TestUInt16_ReferenceIn(System.UInt16? inValue, out bool isNull, out bool isValidType, out System.UInt16 outValue)
        {
            Test_ReferenceHelper<System.UInt16>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestUInt16_ReferenceOut(out System.UInt16? outValue, System.UInt16 inValue)
        {
            outValue = inValue;
        }
        public void TestUInt32_ReferenceIn(uint? inValue, out bool isNull, out bool isValidType, out uint outValue)
        {
            Test_ReferenceHelper<uint>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestUInt32_ReferenceOut(out uint? outValue, uint inValue)
        {
            outValue = inValue;
        }
        public void TestUInt64_ReferenceIn(ulong? inValue, out bool isNull, out bool isValidType, out ulong outValue)
        {
            Test_ReferenceHelper<ulong>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestUInt64_ReferenceOut(out ulong? outValue, ulong inValue)
        {
            outValue = inValue;
        }
        public void TestUInt8_ReferenceIn(byte? inValue, out bool isNull, out bool isValidType, out byte outValue)
        {
            Test_ReferenceHelper<byte>(inValue, out isNull, out isValidType, out outValue);
        }
        public void TestUInt8_ReferenceOut(out byte? outValue, byte inValue)
        {
            outValue = inValue;
        }
        public void TestBoxIterator_InspectableOut(IIterator<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestBoxIVectorView_InspectableOut(IReadOnlyList<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void GetRuntimeClassWithEmptyString(out object inspectable)
        {
            m_empty = new CEmptyGRCNString();
            inspectable = m_empty;
        }
        public void VerifyRuntimeClassWithEmptyString(object inspectable, out bool isSame)
        {
            if (inspectable == m_empty) isSame = true;
            isSame = false;
        }
        public void GetRuntimeClassWithFailingGRCN(out object inspectable)
        {
            inspectable = new CFailingGRCNString();
        }
        public void GetRuntimeClassWithEmptyStringAsInterface(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyGRCN();
        }
        public void TestIterator_InspectableOut(IIterator<int> inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestAnimalArray_IPropertyValueOut([ReadOnlyArray] Animal[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestFishArray_IPropertyValueOut([ReadOnlyArray]IFish[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestVectorArray_IPropertyValueOut([ReadOnlyArray] IList<int>[] inValue, out object outValue)
        {
            outValue = inValue;
        }
        public void TestRCPV1_IPropertyValueOut(out object outValue)
        {
            Animals.Dimensions d = new Animals.Dimensions();
            d.Length = 100;
            d.Width = 20;
            outValue = d;
        }
        public void TestRCPV4_IPropertyValueOut(out object outValue)
        {
            char d = 'D';
            outValue = d;
        }
    }
    public interface IEmptyGRCN
    {
        void GetMyClassName(out string outValue);
    }
    //its runtimeclass name is null
    //TODO
    class CEmptyGRCNString
    {

    }
    //TODO
    class CFailingGRCNString
    {

    }
    class CEmptyGRCN : IEmptyGRCN
    {
        public void GetMyClassName(out string outValue)
        {
            outValue = "CEmptyGRCN";
        }
    }
    class CEmptyGRCNInterface : IEmptyGRCN
    {
        public void GetMyClassName(out string outValue)
        {
            outValue = "CEmptyGRCNInterface";
        }
    }
    class CEmptyFailingGRCNString : IEmptyGRCN
    {
        public void GetMyClassName(out string outValue)
        {
            outValue = "CEmptyFailingGRCNString";
        }
    }
    class CallBackMethods
    {
        public static void AnimalDelegateWithOutParamString(IAnimal animal, out string str)
        {
            Names names = animal.GetNames();
            str = names.Common;
        }
    }
}
