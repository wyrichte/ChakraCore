#include "stdafx.h"
#include "Animal.h"

#define MarshalMethod(type) IFACEMETHOD(Marshal##type)(t_##type _in, __out t_##type* _out) override;

#define IIMock \
    IFACEMETHOD(GetIids)(); \
    IFACEMETHOD(GetRuntimeClassName)(); \
    IFACEMETHOD(GetTrustLevel)();

using namespace Microsoft::WRL;
namespace Animals
{
    [uuid("A88BF705-491F-4FB1-B4C4-00A332135B05")]
        class AnimalFactory : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IAnimalFactory>
    {
        public:
            IIMock;
            IFACEMETHOD(Create)(__deref_out IAnimal **ppObj);
    };
    ;

    [uuid("EB561C4D-2526-4A9E-94D3-4743A5EB658B")]
        class AnimalServer : 
            public RuntimeClass<RuntimeClassFlags<ClassicCom>, IAnimal>
    {

        public:
            AnimalServer();
            IIMock;

            IFACEMETHOD(GetNumLegs)(__out int* numberOfLegs) override;
            IFACEMETHOD(get_Weight)(__out int* weight) override;
            IFACEMETHOD(put_Weight)(int weight) override;
            IFACEMETHOD(GetDimensions)(__out Dimensions* dimensions) override;
            IFACEMETHOD(AddInts)(int val1, int val2, __out int* result) override;
            IFACEMETHOD(GetOuterStruct)(__out OuterStruct* strct) override;
            IFACEMETHOD(MarshalPhylum)(Phylum _in, __out Phylum* _out) override;


            MarshalMethod(Bool);
            MarshalMethod(UInt8);
            MarshalMethod(Int32);
            MarshalMethod(UInt32);
            MarshalMethod(Int64);
            MarshalMethod(UInt64);
            MarshalMethod(Single);
            MarshalMethod(Double);
            MarshalMethod(Char16);
            MarshalMethod(Dimensions);
            MarshalMethod(OuterStruct);
        private:
            int m_Weight;
            Dimensions m_Dimensions;
            OuterStruct m_OuterStruct;
    };
    [uuid("5927ed9f-cee0-466a-be38-1aaa01aea11f")]
        class FishServer :
            public RuntimeClass<RuntimeClassFlags<ClassicCom>, IFish>
    {
        public:
            FishServer();
            IIMock;

            IFACEMETHOD(GetNumFins)(__out int* numberOfFins) override;
        private:
            int m_NumFins;
    };
    ;

    [uuid("46bebd27-90da-447d-80fb-ca55235f6be0")]
        class DinoServer :
            public RuntimeClass<RuntimeClassFlags<ClassicCom>, IDino, IExtinct>
    {
        public:
            DinoServer();
            IIMock;

            IFACEMETHOD(CanRoar)(__out boolean* result);
            IFACEMETHOD(Roar)(int numtimes);

            IFACEMETHOD(IsExtinct)(__out boolean* res);

        private:
            boolean m_canRoar;

    };
    ;
}
