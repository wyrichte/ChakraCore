using System;
using System.Collections.Generic;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
namespace Animals
{
    public delegate void StructWithTargetEventHandler(IInterfaceWithMiscEventFormat sender, _StructForStructWithTargetEvent inValue);
    public struct _StructForStructWithTargetEvent
    {
        public int structId;
        public int structData;
        public bool target;
    }
    public interface IInterface1WithEvent
    {
        event Interface1WithEventHandler Event1;
        event Interface1WithEventHandler Event2;
        void InvokeEvent_I1E1(string hString);
        void InvokeEvent_I1E2(string hString);
    }
    public interface IInterface2WithEvent
    {
        Interface2WithEventHandler Handler1 { get; }
        bool WasHandler1Invoked { get; }
        event Interface2WithEventHandler Event21;
        event Interface2WithEventHandler Event3;
        void InvokeDelegate(Interface2WithEventHandler eventhandler, string hString);
        void InvokeEvent_I2E1(string hString);
        void InvokeEvent_I2E3(string hString);
        void onevent2(string hString, out string outString);
    }
    public interface IInterface3WithEvent
    {
        event Interface3WithEventHandler Event31;
        event Interface3WithEventHandler Event5;
        void addEventListener(string hString, out string outString);
        void InvokeEvent_I3E1(string hString);
        void InvokeEvent_I3E5(string hString);
    }
    public interface IInterface4WithEvent : IInterface1WithEvent, IInterface2WithEvent
    {
        event Interface4WithEventHandler Event1;
        void InvokeEvent_I4E1(string hString);
        void onevent1(string hString, out string outString);
    }
    public interface IInterfaceWithMiscEventFormat
    {
        event DelegateEventHandler DelegateEvent;
        event InterfaceWithTargetEventHandler InterfaceWithTargetEvent;
        event StructEventHandler StructEvent;
        event StructWithTargetEventHandler StructWithTargetEvent;
        void InvokeDelegateEvent(DelegateForDelegateEvent inValue);
        void InvokeInterfaceWithTargetEvent();
        void InvokeStructEvent(_StructForStructEvent inValue);
        void InvokeStructWithTargetEvent(_StructForStructWithTargetEvent inValue);
        void target(int inValue, out int outValue);
    }
    public struct _StructForStructEvent
    {
        public int structId;
        public int structData;
    }
    public interface IDummyInterface
    {
    }
    public sealed class RC1WithEvent : IInterface2WithEvent, IDummyInterface //: IInterface1WithEvent, IInterface2WithEvent
    {
        bool m_nativeInvoked;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event21Table;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event3Table;
        static EventRegistrationTokenTable<Interface1StaticWithEventHandler> Event1Table = new EventRegistrationTokenTable<Interface1StaticWithEventHandler>();
        static EventRegistrationTokenTable<Interface1StaticWithEventHandler> Event2Table = new EventRegistrationTokenTable<Interface1StaticWithEventHandler>();
        void Invoke(IInterface2WithEvent sender, string hString)
        {
            ((RC1WithEvent)sender).m_nativeInvoked = true;
        }
        public RC1WithEvent()
        {
            m_nativeInvoked = false;
            Event21Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
            Event3Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
        }
        public event Interface1StaticWithEventHandler Event1
        {
            add
            {
                return Event1Table.AddEventHandler(value);
            }
            remove
            {
                Event1Table.RemoveEventHandler(value);
            }
        }
        public event Interface1StaticWithEventHandler Event2
        {
            add
            {
                return Event2Table.AddEventHandler(value);
            }
            remove
            {
                Event2Table.RemoveEventHandler(value);
            }
        }
        public event Interface2WithEventHandler Event21
        {
            add
            {
                return Event21Table.AddEventHandler(value);
            }
            remove
            {
                Event21Table.RemoveEventHandler(value);
            }
        }
        public event Interface2WithEventHandler Event3
        {
            add
            {
                return Event3Table.AddEventHandler(value);
            }
            remove
            {
                Event3Table.RemoveEventHandler(value);
            }
        }
        public void InvokeEvent_I1E1(string hString)
        {
            if (Event1Table.InvocationList != null)
                Event1Table.InvocationList(new RC1WithEvent(), hString);
        }
        public void InvokeEvent_I1E2(string hString)
        {
            if (Event2Table.InvocationList != null)
                Event2Table.InvocationList(new RC1WithEvent(), hString);
        }
        public Interface2WithEventHandler Handler1
        {
            get { return new Interface2WithEventHandler(this.Invoke); }
        }
        public bool WasHandler1Invoked
        {
            get { return m_nativeInvoked; }
        }
        public void InvokeDelegate(Interface2WithEventHandler eventhandler, string hString)
        {
            if (eventhandler != null)
                eventhandler(this, hString);
        }
        public void InvokeEvent_I2E1(string hString)
        {
            if (Event21Table.InvocationList != null)
                Event21Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I2E3(string hString)
        {
            if (Event3Table.InvocationList != null)
                Event3Table.InvocationList(this, hString);
        }
        public void onevent2(string hString, out string outString)
        {
            outString = hString;
        }
    }
    public sealed class RC2WithEvent : IInterface2WithEvent, IDummyInterface
    {
        EventRegistrationTokenTable<Interface2WithEventHandler> Event2Table;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event3Table;
        static EventRegistrationTokenTable<Interface1StaticWithEventHandler> StaticEvent1Table = new EventRegistrationTokenTable<Interface1StaticWithEventHandler>();
        static EventRegistrationTokenTable<Interface1StaticWithEventHandler> StaticEvent2Table = new EventRegistrationTokenTable<Interface1StaticWithEventHandler>();
        bool m_nativeInvoked;
        void Invoke(IInterface2WithEvent sender, string hString)
        {
            ((RC2WithEvent)sender).m_nativeInvoked = true;
        }
        public RC2WithEvent()
        {
            m_nativeInvoked = false;
            Event2Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
            Event3Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
        }
        public Interface2WithEventHandler Handler1
        {
            get { return new Interface2WithEventHandler(this.Invoke); }
        }
        public bool WasHandler1Invoked
        {
            get { return m_nativeInvoked; }
        }
        public event Interface2WithEventHandler Event21
        {
            add
            {
                return Event2Table.AddEventHandler(value);
            }
            remove
            {
                Event2Table.RemoveEventHandler(value);
            }
        }
        public event Interface2WithEventHandler Event3
        {
            add
            {
                return Event3Table.AddEventHandler(value);
            }
            remove
            {
                Event3Table.RemoveEventHandler(value);
            }
        }
        public void InvokeDelegate(Interface2WithEventHandler eventhandler, string hString)
        {
            if (eventhandler != null)
                eventhandler(this, hString);
        }
        public void InvokeEvent_I2E1(string hString)
        {
            if (Event2Table.InvocationList != null)
                Event2Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I2E3(string hString)
        {
            if (Event3Table.InvocationList != null)
                Event3Table.InvocationList(this, hString);
        }
        public void onevent2(string hString, out string outString)
        {
            outString = hString;
        }
        public static event Interface1StaticWithEventHandler Event1
        {
            add
            {
                return StaticEvent1Table.AddEventHandler(value);
            }
            remove
            {
                StaticEvent1Table.RemoveEventHandler(value);
            }
        }
        public static event Interface1StaticWithEventHandler Event2
        {
            add
            {
                return StaticEvent2Table.AddEventHandler(value);
            }
            remove
            {
                StaticEvent2Table.RemoveEventHandler(value);
            }
        }
        public static void InvokeEvent_I1E1(string hString)
        {
            if (StaticEvent1Table.InvocationList != null)
                StaticEvent1Table.InvocationList(new RC2WithEvent(), hString);
        }
        public static void InvokeEvent_I1E2(string hString)
        {
            if (StaticEvent2Table.InvocationList != null)
                StaticEvent2Table.InvocationList(new RC2WithEvent(), hString);
        }
    }
    public sealed class RC3WithEvent : IInterface3WithEvent, IDummyInterface
    {
        EventRegistrationTokenTable<Interface3WithEventHandler> EventTable31;
        EventRegistrationTokenTable<Interface3WithEventHandler> EventTable5;
        public RC3WithEvent()
        {
            EventTable31 = new EventRegistrationTokenTable<Interface3WithEventHandler>();
            EventTable5 = new EventRegistrationTokenTable<Interface3WithEventHandler>();
        }
        public event Interface3WithEventHandler Event31
        {
            add
            {
                return EventTable31.AddEventHandler(value);
            }
            remove
            {
                EventTable31.RemoveEventHandler(value);
            }
        }
        public event Interface3WithEventHandler Event5
        {
            add
            {
                return EventTable5.AddEventHandler(value);
            }
            remove
            {
                EventTable5.RemoveEventHandler(value);
            }
        }
        public void addEventListener(string hString, out string outString)
        {
            outString = hString;
        }
        public void InvokeEvent_I3E1(string hString)
        {
            if (EventTable31.InvocationList != null)
                EventTable31.InvocationList(this, hString);
        }
        public void InvokeEvent_I3E5(string hString)
        {
            if (EventTable5.InvocationList != null)
                EventTable5.InvocationList(this, hString);
        }
    }
    public sealed class RC4WithEvent : IInterface1WithEvent, IInterface2WithEvent, IInterface3WithEvent, IDummyInterface
    {
        bool m_nativeInvoked;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event21Table;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event3Table;
        EventRegistrationTokenTable<Interface1WithEventHandler> Event1Table;
        EventRegistrationTokenTable<Interface1WithEventHandler> Event2Table;
        EventRegistrationTokenTable<Interface3WithEventHandler> Event31Table;
        EventRegistrationTokenTable<Interface3WithEventHandler> Event5Table;
        void Invoke(IInterface2WithEvent sender, string hString)
        {
            ((RC4WithEvent)sender).m_nativeInvoked = true;
        }
        public RC4WithEvent()
        {
            m_nativeInvoked = false;
            Event21Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
            Event3Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
            Event1Table = new EventRegistrationTokenTable<Interface1WithEventHandler>();
            Event2Table = new EventRegistrationTokenTable<Interface1WithEventHandler>();
            Event31Table = new EventRegistrationTokenTable<Interface3WithEventHandler>();
            Event5Table = new EventRegistrationTokenTable<Interface3WithEventHandler>();
        }
        public event Interface1WithEventHandler Event1
        {
            add { return Event1Table.AddEventHandler(value); }
            remove { Event1Table.RemoveEventHandler(value); }
        }
        public event Interface1WithEventHandler Event2
        {
            add { return Event2Table.AddEventHandler(value); }
            remove { Event2Table.RemoveEventHandler(value); }
        }
        public event Interface2WithEventHandler Event21
        {
            add { return Event21Table.AddEventHandler(value); }
            remove { Event21Table.RemoveEventHandler(value); }
        }
        public event Interface2WithEventHandler Event3
        {
            add { return Event3Table.AddEventHandler(value); }
            remove { Event3Table.RemoveEventHandler(value); }
        }
        public event Interface3WithEventHandler Event31
        {
            add { return Event31Table.AddEventHandler(value); }
            remove { Event31Table.RemoveEventHandler(value); }
        }
        public event Interface3WithEventHandler Event5
        {
            add { return Event5Table.AddEventHandler(value); }
            remove { Event5Table.RemoveEventHandler(value); }
        }
        public void InvokeEvent_I1E1(string hString)
        {
            if (Event1Table.InvocationList != null)
                Event1Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I1E2(string hString)
        {
            if (Event2Table.InvocationList != null)
                Event2Table.InvocationList(this, hString);
        }
        public Interface2WithEventHandler Handler1
        {
            get { return new Interface2WithEventHandler(this.Invoke); }
        }
        public bool WasHandler1Invoked
        {
            get { return m_nativeInvoked; }
        }
        public void InvokeDelegate(Interface2WithEventHandler eventhandler, string hString)
        {
            if (eventhandler != null)
                eventhandler(this, hString);
        }
        public void InvokeEvent_I2E1(string hString)
        {
            if (Event21Table.InvocationList != null)
                Event21Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I2E3(string hString)
        {
            if (Event3Table.InvocationList != null)
                Event3Table.InvocationList(this, hString);
        }
        public void onevent2(string hString, out string outString)
        {
            outString = hString;
        }
        public void addEventListener(string hString, out string outString)
        {
            outString = hString;
        }
        public void InvokeEvent_I3E1(string hString)
        {
            if (Event31Table.InvocationList != null)
                Event31Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I3E5(string hString)
        {
            if (Event5Table.InvocationList != null)
                Event5Table.InvocationList(this, hString);
        }
    }
    public sealed class RC5WithEvent : IInterface1WithEvent, IInterface2WithEvent, IDummyInterface
    {
        bool m_nativeInvoked;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event21Table;
        EventRegistrationTokenTable<Interface2WithEventHandler> Event3Table;
        EventRegistrationTokenTable<Interface1WithEventHandler> Event1Table;
        EventRegistrationTokenTable<Interface1WithEventHandler> Event2Table;
        static EventRegistrationTokenTable<Interface3WithEventHandler> Event31Table = new EventRegistrationTokenTable<Interface3WithEventHandler>();
        static EventRegistrationTokenTable<Interface3WithEventHandler> Event5Table = new EventRegistrationTokenTable<Interface3WithEventHandler>();
        void Invoke(IInterface2WithEvent sender, string hString)
        {
            ((RC5WithEvent)sender).m_nativeInvoked = true;
        }
        public RC5WithEvent()
        {
            m_nativeInvoked = false;
            Event21Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
            Event3Table = new EventRegistrationTokenTable<Interface2WithEventHandler>();
            Event1Table = new EventRegistrationTokenTable<Interface1WithEventHandler>();
            Event2Table = new EventRegistrationTokenTable<Interface1WithEventHandler>();
        }
        public event Interface1WithEventHandler Event1
        {
            add { return Event1Table.AddEventHandler(value); }
            remove { Event1Table.RemoveEventHandler(value); }
        }
        public event Interface1WithEventHandler Event2
        {
            add { return Event2Table.AddEventHandler(value); }
            remove { Event2Table.RemoveEventHandler(value); }
        }
        public event Interface2WithEventHandler Event21
        {
            add { return Event21Table.AddEventHandler(value); }
            remove { Event21Table.RemoveEventHandler(value); }
        }
        public event Interface2WithEventHandler Event3
        {
            add { return Event3Table.AddEventHandler(value); }
            remove { Event3Table.RemoveEventHandler(value); }
        }
        public static event Interface3WithEventHandler Event31
        {
            add { return Event31Table.AddEventHandler(value); }
            remove { Event31Table.RemoveEventHandler(value); }
        }
        public static event Interface3WithEventHandler Event5
        {
            add { return Event5Table.AddEventHandler(value); }
            remove { Event5Table.RemoveEventHandler(value); }
        }
        public void InvokeEvent_I1E1(string hString)
        {
            if (Event1Table.InvocationList != null)
                Event1Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I1E2(string hString)
        {
            if (Event2Table.InvocationList != null)
                Event2Table.InvocationList(this, hString);
        }
        public Interface2WithEventHandler Handler1
        {
            get { return new Interface2WithEventHandler(this.Invoke); }
        }
        public bool WasHandler1Invoked
        {
            get { return m_nativeInvoked; }
        }
        public void InvokeDelegate(Interface2WithEventHandler eventhandler, string hString)
        {
            if (eventhandler != null)
                eventhandler(this, hString);
        }
        public void InvokeEvent_I2E1(string hString)
        {
            if (Event21Table.InvocationList != null)
                Event21Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I2E3(string hString)
        {
            if (Event31Table.InvocationList != null)
                Event31Table.InvocationList(this, hString);
        }
        public void onevent2(string hString, out string outString)
        {
            outString = hString;
        }
        public static void addEventListener(string hString, out string outString)
        {
            outString = hString;
        }
        public static void InvokeEvent_I3E1(string hString)
        {
            if (Event31Table.InvocationList != null)
                Event31Table.InvocationList(new RC5WithEvent(), hString);
        }
        public static void InvokeEvent_I3E5(string hString)
        {
            if (Event5Table.InvocationList != null)
                Event5Table.InvocationList(new RC5WithEvent(), hString);
        }
    }
    public sealed class RC6WithEvent : IInterface3WithEvent, IInterface4WithEvent, IDummyInterface
    {
        event Interface1WithEventHandler Event1Handler;
        EventRegistrationTokenTable<Interface1WithEventHandler> Event1Table;
        event Interface4WithEventHandler Event4Handler;
        EventRegistrationTokenTable<Interface4WithEventHandler> Event4Table;
        bool m_nativeInvoked;
        void Invoke(IInterface2WithEvent sender, string hString)
        {
            ((RC6WithEvent)sender).m_nativeInvoked = true;
        }
        public RC6WithEvent()
        {
            m_nativeInvoked = false;
            Event1Table = new EventRegistrationTokenTable<Interface1WithEventHandler>();
            Event4Table = new EventRegistrationTokenTable<Interface4WithEventHandler>();
        }
        public event Interface3WithEventHandler Event31;
        public event Interface3WithEventHandler Event5;
        public void addEventListener(string hString, out string outString)
        {
            outString = hString;
        }
        public void InvokeEvent_I3E1(string hString)
        {
            if (Event31 != null)
                Event31(this, hString);
        }
        public void InvokeEvent_I3E5(string hString)
        {
            if (Event5 != null)
                Event5(this, hString);
        }
        public void InvokeEvent_I4E1(string hString)
        {
            if (Event4Handler != null)
                Event4Handler(this, hString);
        }
        public void onevent1(string hString, out string outString)
        {
            outString = hString;
        }
        event Interface1WithEventHandler IInterface1WithEvent.Event1
        {
            add
            {
                return Event1Table.AddEventHandler(value);
            }
            remove
            {
                Event1Table.RemoveEventHandler(value);
            }
        }
        event Interface4WithEventHandler IInterface4WithEvent.Event1
        {
            add
            {
                return Event4Table.AddEventHandler(value);
            }
            remove
            {
                Event4Table.RemoveEventHandler(value);
            }
        }
        public event Interface1WithEventHandler Event2;
        public void InvokeEvent_I1E1(string hString)
        {
            if (Event1Table.InvocationList != null)
                Event1Table.InvocationList(this, hString);
        }
        public void InvokeEvent_I1E2(string hString)
        {
            if (Event2 != null)
                Event2(this, hString);
        }
        public Interface2WithEventHandler Handler1
        {
            get { return new Interface2WithEventHandler(this.Invoke); }
        }
        public bool WasHandler1Invoked
        {
            get { return m_nativeInvoked; }
        }
        public event Interface2WithEventHandler Event21;
        public event Interface2WithEventHandler Event3;
        public void InvokeDelegate(Interface2WithEventHandler eventhandler, string hString)
        {
            if (eventhandler != null)
                eventhandler(this, hString);
        }
        public void InvokeEvent_I2E1(string hString)
        {
            if (Event21 != null)
                Event21(this, hString);
        }
        public void InvokeEvent_I2E3(string hString)
        {
            if (Event3 != null)
                Event3(this, hString);
        }
        public void onevent2(string hString, out string outString)
        {
            outString = hString;
        }
    }
    public interface IInterfaceWithOnEvent1
    {
        void onevent1(string hString, out string outString);
    }
    public sealed class RC7WithEvent : IInterface1WithEvent, IInterfaceWithOnEvent1
    {
        public RC7WithEvent()
        {
        }
        public event Interface1WithEventHandler Event1;
        public event Interface1WithEventHandler Event2;
        public void InvokeEvent_I1E1(string hString)
        {
            if (Event1 != null)
                Event1(this, hString);
        }
        public void InvokeEvent_I1E2(string hString)
        {
            if (Event2 != null)
                Event2(this, hString);
        }
        public void onevent1(string hString, out string outString)
        {
            outString = hString;
        }
    }
    public sealed class RC8WithEvent : IInterfaceWithMiscEventFormat
    {
        public event DelegateEventHandler DelegateEvent;
        public event InterfaceWithTargetEventHandler InterfaceWithTargetEvent;
        public event StructEventHandler StructEvent;
        public event StructWithTargetEventHandler StructWithTargetEvent;
        public void InvokeDelegateEvent(DelegateForDelegateEvent inValue)
        {
            if (DelegateEvent != null)
                DelegateEvent(this, inValue);
        }
        public void InvokeInterfaceWithTargetEvent()
        {
            if (InterfaceWithTargetEvent != null)
                InterfaceWithTargetEvent(this, this);
        }
        public void InvokeStructEvent(_StructForStructEvent inValue)
        {
            if (StructEvent != null)
                StructEvent(this, inValue);
        }
        public void InvokeStructWithTargetEvent(_StructForStructWithTargetEvent inValue)
        {
            if (StructWithTargetEvent != null)
                StructWithTargetEvent(this, inValue);
        }
        public void target(int inValue, out int outValue)
        {
            outValue = inValue;
        }
    }
}
