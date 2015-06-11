using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using System.Collections;
using System.Collections.Generic;
using Windows.Foundation;
using Windows.Foundation.Collections;
using System.Runtime.InteropServices.WindowsRuntime;
using Fabrikam.Kitchen;
using Windows.Foundation.Metadata;
namespace Animals
{
    public delegate void DelegateWithInOutParam_Array(IAnimal sender, [WriteOnlyArrayAttribute]int[] myArray);
    public delegate void DelegateWithInOutParam_ArrayHSTRING(IAnimal sender, [WriteOnlyArrayAttribute]string[] myArray);
    public delegate void DelegateWithOutParam_Array(IAnimal sender, out int[] myArray);
    public delegate void DelegateWithOutParam_ArrayHSTRING(IAnimal sender, out string[] myArray);
    public delegate void DelegateWithIterable(IEnumerable<int> inValue, out IEnumerable<int> outValue);
    public delegate void DelegateWithOutParam_MultipleOutParams(IAnimal sender, out Names names, out int newWeight, int weight, out IAnimal outAnimal);
    public delegate void DelegateWithOutParam_HSTRING(IAnimal sender, out string outParam);
    public delegate void DelegateWithOutParam_InOutMixed(IAnimal sender, out Dimensions outParam, int weight);
    public delegate void DelegateWithOutParam_Interface(IAnimal sender, out IAnimal outParam);
    public delegate void DelegateWithOutParam_Struct(IAnimal sender, out Dimensions outParam);
    public delegate void DelegateWithOutParam_int(IAnimal sender, out int outParam);
    public delegate void DelegateWithVector(IList<int> inValue, out IList<int> outValue);
    public delegate void BooleanOut2(out bool p0, out bool p1);
    public delegate void Interface2WithEventHandler(IInterface2WithEvent sender, string hString);
    public delegate void Interface3WithEventHandler(IDummyInterface sender, string hString);
    public delegate void Interface1WithEventHandler(IInterface1WithEvent sender, string hString);
    public delegate void Interface1StaticWithEventHandler(IDummyInterface sender, string hString);
    public delegate void Interface4WithEventHandler(IInterface4WithEvent sender, string hString);
    public delegate void DelegateEventHandler(IInterfaceWithMiscEventFormat sender, DelegateForDelegateEvent inValue);
    public delegate void InterfaceWithTargetEventHandler(IInterfaceWithMiscEventFormat sender, IInterfaceWithMiscEventFormat inValue);
    public delegate void StructEventHandler(IInterfaceWithMiscEventFormat sender, _StructForStructEvent inValue);
    public delegate void DelegateForDelegateEvent(int inValue);
    public delegate void FossilsFoundHandler(Dino dido, int numFound);
    public delegate void CookiesEatenHandler(Pomapoodle puppy, int cookiesEaten);
    public delegate void DelegateOddSizedStruct(OddSizedStruct inValue, out OddSizedStruct outValue);
    public delegate void DelegatePackedBoolean(PackedBoolean4 inValue, out PackedBoolean4 outValue);
    public delegate void DelegatePackedByte(PackedByte inValue, out PackedByte outValue);
    public delegate void DelegateSmallComplexStruct(SmallComplexStruct inValue, out SmallComplexStruct outValue);
    public delegate void DelegateFillArrayWithInLength([WriteOnlyArrayAttribute]int[] value, uint lengthValue);
    public delegate void DelegateFillArrayWithInLengthHSTRING([WriteOnlyArrayAttribute]string[] value, uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
    public delegate uint DelegateFillArrayWithOutLengthWithRetValLengthHSTRING([WriteOnlyArrayAttribute]string[] value);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
    public delegate uint DelegateFillArrayWithOutLengthWithRetValLength([WriteOnlyArrayAttribute]int[] value);

    public delegate void DelegateFillArrayWithOutLengthHSTRING([WriteOnlyArrayAttribute]string[] value, out uint lengthValue);
    public delegate void DelegateFillArrayWithOutLength([WriteOnlyArrayAttribute]int[] value, out uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
    public delegate int DelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING([WriteOnlyArrayAttribute]string[] value, out uint lengthValue);
    public delegate void DelegateWithInParam_Array(IAnimal sender, [ReadOnlyArray]int[] myArray);
    public delegate void DelegateWithInParam_ArrayHSTRING(IAnimal sender, [ReadOnlyArray]string[] myArray);
    public delegate void DelegatePassArrayWithInLength([ReadOnlyArray]int[] value, uint lengthValue);
    public delegate void DelegatePassArrayWithInLengthHSTRING([ReadOnlyArray]string[] value, uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
    public delegate int DelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING([ReadOnlyArray]string[] value, out uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
    public delegate int DelegatePassArrayWithOutLengthWithRetValRandomParam([ReadOnlyArray]int[] Value, out uint lengthValue);
    public delegate uint DelegatePassArrayWithOutLengthWithRetValLengthHSTRING([ReadOnlyArray]string[] outValue);
    public delegate uint DelegatePassArrayWithOutLengthWithRetValLength([ReadOnlyArray]int[] outValue);
    public delegate void DelegatePassArrayWithOutLengthHSTRING([ReadOnlyArray]string[] value, out uint lengthValue);
    public delegate void DelegatePassArrayWithOutLength([ReadOnlyArray]int[] value, out uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
    public delegate int DelegateFillArrayWithOutLengthWithRetValRandomParam([WriteOnlyArrayAttribute]int[] value, out uint lengthValue);
    public delegate void DelegateReceiveArrayWithInLength(out int[] value, uint lengthValue);
    public delegate void DelegateReceiveArrayWithInLengthHSTRING(out string[] value, uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
    public delegate int DelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(out string[] value, out uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
    public delegate int DelegateReceiveArrayWithOutLengthWithRetValRandomParam(out int[] value, out uint lengthValue);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
    public delegate uint DelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(out string[] value);
    [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
    public delegate uint DelegateReceiveArrayWithOutLengthWithRetValLength(out int[] value);
    public delegate void DelegateReceiveArrayWithOutLengthHSTRING(out string[] value, out uint lengthValue);
    public delegate void DelegateReceiveArrayWithOutLength(out int[] value, out uint lengthValue);
    public delegate void DelegateWithFish(Fish inValue, out Fish outValue);
    public delegate void DelegateWithIFish(IFish inValue, out IFish outValue);
    public delegate void DelegateWithLikeToSwim(ILikeToSwim inValue, out ILikeToSwim outValue);
    public delegate void DelegateWithInOut_Float(int inValue1, out float outValue1, float inValue2, int inValue3, int inValue4, float inValue5, out float outValue2);
    public delegate void DelegateBigComplexStruct(BigComplexStruct inValue, out BigComplexStruct outValue);
    public delegate void DelegateWithExtinct(IExtinct inValue, out IExtinct outValue);
    public delegate void DelegateWithInParam_Float(float inValue);
    public delegate void DelegateWithInParam_BigStruct(CollectionChangedEventArgs e, string objectId, CollectionChangeType eType, uint index, uint previousIndex);
    public delegate void DelegateWithOutParam_Float(out float outValue);
    public delegate void DelegateWithOutParam_BigStruct(out CollectionChangedEventArgs args, out string objectId, out CollectionChangeType eType, out uint index, out uint previousIndex);

    public struct BigComplexStruct
    {
        public byte Field0;
        public PackedByte Field1;
        public byte Field2;
        public PackedBoolean4 Field3;
        public SmallComplexStruct Field4;
        public SmallComplexStruct Field5;
        public byte Field6;
        public int Field7;
    }
    public struct OddSizedStruct
    {
        public byte Field0;
        public byte Field1;
        public byte Field2;
    }
    public struct SmallComplexStruct
    {
        public byte Field0;
        public PackedByte Field1;
        public byte Field2;
    }
    public enum CollectionChangeType
    {
        ItemAdded = 0,
        ItemChanged = 1,
        ItemRemoved = 2,
    }
    public struct CollectionChangedEventArgs
    {
        public CollectionChangeType eType;
        public uint index;
        public string objectId;
        public uint previousIndex;
    }
    public struct Dimensions
    {
        public int Length;
        public int Width;
    }
    public struct _StudyInfo
    {
        public string StudyName;
        public Guid SubjectID;
    }
    public struct Names
    {
        public string Common;
        public string Scientific;
        public string AlsoKnownAs;
    }
    public struct _InnerStruct
    {
        public int a;
    }
    public struct OuterStruct
    {
        public _InnerStruct Inner;
    }
    public enum Phylum
    {
        First = 0,
        Acanthocephala = 0,
        Acoelomorpha = 1,
        Annelida = 2,
        Arthropoda = 3,
        Brachiopoda = 4,
        Bryozoa = 5,
        Chaetognatha = 6,
        Chordata = 7,
        Cnidaria = 8,
        Ctenophora = 9,
        Cycliophora = 10,
        Echinodermata = 11,
        Echiura = 12,
        Entoprocta = 13,
        Gastrotricha = 14,
        Gnathostomulida = 15,
        Hemichordata = 16,
        Kinorhyncha = 17,
        Loricifera = 18,
        Micrognathozoa = 19,
        Mollusca = 20,
        Nematoda = 21,
        Nematomorpha = 22,
        Nemertea = 23,
        Onychophora = 24,
        Orthonectida = 25,
        Phoronida = 26,
        Placozoa = 27,
        Platyhelminthes = 28,
        Porifera = 29,
        Priapulida = 30,
        Rhombozoa = 31,
        Rotifera = 32,
        Sipuncula = 33,
        Tardigrada = 34,
        Xenoturbellid = 35,
        Last = 35
    }
    public struct _PhylumChange
    {
        public Phylum Current;
        public Phylum Original;
    }
    public struct PackedBoolean4
    {
        public bool Field0;
        public bool Field1;
        public bool Field2;
        public bool Field3;
    }
    public struct PackedByte
    {
        public byte Field0;
    }
    public interface IAnimal
    {
        Exception ErrorCode { get; }
        Guid ID { get; set; }
        IAnimal Mother { get; set; }
        Dimensions MyDimensions { get; set; }
        Phylum MyPhylum { get; set; }
        int Weight { get; set; }
        int AddInts(int val1, int val2);
        void CallDelegateWithMultipleOutParams(DelegateWithOutParam_MultipleOutParams onDelegateWithMultipleOutParams, out Names names, out int newWeight, int weight, out IAnimal outAnimal);
        void CallDelegateWithOutParam_HSTRING(DelegateWithOutParam_HSTRING onDelegateWithOutHSTRING, out string outParam);
        void CallDelegateWithOutParam_InOutMixed(DelegateWithOutParam_InOutMixed onDelegateWithInOutMixed, out Dimensions outParam, int weight);
        void CallDelegateWithOutParam_int(DelegateWithOutParam_int onDelegateWithOutint, out int outParam);
        void CallDelegateWithOutParam_Interface(DelegateWithOutParam_Interface onDelegateWithOutInterface, out IAnimal outParam);
        void CallDelegateWithOutParam_Struct(DelegateWithOutParam_Struct onDelegateWithOutStruct, out Dimensions outParam);
        void DelIn_BooleanOut2(BooleanOut2 p0);
        void DoubleOffset2Int(int a, int b, double c, out int reta, out int retb, out double retc);
        void DoubleOffsetByte(byte a, double b, out byte reta, out double retb);
        void DoubleOffsetChar(char a, double b, out char reta, out double retb);
        void DoubleOffsetInt(int a, double b, out int reta, out double retb);
        void DoubleOffsetInt64(long a, double b, out long reta, out double retb);
        void DoubleOffsetStruct(Names a, double b, out Names reta, out double retb);
        void FloatOffset2Int(int a, int b, float c, out int reta, out int retb, out float retc);
        void FloatOffsetByte(byte a, float b, out byte reta, out float retb);
        void FloatOffsetChar(char a, float b, out char reta, out float retb);
        void FloatOffsetInt(int a, float b, out int reta, out float retb);
        void FloatOffsetInt64(long a, float b, out long reta, out float retb);
        void FloatOffsetStruct(Names a, float b, out Names reta, out float retb);
        Dimensions GetDimensions();
        void GetGreeting(out string greeting);
        IDictionary<int, string> GetMap(IList<int> uniqueNumbersVector);
        Names GetNames();
        void GetNativeDelegateAsOutParam(out DelegateWithOutParam_HSTRING outDelegate);
        void GetNULLHSTRING(out string _out);
        void GetNumLegs(out int numberOfLegs);
        OuterStruct GetOuterStruct();
        void InterspersedInOutBool(bool a, out bool reta, bool b, out bool retb);
        void InterspersedInOutChar16(char a, out char reta, char b, out char retb);
        void InterspersedInOutDimensions(Dimensions a, out Dimensions reta, Dimensions b, out Dimensions retb);
        void InterspersedInOutDouble(double a, out double reta, double b, out double retb);
        void InterspersedInOutFish(IFish a, out IFish reta, IFish b, out Fish retb);
        void InterspersedInOutHSTRING(string a, string b, out string reta, out string retb);
        void InterspersedInOutIFish(IFish a, out IFish reta, IFish b, out IFish retb);
        void InterspersedInOutInt32(int a, out int reta, int b, out int retb);
        void InterspersedInOutInt64(long a, out long reta, long b, out long retb);
        void InterspersedInOutPhylum(Phylum a, out Phylum reta, Phylum b, out Phylum retb);
        void InterspersedInOutSingle(float a, out float reta, float b, out float retb);
        void InterspersedInOutUInt32(uint a, out uint reta, uint b, out uint retb);
        void InterspersedInOutUInt64(ulong a, out ulong reta, ulong b, out ulong retb);
        void InterspersedInOutUInt8(byte a, out byte reta, byte b, out byte retb);
        bool IsHungry();
        bool isSleepy();
        void LayoutBasicWithStructs(byte a, _InnerStruct b, int c, double d, Names e, byte f, byte g, Dimensions h, int i, out byte reta, out _InnerStruct retb, out int retc, out double retd, out Names rete, out byte retf, out byte retg, out Dimensions reth, out int reti);
        void LayoutOfManyMembers(byte a, int b, byte c, double d, byte e, byte f, double g, int h, double i, out byte reta, out int retb, out byte retc, out double retd, out byte rete, out byte retf, out double retg, out int reth, out double reti);
        void LayoutStructs(_InnerStruct a, Dimensions b, OuterStruct c, Names d, _PhylumChange e, out _InnerStruct reta, out Dimensions retb, out OuterStruct retc, out Names retd, out _PhylumChange rete);
        void LikesChef(out Fabrikam.Kitchen.IChef chef);
        void MarshalBool(bool _in, out bool _out);
        void MarshalChar16(char _in, out char _out);
        void MarshalDimensions(Dimensions _in, out Dimensions _out);
        void MarshalDouble(double _in, out double _out);
        void MarshalGUID(Guid _in, out Guid _out);
        void MarshalHRESULT(Exception hrIn, out Exception hrOut);
        void MarshalHSTRING(string _in, out string _out);
        void MarshalInt16(short _in, out short _out);
        void MarshalInt32(int _in, out int _out);
        void MarshalInt64(long _in, out long _out);
        void MarshalNames(Names _in, out Names _out);
        void MarshalNullAsDelegate(DelegateWithOutParam_HSTRING inDelegate, out string outMessage);
        void MarshalOuterStruct(OuterStruct _in, out OuterStruct _out);
        Phylum MarshalPhylum(Phylum phylum);
        void MarshalPhylumChange(_PhylumChange _in, out _PhylumChange _out);
        void MarshalSingle(float _in, out float _out);
        void MarshalStudyInfo(_StudyInfo _in, out _StudyInfo _out);
        void MarshalUInt16(ushort _in, out ushort _out);
        void MarshalUInt32(uint _in, out uint _out);
        void MarshalUInt64(ulong _in, out ulong _out);
        void MarshalUInt8(byte _in, out byte _out);
        void MethodDelegateAsOutParam(DelegateWithOutParam_HSTRING inDelegate, out DelegateWithOutParam_HSTRING outDelegate);
        void MultiDouble3(double a, double b, double c, out double reta, out double retb, out double retc);
        void MultiDouble4(double a, double b, double c, double d, out double reta, out double retb, out double retc, out double retd);
        void MultiFloat3(float a, float b, float c, out float reta, out float retb, out float retc);
        void MultiFloat4(float a, float b, float c, float d, out float reta, out float retb, out float retc, out float retd);
        void MultipleOutBool(bool a, bool b, out bool reta, out bool retb);
        void MultipleOutChar16(char a, char b, out char reta, out char retb);
        void MultipleOutDimensions(Dimensions a, Dimensions b, out Dimensions reta, out Dimensions retb);
        void MultipleOutDouble(double a, double b, out double reta, out double retb);
        void MultipleOutFish(Fish a, Fish b, out Fish reta, out Fish retb);
        void MultipleOutHSTRING(string a, string b, out string reta, out string retb);
        void MultipleOutIFish(IFish a, IFish b, out IFish reta, out IFish retb);
        void MultipleOutInt32(int a, int b, out int reta, out int retb);
        void MultipleOutInt64(long a, long b, out long reta, out long retb);
        void MultipleOutPhylum(Phylum a, Phylum b, out Phylum reta, out Phylum retb);
        void MultipleOutSingle(float a, float b, out float reta, out float retb);
        void MultipleOutUInt32(uint a, uint b, out uint reta, out uint retb);
        void MultipleOutUInt64(ulong a, ulong b, out ulong reta, out ulong retb);
        void MultipleOutUInt8(byte a, byte b, out byte reta, out byte retb);
        void SetGreeting(string greeting);
        void SetNumLegs(int numberOfLegs);
        void TestError(Exception hr);
        void TestPackedBoolean1(PackedBoolean4 value);
        void TestPackedByte12(PackedByte value);
        void VerifyMarshalGUID(string expected, Guid _in, out Guid _out);
    }
    public interface IArrayMethods
    {
        void CallDelegateFillArray(DelegateWithInOutParam_Array delegateFillArray);
        void CallDelegateFillArrayHSTRING(DelegateWithInOutParam_ArrayHSTRING delegateFillArrayHSTRING);
        void CallDelegateFillArrayWithInLength(DelegateFillArrayWithInLength delegateIn);
        void CallDelegateFillArrayWithInLengthHSTRING(DelegateFillArrayWithInLengthHSTRING delegateIn);
        void CallDelegateFillArrayWithOutLength(DelegateFillArrayWithOutLength delegateIn);
        void CallDelegateFillArrayWithOutLengthHSTRING(DelegateFillArrayWithOutLengthHSTRING delegateIn);
        void CallDelegateFillArrayWithOutLengthWithRetValLength(DelegateFillArrayWithOutLengthWithRetValLength delegateIn);
        void CallDelegateFillArrayWithOutLengthWithRetValLengthHSTRING(DelegateFillArrayWithOutLengthWithRetValLengthHSTRING delegateIn);
        int CallDelegateFillArrayWithOutLengthWithRetValRandomParam(DelegateFillArrayWithOutLengthWithRetValRandomParam delegateIn);
        int CallDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING(DelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING delegateIn);
        void CallDelegatePassArray(DelegateWithInParam_Array delegatePassArray);
        void CallDelegatePassArrayHSTRING(DelegateWithInParam_ArrayHSTRING delegatePassArrayHSTRING);
        void CallDelegatePassArrayWithInLength(DelegatePassArrayWithInLength delegateIn);
        void CallDelegatePassArrayWithInLengthHSTRING(DelegatePassArrayWithInLengthHSTRING delegateIn);
        void CallDelegatePassArrayWithOutLength(DelegatePassArrayWithOutLength delegateIn);
        void CallDelegatePassArrayWithOutLengthHSTRING(DelegatePassArrayWithOutLengthHSTRING delegateIn);
        void CallDelegatePassArrayWithOutLengthWithRetValLength(DelegatePassArrayWithOutLengthWithRetValLength delegateIn);
        void CallDelegatePassArrayWithOutLengthWithRetValLengthHSTRING(DelegatePassArrayWithOutLengthWithRetValLengthHSTRING delegateIn);
        int CallDelegatePassArrayWithOutLengthWithRetValRandomParam(DelegatePassArrayWithOutLengthWithRetValRandomParam delegateIn);
        int CallDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING(DelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING delegateIn);
        void CallDelegateReceiveArray(DelegateWithOutParam_Array delegateReceiveArray);
        void CallDelegateReceiveArrayHSTRING(DelegateWithOutParam_ArrayHSTRING delegateReceiveArrayHSTRING);
        void CallDelegateReceiveArrayWithInLength(DelegateReceiveArrayWithInLength delegateIn);
        void CallDelegateReceiveArrayWithInLengthHSTRING(DelegateReceiveArrayWithInLengthHSTRING delegateIn);
        void CallDelegateReceiveArrayWithOutLength(DelegateReceiveArrayWithOutLength delegateIn);
        void CallDelegateReceiveArrayWithOutLengthHSTRING(DelegateReceiveArrayWithOutLengthHSTRING delegateIn);
        void CallDelegateReceiveArrayWithOutLengthWithRetValLength(DelegateReceiveArrayWithOutLengthWithRetValLength delegateIn);
        void CallDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(DelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING delegateIn);
        int CallDelegateReceiveArrayWithOutLengthWithRetValRandomParam(DelegateReceiveArrayWithOutLengthWithRetValRandomParam delegateIn);
        int CallDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(DelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING delegateIn);
        void FillArray([WriteOnlyArrayAttribute]int[] value, out IList<int> outVector);
        void FillArrayHSTRING([WriteOnlyArrayAttribute]string[] value, out IList<string> outVector);
        void FillArrayWithInLength([WriteOnlyArrayAttribute][Windows.Foundation.Metadata.LengthIs(2)]int[] value, uint lengthValue);
        void FillArrayWithInLengthHSTRING([WriteOnlyArrayAttribute][Windows.Foundation.Metadata.LengthIs(2)]string[] value, uint lengthValue);
        void FillArrayWithOutLength([WriteOnlyArrayAttribute]int[] value, out uint lengthValue);
        void FillArrayWithOutLengthHSTRING([WriteOnlyArrayAttribute]string[] value, out uint lengthValue);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        uint FillArrayWithOutLengthWithRetValLength([WriteOnlyArrayAttribute]int[] value);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        uint FillArrayWithOutLengthWithRetValLengthHSTRING([WriteOnlyArrayAttribute]string[] value);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        int FillArrayWithOutLengthWithRetValRandomParam([WriteOnlyArrayAttribute]int[] value, out uint lengthValue);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        int FillArrayWithOutLengthWithRetValRandomParamHSTRING([WriteOnlyArrayAttribute]string[] value, out uint lengthValue);
        void PassArray([ReadOnlyArray]int[] value, out IList<int> outVector);
        void PassArrayHSTRING([ReadOnlyArray]string[] value, out IList<string> outVector);
        void PassArrayWithInLength([ReadOnlyArray][Windows.Foundation.Metadata.LengthIs(2)]int[] value, uint lengthValue);
        void PassArrayWithInLengthHSTRING([ReadOnlyArray][Windows.Foundation.Metadata.LengthIs(2)]string[] value, uint lengthValue);
        void PassArrayWithOutLength([ReadOnlyArray]int[] value, out uint lengthValue);
        void PassArrayWithOutLengthHSTRING([ReadOnlyArray]string[] value, out uint lengthValue);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        uint PassArrayWithOutLengthWithRetValLength([ReadOnlyArray]int[] value);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        uint PassArrayWithOutLengthWithRetValLengthHSTRING([ReadOnlyArray]string[] value);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        int PassArrayWithOutLengthWithRetValRandomParam([ReadOnlyArray]int[] value, out uint lengthValue);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        int PassArrayWithOutLengthWithRetValRandomParamHSTRING([ReadOnlyArray]string[] value, out uint lengthValue);
        void PureFillArray([WriteOnlyArrayAttribute]int[] value);
        void PurePassArray([ReadOnlyArray]int[] value);
        void PureReceiveArray(out int[] value);
        void ReceiveArray(out int[] value, out IList<int> outVector);
        void ReceiveArrayHSTRING(out string[] value, out IList<string> outVector);
        void ReceiveArrayWithInLength([Windows.Foundation.Metadata.LengthIs(2)]out int[] value, uint lengthValue);
        void ReceiveArrayWithInLengthHSTRING([Windows.Foundation.Metadata.LengthIs(2)]out string[] value, uint lengthValue);
        void ReceiveArrayWithOutLength(out int[] value, out uint lengthValue);
        void ReceiveArrayWithOutLengthHSTRING(out string[] value, out uint lengthValue);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        uint ReceiveArrayWithOutLengthWithRetValLength(out int[] value);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        uint ReceiveArrayWithOutLengthWithRetValLengthHSTRING(out string[] value);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        int ReceiveArrayWithOutLengthWithRetValRandomParam(out int[] value, out uint lengthValue);
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        int ReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(out string[] value, out uint lengthValue);
    }
    /// <summary>
    /// 
    /// </summary>
    public sealed class Animal : IAnimal, IArrayMethods
    {
        int m_Weight;
        int m_NumLegs;
        string m_Greeting;
        Dimensions m_Dimensions;
        OuterStruct m_OuterStruct;
        Names m_Names;
        IAnimal mother;
        Guid m_ID;
        int[] m_array;
        UInt32 m_arraySize;
        uint m_arrayLength;
        string[] m_arrayHSTRING;
        UInt32 m_arraySizeHSTRING;
        uint m_arrayLengthHSTRING;
        Phylum m_Phylum;
        IList<int> m_Vector;
        IEnumerable<int> m_Iterable;
        //AnimalFactory
        static bool m_isLovable;
        static Fish m_Fish;
        static ILikeToSwim m_LikeToSwim;
        static Dino m_Dino;
        static IExtinct m_Extinct;
        static Fabrikam.Kitchen.Toaster m_Toaster;
        #region "IAnimal"
        public Names GetNames()
        {
            return m_Names;
        }
        // Methods that will conflict with camel casing 
        public bool IsHungry()
        {
            return false;
        }
        public bool isSleepy()
        {
            return false;
        }
        // Methods with only [ReadOnlyArray] parameters  
        public void SetNumLegs(int numberOfLegs)
        {
            m_NumLegs = numberOfLegs;
        }
        public void SetGreeting(string greeting)
        {
            m_Greeting = greeting;
        }
        // Methods with only [WriteOnlyArrayAttribute] parameters  
        public void GetNumLegs(out int numberOfLegs)
        {
            numberOfLegs = m_NumLegs;
        }
        public void GetGreeting(out string greeting)
        {
            greeting = m_Greeting;
        }
        public int Weight
        {
            get { return m_Weight; }
            set
            {
                if (value < 110)
                {
                    m_Weight = value;
                }
                else
                {
                    throw new ArgumentException();
                }
            }
        }
        public IAnimal Mother
        {
            get { return mother; }
            set { mother = value; }
        }
        public Guid ID
        {
            get { return m_ID; }
            set { m_ID = value; }
        }
        public Dimensions MyDimensions
        {
            get { return m_Dimensions; }
            set { m_Dimensions = value; }
        }
        public Phylum MyPhylum
        {
            get { return m_Phylum; }
            set { m_Phylum = value; }
        }
        public Dimensions GetDimensions()
        {
            return m_Dimensions;
        }
        public int AddInts(int val1, int val2)
        {
            return val1 + val2;
        }
        public OuterStruct GetOuterStruct()
        {
            return m_OuterStruct;
        }
        public Phylum MarshalPhylum(Phylum phylum)
        {
            return phylum;
        }
        public void MarshalPhylumChange(_PhylumChange _in, out _PhylumChange _out)
        {
            _out = new _PhylumChange();
            _out = _in;
        }
        public void MarshalHSTRING(string _in, out string _out)
        {
            _out = _in;
        }
        public void MarshalNames(Names _in, out Names _out)
        {
            _out = new Names();
            _out = _in;
        }
        public IDictionary<int, string> GetMap(IList<int> uniqueNumbersVector)
        {
            return new Dictionary<int, string>();
        }
        public void LikesChef(out Fabrikam.Kitchen.IChef chef)
        {
            IKitchen kitchen = new Kitchen();
            chef = new Chef("Aarti Sequeira", kitchen);
        }
        public void MarshalBool(bool _in, out bool _out)
        {
            _out = _in;
        }
        public void MarshalUInt8(byte _in, out byte _out)
        {
            _out = _in;
        }
        public void MarshalInt32(int _in, out int _out)
        {
            _out = _in;
        }
        public void MarshalUInt32(uint _in, out uint _out)
        {
            _out = _in;
        }
        public void MarshalInt64(long _in, out long _out)
        {
            _out = _in;
        }
        public void MarshalUInt64(ulong _in, out ulong _out)
        {
            _out = _in;
        }
        public void MarshalSingle(float _in, out float _out)
        {
            _out = _in;
        }
        public void MarshalDouble(double _in, out double _out)
        {
            _out = _in;
        }
        public void MarshalChar16(char _in, out char _out)
        {
            _out = _in;
        }
        public void MarshalInt16(short _in, out short _out)
        {
            _out = _in;
        }
        public void MarshalUInt16(ushort _in, out ushort _out)
        {
            _out = _in;
        }
        public void MarshalHRESULT(Exception hrIn, out Exception hrOut)
        {
            //hrOut = new HRESULTException("", new COMException());            
            //throw new HRESULTException("", new COMException());            
            throw hrIn;
        }
        public Exception ErrorCode
        {
            get
            {
                return new ArgumentException();
            }
        }
        public void MarshalDimensions(Dimensions _in, out Dimensions _out)
        {
            _out = _in;
        }
        public void MarshalOuterStruct(OuterStruct _in, out OuterStruct _out)
        {
            _out = new OuterStruct();
            _out = _in;
        }
        public void MarshalStudyInfo(_StudyInfo _in, out _StudyInfo _out)
        {
            _out = _in;
        }
        public void MarshalGUID(Guid _in, out Guid _out)
        {
            _out = new Guid();
            _out = _in;
        }
        public void VerifyMarshalGUID(string expected, Guid _in, out Guid _out)
        {
            Guid expectedGuid = new Guid(expected);
            if (!(_in.Equals(expectedGuid)))
                throw new ArgumentException();
            _out = _in;
        }
        public void GetNULLHSTRING(out string _out)
        {
            _out = "";
        }
        //Methods with Multiple [WriteOnlyArrayAttribute] parameters (all basic types)  
        public void MultipleOutBool(bool a, bool b, out bool reta, out bool retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutUInt8(byte a, byte b, out byte reta, out byte retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutInt32(int a, int b, out int reta, out int retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutUInt32(uint a, uint b, out uint reta, out uint retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutInt64(long a, long b, out long reta, out long retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutUInt64(ulong a, ulong b, out ulong reta, out ulong retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutSingle(float a, float b, out float reta, out float retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutDouble(double a, double b, out double reta, out double retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutChar16(char a, char b, out char reta, out char retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutHSTRING(string a, string b, out string reta, out string retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutPhylum(Phylum a, Phylum b, out Phylum reta, out Phylum retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutDimensions(Dimensions a, Dimensions b, out Dimensions reta, out Dimensions retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutIFish(IFish a, IFish b, out IFish reta, out IFish retb)
        {
            reta = a;
            retb = b;
        }
        public void MultipleOutFish(Fish a, Fish b, out Fish reta, out Fish retb)
        {
            reta = a;
            retb = b;
        }
        //Methods with interspersed [ReadOnlyArray] and [WriteOnlyArrayAttribute] parameters (all basic types)  
        public void InterspersedInOutBool(bool a, out bool reta, bool b, out bool retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutUInt8(byte a, out byte reta, byte b, out byte retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutInt32(int a, out int reta, int b, out int retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutUInt32(uint a, out uint reta, uint b, out uint retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutInt64(long a, out long reta, long b, out long retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutUInt64(ulong a, out ulong reta, ulong b, out ulong retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutSingle(float a, out float reta, float b, out float retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutDouble(double a, out double reta, double b, out double retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutChar16(char a, out char reta, char b, out char retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutHSTRING(string a, string b, out string reta, out string retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutPhylum(Phylum a, out Phylum reta, Phylum b, out Phylum retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutDimensions(Dimensions a, out Dimensions reta, Dimensions b, out Dimensions retb)
        {
            reta = new Dimensions();
            retb = new Dimensions();
            reta = a;
            retb = b;
        }
        public void InterspersedInOutIFish(IFish a, out IFish reta, IFish b, out IFish retb)
        {
            reta = a;
            retb = b;
        }
        public void InterspersedInOutFish(IFish a, out IFish reta, IFish b, out Fish retb)
        {
            reta = a;
            retb = (Fish)b;
        }
        //Method to ensure layout is correct for with multiple or different alignment members  
        public void LayoutOfManyMembers(byte a, int b, byte c, double d, byte e, byte f, double g, int h, double i, out byte reta, out int retb, out byte retc, out double retd, out byte rete, out byte retf, out double retg, out int reth, out double reti)
        {
            reta = a;
            retb = b;
            retc = c;
            retd = d;
            rete = e;
            retf = f;
            retg = g;
            reth = h;
            reti = i;
        }
        public void LayoutStructs(_InnerStruct a, Dimensions b, OuterStruct c, Names d, _PhylumChange e, out _InnerStruct reta, out Dimensions retb, out OuterStruct retc, out Names retd, out _PhylumChange rete)
        {
            reta = a;
            retb = b;
            retc = c;
            retd = d;
            rete = e;
            retd.Common = d.Common;
            retd.AlsoKnownAs = d.AlsoKnownAs;
            retd.Scientific = d.Scientific;
        }
        public void LayoutBasicWithStructs(byte a, _InnerStruct b, int c, double d, Names e, byte f, byte g, Dimensions h, int i, out byte reta, out _InnerStruct retb, out int retc, out double retd, out Names rete, out byte retf, out byte retg, out Dimensions reth, out int reti)
        {
            reta = a;
            retb = b;
            retc = c;
            retd = d;
            rete = e;
            retf = f;
            retg = g;
            reth = h;
            reti = i;
            rete.Common = e.Common;
            rete.Scientific = e.Scientific;
            rete.AlsoKnownAs = e.AlsoKnownAs;
        }
        //Methods with multiple float/double parameters
        public void MultiFloat3(float a, float b, float c, out float reta, out float retb, out float retc)
        {
            reta = a;
            retb = b;
            retc = c;
        }
        public void MultiFloat4(float a, float b, float c, float d, out float reta, out float retb, out float retc, out float retd)
        {
            reta = a;
            retb = b;
            retc = c;
            retd = d;
        }
        public void MultiDouble3(double a, double b, double c, out double reta, out double retb, out double retc)
        {
            reta = a;
            retb = b;
            retc = c;
        }
        public void MultiDouble4(double a, double b, double c, double d, out double reta, out double retb, out double retc, out double retd)
        {
            reta = a;
            retb = b;
            retc = c;
            retd = d;
        }
        //Methods with float/double parameters at different offsets
        public void FloatOffsetChar(char a, float b, out char reta, out float retb)
        {
            reta = a;
            retb = b;
        }
        public void FloatOffsetByte(byte a, float b, out byte reta, out float retb)
        {
            reta = a;
            retb = b;
        }
        public void FloatOffsetInt(int a, float b, out int reta, out float retb)
        {
            reta = a;
            retb = b;
        }
        public void FloatOffsetInt64(long a, float b, out long reta, out float retb)
        {
            reta = a;
            retb = b;
        }
        public void FloatOffset2Int(int a, int b, float c, out int reta, out int retb, out float retc)
        {
            reta = a;
            retb = b;
            retc = c;
        }
        public void FloatOffsetStruct(Names a, float b, out Names reta, out float retb)
        {
            reta = new Names();
            reta.AlsoKnownAs = a.AlsoKnownAs;
            reta.Common = a.Common;
            reta.Scientific = a.Scientific;
            retb = b;
        }
        public void DoubleOffsetChar(char a, double b, out char reta, out double retb)
        {
            reta = a;
            retb = b;
        }
        public void DoubleOffsetByte(byte a, double b, out byte reta, out double retb)
        {
            reta = a;
            retb = b;
        }
        public void DoubleOffsetInt(int a, double b, out int reta, out double retb)
        {
            reta = a;
            retb = b;
        }
        public void DoubleOffsetInt64(long a, double b, out long reta, out double retb)
        {
            reta = a;
            retb = b;
        }
        public void DoubleOffset2Int(int a, int b, double c, out int reta, out int retb, out double retc)
        {
            reta = a;
            retb = b;
            retc = c;
        }
        public void DoubleOffsetStruct(Names a, double b, out Names reta, out double retb)
        {
            reta = a;
            retb = b;
        }
        //Method to return given int as HRESULT (for error testing)  
        public void TestError(Exception hr)
        {
            int hresult = Marshal.GetHRForException(hr);
            throw Marshal.GetExceptionForHR(hresult);
        }
        public void DelIn_BooleanOut2(BooleanOut2 p0)
        {
            bool trueResult;
            bool falseResult;
            p0(out trueResult, out falseResult);
            if ((trueResult != true) || (falseResult != false))
            {
                throw new ArgumentException();
            }
        }
        public void CallDelegateWithOutParam_HSTRING(DelegateWithOutParam_HSTRING onDelegateWithOutHSTRING, out string outParam)
        {
            onDelegateWithOutHSTRING(this, out outParam);
        }
        public void CallDelegateWithOutParam_int(DelegateWithOutParam_int onDelegateWithOutint, out int outParam)
        {
            onDelegateWithOutint(this, out outParam);
        }
        public void CallDelegateWithOutParam_Interface(DelegateWithOutParam_Interface onDelegateWithOutInterface, out IAnimal outParam)
        {
            onDelegateWithOutInterface(this, out outParam);
        }
        public void CallDelegateWithOutParam_Struct(DelegateWithOutParam_Struct onDelegateWithOutStruct, out Dimensions outParam)
        {
            onDelegateWithOutStruct(this, out outParam);
        }
        public void CallDelegateWithOutParam_InOutMixed(DelegateWithOutParam_InOutMixed onDelegateWithInOutMixed, out Dimensions outParam, int weight)
        {
            onDelegateWithInOutMixed(this, out outParam, weight);
        }
        public void CallDelegateWithMultipleOutParams(DelegateWithOutParam_MultipleOutParams onDelegateWithMultipleOutParams, out Names names, out int newWeight, int weight, out IAnimal outAnimal)
        {
            onDelegateWithMultipleOutParams(this, out names, out newWeight, weight, out outAnimal);
        }
        public void MarshalNullAsDelegate(DelegateWithOutParam_HSTRING inDelegate, out string outMessage)
        {
            if (inDelegate == null)
            {
                outMessage = "Success";
            }
            else
            {
                outMessage = "Fail";
            }
        }
        public void MethodDelegateAsOutParam(DelegateWithOutParam_HSTRING inDelegate, out DelegateWithOutParam_HSTRING outDelegate)
        {
            string outStr;
            if (inDelegate != null)
            {
                inDelegate(this, out outStr);
            }
            outDelegate = inDelegate;
        }
        public void GetNativeDelegateAsOutParam(out DelegateWithOutParam_HSTRING outDelegate)
        {
            outDelegate = new DelegateWithOutParam_HSTRING(CallBackMethods.AnimalDelegateWithOutParamString);
        }
        public void TestPackedByte12(PackedByte value)
        {
            if (value.Field0 != 188)
                throw new ArgumentException();
        }
        public void TestPackedBoolean1(PackedBoolean4 value)
        {
            if (!(value.Field0) || (value.Field1) || !(value.Field2) || (value.Field3))
            {
                throw new ArgumentException();
            }
        }
        #endregion
        #region IArrayMethods"
        public void PurePassArray([ReadOnlyArray]int[] value)
        {
            PassArrayCore(value, (uint)value.Length);
        }
        public void PureFillArray([WriteOnlyArray]int[] value)
        {
            for (int index = 0; index < m_array.Length; index++)
            {
                value[index] = m_array[index];
            }
        }
        public void PureReceiveArray(out int[] value)
        {
            value = new int[m_arraySize];
            if (m_array != null)
            {
                value = new int[m_arraySize];
                for (uint index = 0; index < m_arrayLength; index++)
                {
                    value[index] = m_array[index];
                }
                for (uint index = m_arrayLength; index < m_arraySize; index++)
                {
                    value[index] = 0;
                }
            }
        }
        public void PassArray([ReadOnlyArray]int[] value, out IList<int> outVector)
        {
            outVector = new List<int>();
            if (value != null)
            {
                m_array = new int[value.Length];
                m_arraySize = (uint)value.Length;
                m_arrayLength = (uint)value.Length;
                int index = 0;
                foreach (int v in value)
                {
                    outVector.Add(v);
                    m_array[index] = v;
                    index++;
                }
            }
        }
        public void FillArray([WriteOnlyArrayAttribute] int[] value, out IList<int> outVector)
        {
            outVector = new List<int>();
            if (value != null)
            {
                for (int index = 0; (index < m_arraySize) && (index < value.Length); index++)
                {
                    value[index] = m_array[index];
                    outVector.Add(value[index]);
                }
                for (uint index = m_arraySize; index < value.Length; index++)
                {
                    value[index] = 0;
                }
            }
        }
        public void ReceiveArray(out int[] value, out IList<int> outVector)
        {
            outVector = new List<int>();
            value = new int[10];
            PureReceiveArray(out value);
            if (m_array != null)
            {
                foreach (int v in m_array)
                {
                    outVector.Add(v);
                }
            }
        }
        public void CallDelegatePassArray(DelegateWithInParam_Array delegatePassArray)
        {
            delegatePassArray(this, m_array);
        }
        public void CallDelegateFillArray(DelegateWithInOutParam_Array delegateFillArray)
        {
            int[] fillArray = new int[m_arraySize];
            delegateFillArray(this, fillArray);
            m_array = fillArray;
            m_arraySize = (uint)fillArray.Length;
        }
        public void CallDelegateReceiveArray(DelegateWithOutParam_Array delegateReceiveArray)
        {
            int[] fillArray;
            delegateReceiveArray(this, out fillArray);
            m_array = fillArray;
            m_arraySize = (uint)fillArray.Length;
        }
        public void PassArrayHSTRING([ReadOnlyArray]string[] value, out IList<string> outVector)
        {
            outVector = new List<string>();
            PassArrayHSTRINGCore(value, (uint)value.Length);
            for (int i = 0; i < value.Length; i++)
            {
                outVector.Add(value[i]);
            }
        }
        public void FillArrayHSTRING([WriteOnlyArrayAttribute]string[] value, out IList<string> outVector)
        {
            outVector = new List<string>();
            if (value != null)
            {
                for (int index = 0; (index < m_arraySizeHSTRING) && (index < value.Length); index++)
                {
                    value[index] = m_arrayHSTRING[index];
                    outVector.Add(value[index]);
                }
                for (uint index = m_arraySizeHSTRING; index < value.Length; index++)
                {
                    value[index] = null;
                }
            }
        }
        public void ReceiveArrayHSTRING(out string[] value, out IList<string> outVector)
        {
            outVector = new List<string>();
            value = new string[m_arraySizeHSTRING];
            for (uint i = 0; i < m_arrayLengthHSTRING; i++)
            {
                value[i] = m_arrayHSTRING[i];
            }
            for (uint i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
            {
                value[i] = string.Empty;
            }
            foreach (string str in value)
            {
                outVector.Add(str);
            }
        }
        public void CallDelegatePassArrayHSTRING(DelegateWithInParam_ArrayHSTRING delegatePassArrayHSTRING)
        {
            //string[] fillArray = new string[m_arraySizeHSTRING];
            //m_arrayHSTRING.CopyTo(fillArray, 0);
            delegatePassArrayHSTRING(this, m_arrayHSTRING);
        }
        public void CallDelegateFillArrayHSTRING(DelegateWithInOutParam_ArrayHSTRING delegateFillArrayHSTRING)
        {
            string[] fillArray = new string[m_arraySizeHSTRING];
            m_arrayHSTRING.CopyTo(fillArray, 0);
            delegateFillArrayHSTRING(this, fillArray);
            m_arrayHSTRING = fillArray;
            m_arraySizeHSTRING = (uint)fillArray.Length;
            m_arrayLengthHSTRING = (uint)fillArray.Length;
        }
        public void CallDelegateReceiveArrayHSTRING(DelegateWithOutParam_ArrayHSTRING delegateReceiveArrayHSTRING)
        {
            string[] fillArray;
            delegateReceiveArrayHSTRING(this, out fillArray);
            m_arrayHSTRING = fillArray;
            m_arraySizeHSTRING = (uint)fillArray.Length;
            m_arrayLengthHSTRING = (uint)fillArray.Length;
        }
        public void PassArrayWithInLength([ReadOnlyArray][Windows.Foundation.Metadata.LengthIs(2)]int[] value, uint lengthValue)
        {
            PassArrayCore(value, lengthValue);
        }
        public void PassArrayWithOutLength([ReadOnlyArray]int[] value, out uint lengthValue)
        {
            PurePassArray(value);
            lengthValue = m_arrayLength;
        }
        public void FillArrayWithInLength([WriteOnlyArrayAttribute][Windows.Foundation.Metadata.LengthIs(2)]int[] value, uint lengthValue)
        {
            if (value == null) throw new ArgumentException("value is null");
            if (lengthValue > value.Length) throw new ArgumentException("The value of lengthValue is wrong");
            uint i = 0;
            for (; i < lengthValue && i < m_arrayLength; i++)
            {
                value[i] = m_array[i];
            }
            for (; i < lengthValue; i++)
                value[i] = 0;
        }
        public void FillArrayWithOutLength([WriteOnlyArrayAttribute]int[] value, out uint lengthValue)
        {
            lengthValue = 0;
            for (uint i = 0; i < value.Length && i < m_arrayLength; i++)
            {
                value[i] = m_array[i];
                lengthValue++;
            }
        }
        public void ReceiveArrayWithInLength([Windows.Foundation.Metadata.LengthIs(2)]out int[] value, uint lengthValue)
        {
            uint finalSize = (lengthValue > m_arraySize) ? lengthValue : m_arraySize;
            value = new int[finalSize];
            int i = 0;
            for (; i < lengthValue && i < m_arrayLength; i++)
            {
                value[i] = m_array[i];
            }
            for (; i < lengthValue; i++)
                value[i] = 0;
        }
        public void ReceiveArrayWithOutLength(out int[] value, out uint lengthValue)
        {
            value = new int[m_arraySize];
            for (uint i = 0; i < m_arrayLength; i++)
            {
                value[i] = m_array[i];
            }
            lengthValue = m_arrayLength;
        }
        public uint PassArrayWithOutLengthWithRetValLength([ReadOnlyArray]int[] value)
        {
            uint lengthValue = 0;
            PassArrayWithOutLength(value, out lengthValue);
            return lengthValue;
        }
        public int PassArrayWithOutLengthWithRetValRandomParam([ReadOnlyArray]int[] value, out uint lengthValue)
        {
            PassArrayWithOutLength(value, out lengthValue);
            return 100;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        public uint FillArrayWithOutLengthWithRetValLength([WriteOnlyArrayAttribute]int[] value)
        {
            uint lengthValue = 0;
            FillArrayWithOutLength(value, out lengthValue);
            return lengthValue;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        public int FillArrayWithOutLengthWithRetValRandomParam([WriteOnlyArrayAttribute]int[] value, out uint lengthValue)
        {
            FillArrayWithOutLength(value, out lengthValue);
            return 100;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        public uint ReceiveArrayWithOutLengthWithRetValLength(out int[] value)
        {
            uint lengthValue = 0;
            ReceiveArrayWithOutLength(out value, out lengthValue);
            return lengthValue;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        public int ReceiveArrayWithOutLengthWithRetValRandomParam(out int[] value, out uint lengthValue)
        {
            ReceiveArrayWithOutLength(out value, out lengthValue);
            return 100;
        }
        public void CallDelegatePassArrayWithInLength(DelegatePassArrayWithInLength delegateIn)
        {
            uint arraysize = m_arraySize;
            int[] pArray = new int[arraysize];
            for (int i = 0; i < pArray.Length; i++)
                pArray[i] = m_array[i];
            uint arrayLength = m_arrayLength;
            delegateIn(pArray, arrayLength);
        }
        public void CallDelegatePassArrayWithOutLength(DelegatePassArrayWithOutLength delegateIn)
        {
            uint arraysize = m_arraySize;
            uint arrayLength = 0;
            int[] pArray = new int[arraysize];
            for (uint i = 0; i < (uint)pArray.Length; i++)
                pArray[i] = m_array[i];
            for (uint j = m_arrayLength; j < arraysize; j++)
                pArray[j] = 0;
            delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
        }
        public void CallDelegateFillArrayWithInLength(DelegateFillArrayWithInLength delegateIn)
        {
            uint arraysize = m_arraySize;
            int[] pArray = new int[arraysize];
            uint arrayLength = m_arrayLength;
            delegateIn(pArray, arrayLength);
        }
        public void CallDelegateFillArrayWithOutLength(DelegateFillArrayWithOutLength delegateIn)
        {
            uint arraysize = m_arraySize;
            int[] pArray = new int[arraysize];
            uint arrayLength = 0;
            delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_array != null) m_array = null;
            m_array = pArray;
            m_arrayLength = arrayLength;
            m_arraySize = arraysize;
        }
        public void CallDelegateReceiveArrayWithInLength(DelegateReceiveArrayWithInLength delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = m_arrayLength;
            int[] pArray = null;
            delegateIn(out pArray, arrayLength);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_array != null)
            {
                m_array = null;
            }
            m_array = pArray;
            m_arrayLength = arrayLength;
            m_arraySize = arraysize;
        }
        public void CallDelegateReceiveArrayWithOutLength(DelegateReceiveArrayWithOutLength delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = 0;
            int[] pArray = null;
            delegateIn(out pArray, out arrayLength);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_array != null)
            {
                m_array = null;
            }
            m_array = pArray;
            m_arrayLength = arrayLength;
            m_arraySize = arraysize;
        }
        public void CallDelegatePassArrayWithOutLengthWithRetValLength(DelegatePassArrayWithOutLengthWithRetValLength delegateIn)
        {
            uint arraysize = m_arraySize;
            uint arrayLength = 0;
            int[] pArray = new int[arraysize];
            for (uint i = 0; i < (uint)pArray.Length; i++)
                pArray[i] = m_array[i];
            for (uint j = m_arrayLength; j < arraysize; j++)
                pArray[j] = 0;
            arrayLength = delegateIn(pArray);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
        }
        public int CallDelegatePassArrayWithOutLengthWithRetValRandomParam(DelegatePassArrayWithOutLengthWithRetValRandomParam delegateIn)
        {
            uint arraysize = m_arraySize;
            uint arrayLength = 0;
            int[] pArray = new int[arraysize];
            for (uint i = 0; i < (uint)pArray.Length; i++)
                pArray[i] = m_array[i];
            for (uint j = m_arrayLength; j < arraysize; j++)
                pArray[j] = 0;
            int randomRetVal = delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            return randomRetVal;
        }
        public void CallDelegateFillArrayWithOutLengthWithRetValLength(DelegateFillArrayWithOutLengthWithRetValLength delegateIn)
        {
            uint arraysize = m_arraySize;
            int[] pArray = new int[arraysize];
            uint arrayLength = delegateIn(pArray);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            m_array = pArray;
            m_arrayLength = arrayLength;
            m_arraySize = arraysize;
        }
        public int CallDelegateFillArrayWithOutLengthWithRetValRandomParam(DelegateFillArrayWithOutLengthWithRetValRandomParam delegateIn)
        {
            uint arraysize = m_arraySize;
            int[] pArray = new int[arraysize];
            uint arrayLength = 0;
            int randomRetVal = delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            m_array = pArray;
            m_arrayLength = arrayLength;
            return randomRetVal;
        }
        public void CallDelegateReceiveArrayWithOutLengthWithRetValLength(DelegateReceiveArrayWithOutLengthWithRetValLength delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = 0;
            int[] pArray = null;
            arrayLength = delegateIn(out pArray);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            m_array = pArray;
            m_arrayLength = arrayLength;
            m_arraySize = arraysize;
        }
        public int CallDelegateReceiveArrayWithOutLengthWithRetValRandomParam(DelegateReceiveArrayWithOutLengthWithRetValRandomParam delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = 0;
            int[] pArray = null;
            int randomRetVal = delegateIn(out pArray, out arrayLength);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_array != null)
            {
                m_array = null;
            }
            m_array = pArray;
            m_arrayLength = arrayLength;
            m_arraySize = arraysize;
            return randomRetVal;
        }
        public void PassArrayWithInLengthHSTRING([ReadOnlyArray][Windows.Foundation.Metadata.LengthIs(2)]string[] value, uint lengthValue)
        {
            PassArrayHSTRINGCore(value, lengthValue);
        }
        public void PassArrayWithOutLengthHSTRING([ReadOnlyArray]string[] value, out uint lengthValue)
        {
            PassArrayHSTRINGCore(value, (uint)value.Length);
            lengthValue = m_arrayLengthHSTRING;
        }
        public void FillArrayWithInLengthHSTRING([WriteOnlyArrayAttribute][Windows.Foundation.Metadata.LengthIs(2)]string[] value, uint lengthValue)
        {
            if (value == null) throw new ArgumentException("value is null");
            if (lengthValue > value.Length) throw new ArgumentException("The value of lengthValue is wrong");
            for (int j = 0; j < value.Length; j++)
            {
                value[j] = string.Empty;
            }
            uint i = 0;
            for (; i < lengthValue && i < m_arrayLengthHSTRING; i++)
            {
                value[i] = m_arrayHSTRING[i];
            }
            for (; i < lengthValue; i++)
                value[i] = string.Empty;
        }
        public void FillArrayWithOutLengthHSTRING([WriteOnlyArrayAttribute]string[] value, out uint lengthValue)
        {
            lengthValue = 0;
            for (uint i = 0; i < (uint)value.Length; i++)
            {
                value[i] = string.Empty;
            }
            for (uint i = 0; i < value.Length && i < m_arrayLengthHSTRING; i++)
            {
                value[i] = m_arrayHSTRING[i];
                lengthValue++;
            }
        }
        public void ReceiveArrayWithInLengthHSTRING([Windows.Foundation.Metadata.LengthIs(2)]out string[] value, uint lengthValue)
        {
            uint finalSize = (lengthValue > m_arraySizeHSTRING) ? lengthValue : m_arraySizeHSTRING;
            value = new string[finalSize];
            for (int j = 0; j < finalSize; j++)
                value[j] = string.Empty;
            int i = 0;
            for (; i < lengthValue && i < m_arraySizeHSTRING; i++)
            {
                value[i] = m_arrayHSTRING[i];
            }
            for (; i < lengthValue; i++)
                value[i] = string.Empty;
        }
        public void ReceiveArrayWithOutLengthHSTRING(out string[] value, out uint lengthValue)
        {
            value = new string[m_arraySizeHSTRING];
            for (uint i = 0; i < m_arraySizeHSTRING; i++)
            {
                value[i] = string.Empty;
            }
            for (uint i = 0; i < m_arrayLengthHSTRING; i++)
            {
                value[i] = m_arrayHSTRING[i];
            }
            lengthValue = m_arrayLengthHSTRING;
        }
        public uint PassArrayWithOutLengthWithRetValLengthHSTRING([ReadOnlyArray]string[] value)
        {
            uint lengthValue = 0;
            PassArrayWithOutLengthHSTRING(value, out lengthValue);
            return lengthValue;
        }
        public int PassArrayWithOutLengthWithRetValRandomParamHSTRING([ReadOnlyArray]string[] value, out uint lengthValue)
        {
            PassArrayWithOutLengthHSTRING(value, out lengthValue);
            return 100;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        public uint FillArrayWithOutLengthWithRetValLengthHSTRING([WriteOnlyArrayAttribute]string[] value)
        {
            uint lengthValue = 0;
            FillArrayWithOutLengthHSTRING(value, out lengthValue);
            return lengthValue;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        public int FillArrayWithOutLengthWithRetValRandomParamHSTRING([WriteOnlyArrayAttribute]string[] value, out uint lengthValue)
        {
            FillArrayWithOutLengthHSTRING(value, out lengthValue);
            return 100;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("lengthValue")]
        public uint ReceiveArrayWithOutLengthWithRetValLengthHSTRING([Windows.Foundation.Metadata.LengthIs(0)]out string[] value)
        {
            uint lengthValue = 0;
            ReceiveArrayWithOutLengthHSTRING(out value, out lengthValue);
            return lengthValue;
        }
        [return: System.Runtime.InteropServices.WindowsRuntime.ReturnValueName("randomRetVal")]
        public int ReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(out string[] value, out uint lengthValue)
        {
            ReceiveArrayWithOutLengthHSTRING(out value, out lengthValue);
            return 100;
        }
        public void CallDelegatePassArrayWithInLengthHSTRING(DelegatePassArrayWithInLengthHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            string[] pArray = new string[arraysize];
            for (int i = 0; i < pArray.Length; i++)
                pArray[i] = m_arrayHSTRING[i];
            uint arrayLength = m_arrayLengthHSTRING;
            delegateIn(pArray, arrayLength);
        }
        public void CallDelegatePassArrayWithOutLengthHSTRING(DelegatePassArrayWithOutLengthHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            uint arrayLength = 0;
            string[] pArray = new string[arraysize];
            for (uint i = 0; i < m_arrayLengthHSTRING; i++)
                pArray[i] = m_arrayHSTRING[i];
            for (uint i = m_arrayLengthHSTRING; i < arraysize; i++)
                pArray[i] = string.Empty;
            delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
        }
        public void CallDelegateFillArrayWithInLengthHSTRING(DelegateFillArrayWithInLengthHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            string[] pArray = new string[arraysize];
            for (int i = 0; i < pArray.Length; i++)
            {
                pArray[i] = string.Empty;
            }
            uint arrayLength = m_arrayLengthHSTRING;
            delegateIn(pArray, arrayLength);
        }
        public void CallDelegateFillArrayWithOutLengthHSTRING(DelegateFillArrayWithOutLengthHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            string[] pArray = new string[arraysize];
            for (int i = 0; i < pArray.Length; i++)
            {
                pArray[i] = string.Empty;
            }
            uint arrayLength = 0;
            delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_arrayHSTRING != null) m_arrayHSTRING = null;
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
        }
        public void CallDelegateReceiveArrayWithInLengthHSTRING(DelegateReceiveArrayWithInLengthHSTRING delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = m_arrayLengthHSTRING;
            string[] pArray = null;
            delegateIn(out pArray, arrayLength);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_arrayHSTRING != null)
            {
                m_arrayHSTRING = null;
                m_arrayLengthHSTRING = 0;
                m_arraySizeHSTRING = 0;
            }
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
        }
        public void CallDelegateReceiveArrayWithOutLengthHSTRING(DelegateReceiveArrayWithOutLengthHSTRING delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = 0;
            string[] pArray = null;
            delegateIn(out pArray, out arrayLength);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_arrayHSTRING != null)
            {
                m_arrayHSTRING = null;
            }
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
        }
        public void CallDelegatePassArrayWithOutLengthWithRetValLengthHSTRING(DelegatePassArrayWithOutLengthWithRetValLengthHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            uint arrayLength = 0;
            string[] pArray = new string[arraysize];
            for (uint i = 0; i < m_arrayLengthHSTRING; i++)
                pArray[i] = m_arrayHSTRING[i];
            for (uint j = m_arrayLengthHSTRING; j < arraysize; j++)
                pArray[j] = string.Empty;
            arrayLength = delegateIn(pArray);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
        }
        public int CallDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING(DelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            uint arrayLength = 0;
            string[] pArray = new string[arraysize];
            for (uint i = 0; i < m_arrayLengthHSTRING; i++)
                pArray[i] = m_arrayHSTRING[i];
            for (uint j = m_arrayLengthHSTRING; j < arraysize; j++)
                pArray[j] = string.Empty;
            int randomRetVal = delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            return randomRetVal;
        }
        public void CallDelegateFillArrayWithOutLengthWithRetValLengthHSTRING(DelegateFillArrayWithOutLengthWithRetValLengthHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            string[] pArray = new string[arraysize];
            for (int i = 0; i < pArray.Length; i++)
            {
                pArray[i] = string.Empty;
            }
            uint arrayLength = delegateIn(pArray);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_arrayHSTRING != null) m_arrayHSTRING = null;
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
        }
        public int CallDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING(DelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING delegateIn)
        {
            uint arraysize = m_arraySizeHSTRING;
            string[] pArray = new string[arraysize];
            for (int i = 0; i < pArray.Length; i++)
            {
                pArray[i] = string.Empty;
            }
            uint arrayLength = 0;
            int randomRetVal = delegateIn(pArray, out arrayLength);
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            if (m_arrayHSTRING != null) m_arrayHSTRING = null;
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
            return randomRetVal;
        }
        public void CallDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(DelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = 0;
            string[] pArray = null;
            arrayLength = delegateIn(out pArray);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
        }
        public int CallDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(DelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING delegateIn)
        {
            uint arraysize = 0;
            uint arrayLength = 0;
            string[] pArray = null;
            int randomRetVal = delegateIn(out pArray, out arrayLength);
            arraysize = (uint)pArray.Length;
            if (arrayLength > arraysize) throw new Exception("Length isnt correct!");
            m_arrayHSTRING = pArray;
            m_arrayLengthHSTRING = arrayLength;
            m_arraySizeHSTRING = arraysize;
            return randomRetVal;
        }
        void PassArrayCore(int[] value, uint lengthValue)
        {
            uint length = (uint)value.Length;
            if (lengthValue > length)
            {
                //                throw new ArgumentException("The value of lengthValue is wrong");
                Marshal.ThrowExceptionForHR(unchecked((int)0x80070057));
            }
            if (m_array != null)
            {
                m_array = null;
                m_arraySize = 0;
                m_arrayLength = 0;
            }
            m_arraySize = length;
            if (m_arraySize > 0)
            {
                m_array = new int[m_arraySize];
                m_arrayLength = lengthValue;
                for (int i = 0; i < m_arrayLength; i++)
                {
                    m_array[i] = value[i];
                }
            }
        }
        void PassArrayHSTRINGCore(string[] value, uint lengthValue)
        {
            uint arraySize = (uint)value.Length;
            if (lengthValue > arraySize)
            {
                //throw new Exception("The value of lengthValue is wrong!");
                Marshal.ThrowExceptionForHR(unchecked((int)0x80070057));
            }
            if (m_arrayHSTRING != null)
            {
                m_arrayHSTRING = null;
                m_arrayLengthHSTRING = 0;
                m_arraySizeHSTRING = 0;
            }
            if (arraySize > 0)
            {
                m_arrayHSTRING = new string[(uint)value.Length];
                m_arraySizeHSTRING = (uint)value.Length;
                m_arrayLengthHSTRING = lengthValue;
                for (uint i = 0; i < m_arrayLengthHSTRING; i++)
                {
                    m_arrayHSTRING[i] = value[i];
                }
                for (uint i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
                {
                    m_arrayHSTRING[i] = string.Empty;
                }
            }
        }
        #endregion
        #region "IGetVector"
        public IList<int> CopyVector(IList<int> inVector)
        {
            return GetVector<int>(inVector);
        }
        public IList<string> GetStringVector()
        {
            return new List<string>() { "Blue", "Red", "Yellow", "Green", "Pink", "Black", "White", "Tan", "Magenta", "Orange" };
        }
        public IList<string> CopyStringVector(IList<string> inVector)
        {
            return GetVector<string>(inVector);
        }
        public void DuplicateIterable(IEnumerable<int> inIterable, out IEnumerable<int> outIterable)
        {
            outIterable = GetCollectionInterface<int>(inIterable);
        }
        public void DuplicateStringIterable(IEnumerable<string> inIterable, out IEnumerable<string> outIterable)
        {
            outIterable = GetCollectionInterface<string>(inIterable);
        }
        public void DuplicateIterator(IIterator<int> inIterator, out IIterator<int> outIterator)
        {
            //outIterator = GetCollectionInterface<int>(inIterator);
            //To make JS happy just return the same pointer. I have verified above case succeeds.
            outIterator = inIterator;
        }
        public void DuplicateStringIterator(IIterator<string> inIterator, out IIterator<string> outIterator)
        {
            //outIterator = GetCollectionInterface<string>(inIterator);
            //To make JS happy just return the same pointer. I have verified above case succeeds.
            outIterator = inIterator;
        }
        public void DuplicateVectorView(IReadOnlyList<int> inVectorView, out IReadOnlyList<int> outVectorView)
        {
            outVectorView = GetCollectionInterface<int>(inVectorView);
        }
        public void DuplicateStringVectorView(IReadOnlyList<string> inVectorView, out IReadOnlyList<string> outVectorView)
        {
            outVectorView = GetCollectionInterface<string>(inVectorView);
        }
        public void DuplicateVector(IList<int> inVector, out IList<int> outVector)
        {
            outVector = GetVector<int>(inVector);
        }
        public void DuplicateStringVector(IList<string> inVector, out IList<string> outVector)
        {
            //outVector = GetVector<string>(inVector);
            //To make JS happy just return the same pointer. I have verified above case succeeds.
            outVector = inVector;
        }
        public void SendBackSameIterable(IEnumerable<int> inIterable, out IEnumerable<int> outIterable)
        {
            outIterable = inIterable;
        }
        public void SendBackSameStringIterable(IEnumerable<string> inIterable, out IEnumerable<string> outIterable)
        {
            outIterable = inIterable;
        }
        public void SendBackSameIterator(IIterator<int> inIterator, out IIterator<int> outIterator)
        {
            outIterator = inIterator;
        }
        public void SendBackSameStringIterator(IIterator<string> inIterator, out IIterator<string> outIterator)
        {
            outIterator = inIterator;
        }
        public void SendBackSameVectorView(IReadOnlyList<int> inVectorView, out IReadOnlyList<int> outVectorView)
        {
            outVectorView = inVectorView;
        }
        public void SendBackSameStringVectorView(IReadOnlyList<string> inVectorView, out IReadOnlyList<string> outVectorView)
        {
            outVectorView = inVectorView;
        }
        public void SendBackSameVector(IList<int> inVector, out IList<int> outVector)
        {
            outVector = inVector;
        }
        public void SendBackSameStringVector(IList<string> inVector, out IList<string> outVector)
        {
            outVector = inVector;
        }
        public void GetObservableVector(out IObservableVector<int> outObservableVector)
        {
            outObservableVector = new RCIObservable();
        }
        public void GetObservableStringVector(out IObservableVector<string> outObservableVector)
        {
            outObservableVector = new RCIDoubleObservable();
        }
        public void SendAndGetIVectorStructs(IList<_InnerStruct> inVector, out IList<_InnerStruct> outVector)
        {
            outVector = new List<_InnerStruct>();
            foreach (_InnerStruct s in inVector)
            {
                outVector.Add(s);
            }
        }
        public void CallDelegateWithVector(DelegateWithVector inValue, out IList<int> outValue)
        {
            inValue(m_Vector, out outValue);
        }
        public void CallDelegateWithIterable(DelegateWithIterable inValue, out IEnumerable<int> outValue)
        {
            inValue(m_Iterable, out outValue);
        }
        public IList<int> MyVector
        {
            get { return m_Vector; }
            set { m_Vector = value; }
        }
        public IEnumerable<int> MyIterable
        {
            get { return m_Iterable; }
            set { m_Iterable = value; }
        }
        public IList<int> GetVector()
        {
            return new List<int>() { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        }
        public void GetReadOnlyVector(IList<int> inVector, out IList<int> outVector)
        {
            if (inVector == null) outVector = null;
            outVector = new AnimalReadOnlyVector_int(inVector, this);
        }
        private IList<T> GetVector<T>(IList<T> inVector)
        {
            List<T> outVector = new List<T>();
            if (inVector == null)
            {
                throw new ArgumentException();
            }
            foreach (T s in inVector)
            {
                outVector.Add(s);
            }
            return outVector;
        }
        private IEnumerable<T> GetCollectionInterface<T>(IEnumerable<T> inValue)
        {
            List<T> outVector = new List<T>();
            if (inValue == null)
            {
                throw new ArgumentException();
            }
            foreach (T s in inValue)
            {
                outVector.Add(s);
            }
            for (int i = 0; i < outVector.Count; i++)
            {
                yield return outVector[i];
            }
        }
        private IIterator<T> GetCollectionInterface<T>(IIterator<T> inValue)
        {
            MyITerator<T> outVector = new MyITerator<T>();
            if (inValue == null)
            {
                throw new ArgumentException();
            }
            while (inValue.HasCurrent)
            {
                outVector.Add(inValue.Current);
                inValue.MoveNext();
            }
            return (IIterator<T>)outVector;
        }
        private IReadOnlyList<T> GetCollectionInterface<T>(IReadOnlyList<T> inValue)
        {
            List<T> outVector = new List<T>();
            if (inValue == null)
            {
                throw new ArgumentException();
            }
            foreach (T t in inValue)
            {
                outVector.Add(t);
            }
            return (IReadOnlyList<T>)outVector;
        }
        #endregion
        #region "IStaticAnimal"
        public static bool IsLovable
        {
            get
            {
                return m_isLovable;
            }
            set
            {
                m_isLovable = value;
            }
        }
        public static int GetAnswer()
        {
            return 42;
        }
        public static bool TakeANap(int numberOfMinutes)
        {
            if (numberOfMinutes <= 0)
            {
                throw Marshal.GetExceptionForHR(unchecked((int)0x80070057));
            }
            else
            {
                if (numberOfMinutes > 10)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        public static Dino DinoMarshalAs()
        {
            return new Dino();
        }
        public static Dino DinoDefault()
        {
            return new Dino();
        }
        public static IList<Dino> DinoDefaultVector()
        {
            List<Dino> dinos = new List<Dino>();
            dinos.Add(new Dino());
            dinos.Add(new Dino());
            return dinos;
        }
        public static void SendBackSameIDino(IDino inValue, out IDino outValue)
        {
            outValue = inValue;
        }
        public static Dino SendBackSameDino(Dino inValue)
        {
            return inValue;
        }
        public static void SendBackSameExtinct(IExtinct inValue, out IExtinct outValue)
        {
            outValue = inValue;
        }
        public static void CallDelegateWithExtinct(DelegateWithExtinct inValue, out IExtinct outValue)
        {
            if (inValue == null)
            {
                throw new ArgumentException();
            }
            inValue(m_Extinct, out outValue);
        }
        public static Dino MyDino
        {
            get
            {
                return m_Dino;
            }
            set
            {
                m_Dino = value;
            }
        }
        public static IExtinct MyExtinct
        {
            get
            {
                return m_Extinct;
            }
            set
            {
                m_Extinct = value;
            }
        }
        public static void SendBackSameInspectableVector(IList<object> inValue, out IList<object> outValue)
        {
            outValue = inValue;
        }
        public static void MethodWithInParam_BigStruct(CollectionChangedEventArgs inParam, out string objectId, out CollectionChangeType eType, out uint index, out uint previousIndex)
        {
            objectId = inParam.objectId;
            eType = inParam.eType;
            index = inParam.index;
            previousIndex = inParam.previousIndex;
        }
        public static void MethodWithOutParam_BigStruct(string objectId, CollectionChangeType eType, uint index, uint previousIndex, out CollectionChangedEventArgs outParam)
        {
            outParam.objectId = objectId;
            outParam.eType = eType;
            outParam.index = index;
            outParam.previousIndex = previousIndex;
        }
        public static void CallDelegateWithInParam_BigStruct(DelegateWithInParam_BigStruct delegateStruct, string objectId, CollectionChangeType eType, uint index, uint previousIndex)
        {
            CollectionChangedEventArgs args;
            string toPassObjectId;
            args.objectId = objectId;
            toPassObjectId = objectId;
            args.eType = eType;
            args.index = index;
            args.previousIndex = previousIndex;
            delegateStruct(args, toPassObjectId, eType, index, previousIndex);
        }
        public static void CallDelegateWithOutParam_BigStruct(DelegateWithOutParam_BigStruct delegateStruct, out string objectId, out CollectionChangeType eType, out uint index, out uint previousIndex, out string objectIdFromStruct, out CollectionChangeType eTypeFromStruct, out uint indexFromStruct, out uint previousIndexFromStruct)
        {
            CollectionChangedEventArgs args;
            delegateStruct(out args, out objectId, out eType, out index, out previousIndex);
            objectIdFromStruct = args.objectId;
            eTypeFromStruct = args.eType;
            indexFromStruct = args.index;
            previousIndexFromStruct = args.previousIndex;
        }
        public static void MarshalInAndOutPackedByte(PackedByte inParam, out PackedByte outParam)
        {
            outParam = inParam;
        }
        public static void GetPackedByteArray(out PackedByte[] value)
        {
            value = new PackedByte[5];
            PackedByte packedByte = new PackedByte();
            // write the values into value array
            for (int i = 0; i < 5; i++)
            {
                packedByte.Field0 = (byte)i;
                value[i] = packedByte;
            }
        }
        public static void CallDelegateWithInOutPackedByte(PackedByte inParam, out PackedByte outParam, DelegatePackedByte delegateIn)
        {
            delegateIn(inParam, out outParam);
        }
        public static void MarshalInAndOutPackedBoolean(PackedBoolean4 inParam, out PackedBoolean4 outParam)
        {
            outParam = inParam;
        }
        public static void GetPackedBooleanArray(out PackedBoolean4[] value)
        {
            value = new PackedBoolean4[5];
            PackedBoolean4 packedBoolean4;
            // write the values into value array
            for (int i = 0; i < 5; i++)
            {
                packedBoolean4.Field0 = false;
                packedBoolean4.Field1 = true;
                packedBoolean4.Field2 = true;
                packedBoolean4.Field3 = false;
                value[i] = packedBoolean4;
            }
        }
        public static void CallDelegateWithInOutPackedBoolean(PackedBoolean4 inParam, out PackedBoolean4 outParam, DelegatePackedBoolean delegateIn)
        {
            delegateIn(inParam, out outParam);
        }
        public static void MarshalInAndOutOddSizedStruct(OddSizedStruct inParam, out OddSizedStruct outParam)
        {
            outParam = inParam;
        }
        public static void GetOddSizedStructArray(out OddSizedStruct[] value)
        {
            value = new OddSizedStruct[5];
            OddSizedStruct oddSizedStruct;
            // write the values into value array
            for (int i = 0; i < 5; i++)
            {
                oddSizedStruct.Field0 = (byte)i;
                oddSizedStruct.Field1 = (byte)(i + 50);
                oddSizedStruct.Field2 = (byte)(i + 200);
                value[i] = oddSizedStruct;
            }
        }
        public static void CallDelegateWithInOutOddSizedStruct(OddSizedStruct inParam, out OddSizedStruct outParam, DelegateOddSizedStruct delegateIn)
        {
            delegateIn(inParam, out outParam);
        }
        public static void MarshalInAndOutSmallComplexStruct(SmallComplexStruct inParam, out SmallComplexStruct outParam)
        {
            outParam = inParam;
        }
        public static void GetSmallComplexStructArray(out SmallComplexStruct[] value)
        {
            value = new SmallComplexStruct[5];
            SmallComplexStruct smallComplexStruct;
            // write the values into value array
            for (int i = 0; i < 5; i++)
            {
                smallComplexStruct.Field0 = (byte)i;
                smallComplexStruct.Field1.Field0 = (byte)(i + 50);
                smallComplexStruct.Field2 = (byte)(i + 200);
                value[i] = smallComplexStruct;
            }
        }
        public static void CallDelegateWithInOutSmallComplexStruct(SmallComplexStruct inParam, out SmallComplexStruct outParam, DelegateSmallComplexStruct delegateIn)
        {
            delegateIn(inParam, out outParam);
        }
        public static void MarshalInAndOutBigComplexStruct(BigComplexStruct inParam, out BigComplexStruct outParam)
        {
            outParam = inParam;
        }
        public static void GetBigComplexStructArray(out BigComplexStruct[] value)
        {
            value = new BigComplexStruct[5];
            BigComplexStruct bigComplexStruct = new BigComplexStruct();
            for (int i = 0; i < 5; i++)
            {
                bigComplexStruct.Field0 = (byte)i;
                bigComplexStruct.Field1.Field0 = (byte)(i + 50);
                bigComplexStruct.Field2 = (byte)(i + 200);
                bigComplexStruct.Field3.Field0 = false;
                bigComplexStruct.Field3.Field1 = true;
                bigComplexStruct.Field3.Field2 = false;
                bigComplexStruct.Field3.Field3 = true;
                bigComplexStruct.Field4.Field0 = (byte)(i + 180);
                bigComplexStruct.Field4.Field1.Field0 = (byte)(i + 150);
                bigComplexStruct.Field4.Field2 = (byte)(i + 190);
                bigComplexStruct.Field5.Field0 = (byte)(i + 80);
                bigComplexStruct.Field5.Field1.Field0 = (byte)(i + 50);
                bigComplexStruct.Field5.Field2 = (byte)(i + 90);
                bigComplexStruct.Field6 = (byte)(i + 7);
                bigComplexStruct.Field7 = (int)(i + 2000);
                value[i] = bigComplexStruct;
            }
        }
        public static void CallDelegateWithInOutBigComplexStruct(BigComplexStruct inParam, out BigComplexStruct outParam, DelegateBigComplexStruct delegateIn)
        {
            delegateIn(inParam, out outParam);
        }
        public static void CallDelegateWithInFloat(DelegateWithInParam_Float inDelegate, float inValue)
        {
            if (inDelegate == null)
                throw new ArgumentException();
            inDelegate(inValue);
        }
        public static void CallDelegateWithOutFloat(DelegateWithOutParam_Float inDelegate, out float outValue)
        {
            inDelegate(out outValue);
        }
        public static void CallDelegateWithInOutFloat(DelegateWithInOut_Float inDelegate, int inValue1, out float outValue1, float inValue2, int inValue3, int inValue4, float inValue5, out float outValue2)
        {
            if ((inDelegate == null))
            {
                throw new ArgumentException();
            }
            inDelegate(inValue1, out outValue1, inValue2, inValue3, inValue4, inValue5, out outValue2);
        }
        public static void PassUInt8Array([ReadOnlyArray]Byte[] value, out IList<byte> passedValuesVector)
        {
            passedValuesVector = new List<byte>();
            for (int i = 0; i < value.Length; i++)
                passedValuesVector.Add(value[i]);
        }
        public static void FillUInt8Array([WriteOnlyArrayAttribute] Byte[] value, IList<byte> fillFromVector)
        {
            for (int i = 0; i < fillFromVector.Count && i < value.Length; i++)
                value[i] = fillFromVector[i];
        }
        #endregion
        #region "IStaticAnimal2"
        public static Guid GetCLSID()
        {
            return new Guid("EB561C4D-2526-4A9E-94D3-4743A5EB658B");
        }
        public static int MultiplyNumbers(int value1, int value2)
        {
            return value1 * value2;
        }
        public static void SendBackSameIFish(IFish inValue, out IFish outValue)
        {
            outValue = inValue;
        }
        public static void SendBackSameFish(Fish inValue, out Fish outValue)
        {
            outValue = inValue;
        }
        public static void SendBackSameLikeToSwim(ILikeToSwim inValue, out ILikeToSwim outValue)
        {
            outValue = inValue;
        }
        public static void CallDelegateWithIFish(DelegateWithIFish inValue, out IFish outValue)
        {
            inValue(m_Fish, out outValue);
        }
        public static void CallDelegateWithFish(DelegateWithFish inValue, out Fish outValue)
        {
            inValue(m_Fish, out outValue);
        }
        public static void CallDelegateWithLikeToSwim(DelegateWithLikeToSwim inValue, out ILikeToSwim outValue)
        {
            inValue(m_LikeToSwim, out outValue);
        }
        public static Fish MyIFish
        {
            get
            {
                return m_Fish;
            }
            set
            {
                m_Fish = value;
            }
        }
        public static Fish MyFish
        {
            get
            {
                return m_Fish;
            }
            set
            {
                m_Fish = value;
            }
        }
        public static void CallMyFishMethod(int expected, out bool result)
        {
            int actual = 0;
            m_Fish.GetNumFins(out actual);
            if (expected == actual) result = true;
            else result = false;
        }
        public static ILikeToSwim MyLikeToSwim
        {
            get
            {
                return m_LikeToSwim;
            }
            set
            {
                m_LikeToSwim = value;
            }
        }
        public static void GetRefCount(object inValue, out ulong refCount)
        {
            if (inValue == null)
            {
                refCount = 0;
            }
            else
            {
                IntPtr pUnk = Marshal.GetIUnknownForObject(inValue);
                refCount = (ulong)Marshal.Release(pUnk) - 1;
            }
        }
        public static ulong MyFishRefCount
        {
            get
            {
                if (m_Fish == null)
                {
                    return 0;
                }
                else
                {
                    IntPtr pUnk = Marshal.GetIUnknownForObject(m_Fish);
                    return (ulong)Marshal.Release(pUnk);
                }
            }
        }
        public static Fabrikam.Kitchen.Toaster MyToaster
        {
            get
            {
                return m_Toaster;
            }
            set
            {
                m_Toaster = value;
            }
        }
        public static ulong MyToasterRefCount
        {
            get
            {
                if (m_Toaster == null)
                {
                    return 0;
                }
                else
                {
                    IntPtr pUnk = Marshal.GetIUnknownForObject(m_Toaster);
                    return (ulong)Marshal.Release(pUnk);
                }
            }
        }
        public static void SendBackSamePropertySet(IPropertySet inValue, out IPropertySet outValue)
        {
            outValue = inValue;
        }
        public static int AnimalObjectSize
        {
            get
            {
                return 100;
            }
        }
        public static void GetStringIntegerMap(out IDictionary<string, int> outValue)
        {
            outValue = new Dictionary<string, int>();
            outValue.Add("by", 2);
            outValue.Add("Hundred", 7);
            outValue.Add("Hundred And Fifty", 17);
        }
        public static void GetObservableStringIntegerMap(out Windows.Foundation.Collections.IObservableMap<string, int> outValue)
        {
            outValue = new ObservableHashMapStringInt();
            outValue.Add("Hundred", 100);
            outValue.Add("Twenty", 20);
            outValue.Add("Five", 5);
        }
        public static void GetStringHiddenTypeMap(out IDictionary<string, IHiddenInterface> outValue, out bool wasMethodCalled)
        {
            outValue = null;
            wasMethodCalled = true;
        }
        public static object GetStaticAnimalAsInspectable()
        {
            throw new NotImplementedException();
        }
        public static object GetStaticAnimalAsStaticInterface()
        {
            throw new NotImplementedException();
        }
        public static void TestDefaultDino(Dino inValue, out bool isSame)
        {
            isSame = true;
        }
        public static void TestDefaultFish(Fish inValue, out bool isSame)
        {
            isSame = true;
        }
        public static void TestDefaultAnimal(Animal inValue, out bool isSame)
        {
            isSame = true;
        }
        public static void TestDefaultMultipleIVector(MultipleIVector inValue, out bool isSame)
        {
            isSame = true;
        }
        #endregion
        #region "IFastSigInterface"
        public static void GetOneVector(out IList<int> outVal)
        {
            outVal = new List<int>();
            for (int i = 1; i < 4; i++)
                outVal.Add(i);
        }
        public static void GetNullAsVector(out IList<int> outVal)
        {
            outVal = null;
        }
        public static void GetOneObservableVector(out IObservableVector<int> outVal)
        {
            outVal = new MyObservableVector<int>();
            for (int i = 1; i < 5; i++)
                outVal.Add(i);
        }
        public static void GetNullAsObservableVector(out IObservableVector<int> outVal)
        {
            outVal = null;
        }
        public static void GetOneAnimal(out IAnimal outVal)
        {
            outVal = new Animal();
        }
        public static void GetNullAsAnimal(out IAnimal outVal)
        {
            outVal = null;
        }
        public static void GetOneMap(out IDictionary<string, int> outVal)
        {
            outVal = new Dictionary<string, int>();
            outVal.Add("by", 2);
            outVal.Add("Hundred", 7);
            outVal.Add("Hundred And Fifty", 17);
        }
        public static void GetNullAsMap(out IDictionary<string, int> outVal)
        {
            outVal = null;
        }
        public static void GetOnePropertyValue(out object outVal)
        {
            double? d = 10.5;
            outVal = d;
        }
        public static void GetNullAsPropertyValue(out object outVal)
        {
            outVal = null;
        }
        public static void GetOneEmptyGRCNInterface(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyGRCNInterface();
        }
        public static void GetOneEmptyGRCNNull(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyGRCN();
        }
        public static void GetOneEmptyGRCNFail(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyFailingGRCNString();
        }
        #endregion
        public Animal()
        {
            m_Weight = 50;
            m_NumLegs = 20;
            m_Dimensions = new Dimensions();
            m_Dimensions.Length = 180;
            m_Dimensions.Width = 360;
            _InnerStruct s = new _InnerStruct();
            s.a = 100;
            m_OuterStruct.Inner = s;
            m_Phylum = Phylum.Acoelomorpha;
            m_Greeting = "Hello";
            m_Names = new Names();
            m_Names.Common = "Wolverine";
            m_Names.Scientific = "Gulo gulo";
            m_Names.AlsoKnownAs = "Skunk Bear";
            m_array = null;
            m_arraySize = 0;
            m_arraySizeHSTRING = 0;
        }
        public Animal(IAnimal mother, int weight)
            : this()
        {
            this.mother = mother;
            m_Weight = weight;
            m_NumLegs = 20;
        }
        public Animal(IAnimal mother, int weight, int leg1, int leg2, int leg3)
            : this()
        {
            this.mother = mother;
            m_Weight = weight;
            m_NumLegs = leg1 + leg2 + leg3;
        }
        public Animal(int numberOfLegs)
            : this()
        {
            m_NumLegs = numberOfLegs;
        }
        public Animal(int legs1, int legs2, int legs3)
            : this()
        {
            m_NumLegs = legs1 + legs2 + legs3;
        }
        public Animal(int legs1, int legs2, int legs3, int legs4, int legs5, int legs6)
            : this()
        {
            m_NumLegs = legs1 + legs2 + legs3 + legs4 + legs5 + legs6;
        }
        public Animal(int legs1, int legs2, int legs3, int legs4, int legs5, int legs6, int legs7)
            : this()
        {
            m_NumLegs = legs1 + legs2 + legs3 + legs4 + legs5 + legs6 + legs7;
        }
    }
    public enum _CLROnly
    {
        CLR = 1
    };
}