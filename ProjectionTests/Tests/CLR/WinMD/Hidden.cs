using System;
using System.Collections.Generic;
using Windows.Foundation.Metadata;
using System.Runtime.InteropServices.WindowsRuntime;
namespace Animals
{
    public delegate void DelegateUsing_HiddenClass_In(HiddenClass value);
    public delegate void DelegateUsing_HiddenClass_Out(out HiddenClass value);
    public delegate void DelegateUsing_HiddenInterface_In(IHiddenInterface value);
    public delegate void DelegateUsing_HiddenInterface_Out(out IHiddenInterface value);
    public delegate void DelegateUsing_VisibleClassWithDefaultHiddenInterface_In(VisibleClassWithDefaultHiddenInterface value);
    public delegate void DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out(out VisibleClassWithDefaultHiddenInterface value);
    public delegate void DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In(VisibleClassWithHiddenInterfaceOnly value);
    public delegate void DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out(out VisibleClassWithHiddenInterfaceOnly value);
    [WebHostHidden]
    public delegate void HiddenDelegate(int value);
    public struct _StructWithHiddenInnerStruct
    {
        public _HiddenStruct HiddenInnerStruct;
        public int VisibleStructMember;
    }
    [WebHostHidden]
    public interface IHiddenInterface
    {
        void HiddenMethod();
    }
    [WebHostHidden]
    public interface IRequiredHiddenInterface
    {
        void RequiredHiddenMethod();
    }
    public interface IVisibleInterface : IRequiredHiddenInterface
    {
        VisibleClassWithDefaultHiddenInterface Property__VisibleClassWithDefaultHiddenInterface { get; set; }
        VisibleClassWithDefaultVisibleInterface Property__VisibleClassWithDefaultVisibleInterface { get; set; }
        HiddenClass Property_HiddenClass { get; set; }
        IHiddenInterface Property_HiddenInterface { get; set; }
        VisibleClassWithHiddenInterfaceOnly Property_VisibleClassWithHiddenInterfaceOnly { get; set; }
        void Call_DelegateUsing_HiddenClass_In(DelegateUsing_HiddenClass_In value);
        void Call_DelegateUsing_HiddenClass_Out(DelegateUsing_HiddenClass_Out value);
        void Call_DelegateUsing_HiddenInterface_In(DelegateUsing_HiddenInterface_In value);
        void Call_DelegateUsing_HiddenInterface_Out(DelegateUsing_HiddenInterface_Out value);
        void Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In(DelegateUsing_VisibleClassWithDefaultHiddenInterface_In value);
        void Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out(DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out value);
        void Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In(DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In value);
        void Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out(DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out value);
        void Call_HiddenDelegate(HiddenDelegate value);
        void FillArray_HiddenClass([WriteOnlyArray]HiddenClass[] value);
        void FillArray_HiddenInterface([WriteOnlyArray]IHiddenInterface[] value);
        void FillArray_VisibleClassWithDefaultHiddenInterface([WriteOnlyArray]VisibleClassWithDefaultHiddenInterface[] value);
        void FillArray_VisibleClassWithDefaultVisibleInterface([WriteOnlyArray]VisibleClassWithDefaultVisibleInterface[] value);
        void FillArray_VisibleClassWithHiddenInterfaceOnly([WriteOnlyArray]VisibleClassWithHiddenInterfaceOnly[] value);
        void Get_HiddenDelegate(out HiddenDelegate value);
        void HiddenEnum_In(HiddenEnum value);
        void HiddenEnum_Out(out HiddenEnum value);
        void HiddenOverload(out HiddenEnum value);
        void HiddenOverload(int inValue, out int outValue);
        void HiddenStruct_In(_HiddenStruct value);
        _HiddenStruct HiddenStruct_Out();
        void MethodUsing_HiddenClass_In(HiddenClass hiddenInterface);
        void MethodUsing_HiddenClass_Out(out HiddenClass hiddenInterface);
        void MethodUsing_HiddenInterface_In(IHiddenInterface hiddenInterface);
        void MethodUsing_HiddenInterface_Out(out IHiddenInterface hiddenInterface);
        void MethodUsing_VisibleClassWithDefaultHiddenInterface_In(VisibleClassWithDefaultHiddenInterface hiddenInterface);
        void MethodUsing_VisibleClassWithDefaultHiddenInterface_Out(out VisibleClassWithDefaultHiddenInterface hiddenInterface);
        void MethodUsing_VisibleClassWithDefaultVisibleInterface_In(VisibleClassWithDefaultVisibleInterface visibleInterface);
        void MethodUsing_VisibleClassWithDefaultVisibleInterface_Out(out VisibleClassWithDefaultVisibleInterface visibleInterface);
        void MethodUsing_VisibleClassWithHiddenInterfaceOnly_In(VisibleClassWithHiddenInterfaceOnly hiddenInterface);
        void MethodUsing_VisibleClassWithHiddenInterfaceOnly_Out(out VisibleClassWithHiddenInterfaceOnly hiddenInterface);
        void PassArray_HiddenClass([ReadOnlyArray] HiddenClass[] value);
        void PassArray_HiddenInterface([ReadOnlyArray] IHiddenInterface[] value);
        void PassArray_VisibleClassWithDefaultHiddenInterface([ReadOnlyArray] VisibleClassWithDefaultHiddenInterface[] value);
        void PassArray_VisibleClassWithDefaultVisibleInterface([ReadOnlyArray] VisibleClassWithDefaultVisibleInterface[] value);
        void PassArray_VisibleClassWithHiddenInterfaceOnly([ReadOnlyArray] VisibleClassWithHiddenInterfaceOnly[] value);
        void ReceiveArray_HiddenClass(out HiddenClass[] value);
        void ReceiveArray_HiddenInterface(out IHiddenInterface[] value);
        void ReceiveArray_VisibleClassWithDefaultHiddenInterface(out VisibleClassWithDefaultHiddenInterface[] value);
        void ReceiveArray_VisibleClassWithDefaultVisibleInterface(out VisibleClassWithDefaultVisibleInterface[] value);
        void ReceiveArray_VisibleClassWithHiddenInterfaceOnly(out VisibleClassWithHiddenInterfaceOnly[] value);
        void StructWithHiddenInnerStruct_In(_StructWithHiddenInnerStruct value);
        void StructWithHiddenInnerStruct_Out(out _StructWithHiddenInnerStruct value);
        void Vector_HiddenClass_In(IList<HiddenClass> value);
        void Vector_HiddenClass_Out(out IList<HiddenClass> value);
        void Vector_HiddenInterface_In(IList<IHiddenInterface> value);
        void Vector_HiddenInterface_Out(out IList<IHiddenInterface> value);
        void Vector_VisibleClassWithDefaultHiddenInterface_In(IList<VisibleClassWithDefaultHiddenInterface> value);
        void Vector_VisibleClassWithDefaultHiddenInterface_Out(out IList<VisibleClassWithDefaultHiddenInterface> value);
        void Vector_VisibleClassWithDefaultVisibleInterface_In(IList<VisibleClassWithDefaultVisibleInterface> value);
        void Vector_VisibleClassWithDefaultVisibleInterface_Out(out IList<VisibleClassWithDefaultVisibleInterface> value);
        void Vector_VisibleClassWithHiddenInterfaceOnly_In(IList<VisibleClassWithHiddenInterfaceOnly> value);
        void Vector_VisibleClassWithHiddenInterfaceOnly_Out(out IList<VisibleClassWithHiddenInterfaceOnly> value);
        void VisibleMethod();
    }
    [WebHostHidden]
    public enum HiddenEnum
    {
        First = 0,
        Second = 1,
        Third = 2,
    }
    [WebHostHidden]
    public struct _HiddenStruct
    {
        public int HiddenStructMember;
    }
    [WebHostHidden]
    public sealed class HiddenClass : IHiddenInterface
    {
        public void HiddenMethod()
        {
        }
    }
    public sealed class VisibleClassWithHiddenInterfaceOnly : IHiddenInterface
    {
        public void HiddenMethod()
        {
        }
    }
    public sealed class VisibleClassWithDefaultHiddenInterface : IHiddenInterface, IVisibleInterface
    {
        private _HiddenStruct hiddenStruct;
        private _StructWithHiddenInnerStruct structWithHiddenInnerStruct;
        public VisibleClassWithDefaultHiddenInterface()
        {
        }
        public void HiddenMethod()
        {
        }
        public VisibleClassWithDefaultHiddenInterface Property__VisibleClassWithDefaultHiddenInterface
        {
            get
            {
                return this;
            }
            set
            {
            }
        }
        public VisibleClassWithDefaultVisibleInterface Property__VisibleClassWithDefaultVisibleInterface
        {
            get
            {
                return (this as IHiddenInterface) as VisibleClassWithDefaultVisibleInterface;
            }
            set
            {
            }
        }
        public HiddenClass Property_HiddenClass
        {
            get
            {
                return (this as IHiddenInterface) as HiddenClass;
            }
            set
            {
            }
        }
        public IHiddenInterface Property_HiddenInterface
        {
            get
            {
                return this;
            }
            set
            {
            }
        }
        public VisibleClassWithHiddenInterfaceOnly Property_VisibleClassWithHiddenInterfaceOnly
        {
            get
            {
                return (this as IHiddenInterface) as VisibleClassWithHiddenInterfaceOnly;
            }
            set
            {
            }
        }
        public void Call_DelegateUsing_HiddenClass_In(DelegateUsing_HiddenClass_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_HiddenClass_Out(DelegateUsing_HiddenClass_Out value)
        {
            if (value != null)
            {
                HiddenClass hiddenClass;
                value(out hiddenClass);
            }
        }
        public void Call_DelegateUsing_HiddenInterface_In(DelegateUsing_HiddenInterface_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_HiddenInterface_Out(DelegateUsing_HiddenInterface_Out value)
        {
            if (value != null)
            {
                IHiddenInterface hiddenInterface;
                value(out hiddenInterface);
            }
        }
        public void Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In(DelegateUsing_VisibleClassWithDefaultHiddenInterface_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out(DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out value)
        {
            if (value != null)
            {
                VisibleClassWithDefaultHiddenInterface hiddenInterface;
                value(out hiddenInterface);
            }
        }
        public void Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In(DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out(DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out value)
        {
            if (value != null)
            {
                VisibleClassWithHiddenInterfaceOnly hiddenInterface;
                value(out hiddenInterface);
            }
        }
        public void Call_HiddenDelegate(HiddenDelegate value)
        {
            if (value != null)
                value(111);
        }
        public void FillArray_HiddenClass([WriteOnlyArray]HiddenClass[] value)
        {
        }
        public void FillArray_HiddenInterface([WriteOnlyArray]IHiddenInterface[] value)
        {
        }
        public void FillArray_VisibleClassWithDefaultHiddenInterface([WriteOnlyArray]VisibleClassWithDefaultHiddenInterface[] value)
        {
        }
        public void FillArray_VisibleClassWithDefaultVisibleInterface([WriteOnlyArray]VisibleClassWithDefaultVisibleInterface[] value)
        {
        }
        public void FillArray_VisibleClassWithHiddenInterfaceOnly([WriteOnlyArray]VisibleClassWithHiddenInterfaceOnly[] value)
        {
        }
        public void Get_HiddenDelegate(out HiddenDelegate value)
        {
            value = null;
        }
        public void HiddenEnum_In(HiddenEnum value)
        {
        }
        public void HiddenEnum_Out(out HiddenEnum value)
        {
            value = HiddenEnum.First;
        }
        public void HiddenStruct_In(_HiddenStruct value)
        {
            hiddenStruct.HiddenStructMember = value.HiddenStructMember;
        }
        public _HiddenStruct HiddenStruct_Out()
        {
            return hiddenStruct;
        }
        public void MethodUsing_HiddenClass_In(HiddenClass hiddenInterface)
        {
        }
        public void MethodUsing_HiddenClass_Out(out HiddenClass hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void MethodUsing_HiddenInterface_In(IHiddenInterface hiddenInterface)
        {
        }
        public void MethodUsing_HiddenInterface_Out(out IHiddenInterface hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void MethodUsing_VisibleClassWithDefaultHiddenInterface_In(VisibleClassWithDefaultHiddenInterface hiddenInterface)
        {
        }
        public void MethodUsing_VisibleClassWithDefaultHiddenInterface_Out(out VisibleClassWithDefaultHiddenInterface hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void MethodUsing_VisibleClassWithDefaultVisibleInterface_In(VisibleClassWithDefaultVisibleInterface visibleInterface)
        {
        }
        public void MethodUsing_VisibleClassWithDefaultVisibleInterface_Out(out VisibleClassWithDefaultVisibleInterface visibleInterface)
        {
            visibleInterface = this as IHiddenInterface as VisibleClassWithDefaultVisibleInterface;
        }
        public void MethodUsing_VisibleClassWithHiddenInterfaceOnly_In(VisibleClassWithHiddenInterfaceOnly hiddenInterface)
        {
        }
        public void MethodUsing_VisibleClassWithHiddenInterfaceOnly_Out(out VisibleClassWithHiddenInterfaceOnly hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void PassArray_HiddenClass([ReadOnlyArray]HiddenClass[] value)
        {
        }
        public void PassArray_HiddenInterface([ReadOnlyArray]IHiddenInterface[] value)
        {
        }
        public void PassArray_VisibleClassWithDefaultHiddenInterface([ReadOnlyArray]VisibleClassWithDefaultHiddenInterface[] value)
        {
        }
        public void PassArray_VisibleClassWithDefaultVisibleInterface([ReadOnlyArray]VisibleClassWithDefaultVisibleInterface[] value)
        {
        }
        public void PassArray_VisibleClassWithHiddenInterfaceOnly([ReadOnlyArray]VisibleClassWithHiddenInterfaceOnly[] value)
        {
        }
        public void ReceiveArray_HiddenClass(out HiddenClass[] value)
        {
            value = null;
        }
        public void ReceiveArray_HiddenInterface(out IHiddenInterface[] value)
        {
            value = null;
        }
        public void ReceiveArray_VisibleClassWithDefaultHiddenInterface(out VisibleClassWithDefaultHiddenInterface[] value)
        {
            value = null;
        }
        public void ReceiveArray_VisibleClassWithDefaultVisibleInterface(out VisibleClassWithDefaultVisibleInterface[] value)
        {
            value = null;
        }
        public void ReceiveArray_VisibleClassWithHiddenInterfaceOnly(out VisibleClassWithHiddenInterfaceOnly[] value)
        {
            value = null;
        }
        public void StructWithHiddenInnerStruct_In(_StructWithHiddenInnerStruct value)
        {
            structWithHiddenInnerStruct.VisibleStructMember = value.VisibleStructMember;
            structWithHiddenInnerStruct.HiddenInnerStruct = value.HiddenInnerStruct;
        }
        public void StructWithHiddenInnerStruct_Out(out _StructWithHiddenInnerStruct value)
        {
            value = structWithHiddenInnerStruct;
        }

        public void Vector_HiddenInterface_In(IList<IHiddenInterface> value)
        {
        }
        public void Vector_HiddenInterface_Out(out IList<IHiddenInterface> value)
        {
            value = null;
        }
        public void VisibleMethod()
        {
        }
        public void RequiredHiddenMethod()
        {
        }


        public void HiddenOverload(out HiddenEnum value)
        {
            throw new NotImplementedException();
        }

        public void HiddenOverload(int inValue, out int outValue)
        {
            throw new NotImplementedException();
        }

        public void Vector_HiddenClass_In(IList<HiddenClass> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_HiddenClass_Out(out IList<HiddenClass> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultHiddenInterface_In(IList<VisibleClassWithDefaultHiddenInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultHiddenInterface_Out(out IList<VisibleClassWithDefaultHiddenInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultVisibleInterface_In(IList<VisibleClassWithDefaultVisibleInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultVisibleInterface_Out(out IList<VisibleClassWithDefaultVisibleInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithHiddenInterfaceOnly_In(IList<VisibleClassWithHiddenInterfaceOnly> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithHiddenInterfaceOnly_Out(out IList<VisibleClassWithHiddenInterfaceOnly> value)
        {
            throw new NotImplementedException();
        }
    }
    public sealed class VisibleClassWithDefaultVisibleInterface : IVisibleInterface, IHiddenInterface
    {
        private _HiddenStruct hiddenStruct;
        private _StructWithHiddenInnerStruct structWithHiddenInnerStruct;
        public VisibleClassWithDefaultVisibleInterface()
        {
        }
        public void HiddenMethod()
        {
        }
        public VisibleClassWithDefaultHiddenInterface Property__VisibleClassWithDefaultHiddenInterface
        {
            get
            {
                return this as IVisibleInterface as VisibleClassWithDefaultHiddenInterface;
            }
            set
            {
            }
        }
        public VisibleClassWithDefaultVisibleInterface Property__VisibleClassWithDefaultVisibleInterface
        {
            get
            {
                return (this as IVisibleInterface) as VisibleClassWithDefaultVisibleInterface;
            }
            set
            {
            }
        }
        public HiddenClass Property_HiddenClass
        {
            get
            {
                return (this as IHiddenInterface) as HiddenClass;
            }
            set
            {
            }
        }
        public IHiddenInterface Property_HiddenInterface
        {
            get
            {
                return this;
            }
            set
            {
            }
        }
        public VisibleClassWithHiddenInterfaceOnly Property_VisibleClassWithHiddenInterfaceOnly
        {
            get
            {
                return (this as IHiddenInterface) as VisibleClassWithHiddenInterfaceOnly;
            }
            set
            {
            }
        }
        public void Call_DelegateUsing_HiddenClass_In(DelegateUsing_HiddenClass_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_HiddenClass_Out(DelegateUsing_HiddenClass_Out value)
        {
            if (value != null)
            {
                HiddenClass hiddenClass;
                value(out hiddenClass);
            }
        }
        public void Call_DelegateUsing_HiddenInterface_In(DelegateUsing_HiddenInterface_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_HiddenInterface_Out(DelegateUsing_HiddenInterface_Out value)
        {
            if (value != null)
            {
                IHiddenInterface hiddenInterface;
                value(out hiddenInterface);
            }
        }
        public void Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In(DelegateUsing_VisibleClassWithDefaultHiddenInterface_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out(DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out value)
        {
            if (value != null)
            {
                VisibleClassWithDefaultHiddenInterface hiddenInterface;
                value(out hiddenInterface);
            }
        }
        public void Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In(DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In value)
        {
            if (value != null)
                value(null);
        }
        public void Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out(DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out value)
        {
            if (value != null)
            {
                VisibleClassWithHiddenInterfaceOnly hiddenInterface;
                value(out hiddenInterface);
            }
        }
        public void Call_HiddenDelegate(HiddenDelegate value)
        {
            if (value != null)
                value(111);
        }
        public void FillArray_HiddenClass([WriteOnlyArray] HiddenClass[] value)
        {
        }
        public void FillArray_HiddenInterface([WriteOnlyArray] IHiddenInterface[] value)
        {
        }
        public void FillArray_VisibleClassWithDefaultHiddenInterface([WriteOnlyArray] VisibleClassWithDefaultHiddenInterface[] value)
        {
        }
        public void FillArray_VisibleClassWithDefaultVisibleInterface([WriteOnlyArray] VisibleClassWithDefaultVisibleInterface[] value)
        {
        }
        public void FillArray_VisibleClassWithHiddenInterfaceOnly([WriteOnlyArray] VisibleClassWithHiddenInterfaceOnly[] value)
        {
        }
        public void Get_HiddenDelegate(out HiddenDelegate value)
        {
            value = null;
        }
        public void HiddenEnum_In(HiddenEnum value)
        {
        }
        public void HiddenEnum_Out(out HiddenEnum value)
        {
            value = HiddenEnum.First;
        }
        public void HiddenStruct_In(_HiddenStruct value)
        {
            hiddenStruct.HiddenStructMember = value.HiddenStructMember;
        }
        public _HiddenStruct HiddenStruct_Out()
        {
            return hiddenStruct;
        }
        public void MethodUsing_HiddenClass_In(HiddenClass hiddenInterface)
        {
        }
        public void MethodUsing_HiddenClass_Out(out HiddenClass hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void MethodUsing_HiddenInterface_In(IHiddenInterface hiddenInterface)
        {
        }
        public void MethodUsing_HiddenInterface_Out(out IHiddenInterface hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void MethodUsing_VisibleClassWithDefaultHiddenInterface_In(VisibleClassWithDefaultHiddenInterface hiddenInterface)
        {
        }
        public void MethodUsing_VisibleClassWithDefaultHiddenInterface_Out(out VisibleClassWithDefaultHiddenInterface hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void MethodUsing_VisibleClassWithDefaultVisibleInterface_In(VisibleClassWithDefaultVisibleInterface visibleInterface)
        {
        }
        public void MethodUsing_VisibleClassWithDefaultVisibleInterface_Out(out VisibleClassWithDefaultVisibleInterface visibleInterface)
        {
            visibleInterface = this as IVisibleInterface as VisibleClassWithDefaultVisibleInterface;
        }
        public void MethodUsing_VisibleClassWithHiddenInterfaceOnly_In(VisibleClassWithHiddenInterfaceOnly hiddenInterface)
        {
        }
        public void MethodUsing_VisibleClassWithHiddenInterfaceOnly_Out(out VisibleClassWithHiddenInterfaceOnly hiddenInterface)
        {
            hiddenInterface = null;
        }
        public void PassArray_HiddenClass([ReadOnlyArray] HiddenClass[] value)
        {
        }
        public void PassArray_HiddenInterface([ReadOnlyArray] IHiddenInterface[] value)
        {
        }
        public void PassArray_VisibleClassWithDefaultHiddenInterface([ReadOnlyArray] VisibleClassWithDefaultHiddenInterface[] value)
        {
        }
        public void PassArray_VisibleClassWithDefaultVisibleInterface([ReadOnlyArray] VisibleClassWithDefaultVisibleInterface[] value)
        {
        }
        public void PassArray_VisibleClassWithHiddenInterfaceOnly([ReadOnlyArray]VisibleClassWithHiddenInterfaceOnly[] value)
        {
        }
        public void ReceiveArray_HiddenClass(out HiddenClass[] value)
        {
            value = null;
        }
        public void ReceiveArray_HiddenInterface(out IHiddenInterface[] value)
        {
            value = null;
        }
        public void ReceiveArray_VisibleClassWithDefaultHiddenInterface(out VisibleClassWithDefaultHiddenInterface[] value)
        {
            value = null;
        }
        public void ReceiveArray_VisibleClassWithDefaultVisibleInterface(out VisibleClassWithDefaultVisibleInterface[] value)
        {
            value = null;
        }
        public void ReceiveArray_VisibleClassWithHiddenInterfaceOnly(out VisibleClassWithHiddenInterfaceOnly[] value)
        {
            value = null;
        }
        public void StructWithHiddenInnerStruct_In(_StructWithHiddenInnerStruct value)
        {
            structWithHiddenInnerStruct.VisibleStructMember = value.VisibleStructMember;
            structWithHiddenInnerStruct.HiddenInnerStruct = value.HiddenInnerStruct;
        }
        public void StructWithHiddenInnerStruct_Out(out _StructWithHiddenInnerStruct value)
        {
            value = structWithHiddenInnerStruct;
        }
        public void Vector_HiddenInterface_In(IList<IHiddenInterface> value)
        {
        }
        public void Vector_HiddenInterface_Out(out IList<IHiddenInterface> value)
        {
            value = null;
        }
        public void VisibleMethod()
        {
        }
        public void RequiredHiddenMethod()
        {
        }

        public void HiddenOverload(out HiddenEnum value)
        {
            throw new NotImplementedException();
        }

        public void HiddenOverload(int inValue, out int outValue)
        {
            outValue = inValue;
        }

        public void Vector_HiddenClass_In(IList<HiddenClass> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_HiddenClass_Out(out IList<HiddenClass> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultHiddenInterface_In(IList<VisibleClassWithDefaultHiddenInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultHiddenInterface_Out(out IList<VisibleClassWithDefaultHiddenInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultVisibleInterface_In(IList<VisibleClassWithDefaultVisibleInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithDefaultVisibleInterface_Out(out IList<VisibleClassWithDefaultVisibleInterface> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithHiddenInterfaceOnly_In(IList<VisibleClassWithHiddenInterfaceOnly> value)
        {
            throw new NotImplementedException();
        }

        public void Vector_VisibleClassWithHiddenInterfaceOnly_Out(out IList<VisibleClassWithHiddenInterfaceOnly> value)
        {
            throw new NotImplementedException();
        }
    }
    
    public sealed class VisibleClassWithVisibleInterfaceAndHiddenStaticInterface
    {
        public VisibleClassWithVisibleInterfaceAndHiddenStaticInterface()
        {
            // c# don't support static interface
            throw new NotImplementedException();
        }
    }
}
