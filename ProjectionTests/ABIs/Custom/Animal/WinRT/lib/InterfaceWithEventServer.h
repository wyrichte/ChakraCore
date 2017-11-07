#pragma once

namespace Animals
{
    using namespace Microsoft::WRL;

    class CIInterface2WithEvent;
    class CInterface2WithEventHandler : public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
        IInterface2WithEventHandler>
    {
        CIInterface2WithEvent *m_sender;

    public:
        CInterface2WithEventHandler(CIInterface2WithEvent *sender) : m_sender(sender)
        {
        }

        IFACEMETHOD(Invoke)(__in IInterface2WithEvent *sender, __in HSTRING) override;
    };

    class CIInterface1WithEvent : public Microsoft::WRL::Implements<IInterface1WithEvent>
    {
    private:
        EventSource<IInterface1WithEventHandler> _evtEvent1;
        EventSource<IInterface1WithEventHandler> _evtEvent2;

    public:
        CIInterface1WithEvent() { }
        ~CIInterface1WithEvent() { }

        IFACEMETHOD(add_Event1)(__in Animals::IInterface1WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent1.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event1)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent1.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I1E1)(__in HSTRING hString) 
        {
			_evtEvent1.InvokeAll(this, hString);
			return S_OK;
		};
        
        IFACEMETHOD(add_Event2)(__in Animals::IInterface1WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent2.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event2)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent2.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I1E2)(__in HSTRING hString) 
        {
			_evtEvent2.InvokeAll(this, hString);
			return S_OK;
		};
    };

    class CIInterface2WithEvent : public Implements<IInterface2WithEvent>
    {
    private:
        Microsoft::WRL::EventSource<IInterface2WithEventHandler> _evtEvent1;
        Microsoft::WRL::EventSource<IInterface2WithEventHandler> _evtEvent3;

    public:
        boolean m_nativeInvoked;

        CIInterface2WithEvent() 
        { 
            m_nativeInvoked = false; 
        }

        ~CIInterface2WithEvent() { }

        IFACEMETHOD(add_Event21)(__in Animals::IInterface2WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent1.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event21)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent1.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I2E1)(__in HSTRING hString) 
        {
			_evtEvent1.InvokeAll(this, hString);
			return S_OK;
		};
        
        IFACEMETHOD(add_Event3)(__in Animals::IInterface2WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent3.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event3)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent3.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I2E3)(__in HSTRING hString) 
        {
			_evtEvent3.InvokeAll(this, hString);
			return S_OK;
		};

        IFACEMETHOD(onevent2)(__in HSTRING hString, __out HSTRING *outString) 
        {
            WindowsDuplicateString(hString, outString);
            return S_OK;
		};

        IFACEMETHOD(get_Handler1)(__out IInterface2WithEventHandler **eventhandler)
        {
            if (eventhandler == nullptr)
            {
                return E_POINTER;
            }

            auto out = Make<CInterface2WithEventHandler>(this);
            *eventhandler = out.Detach();
            return S_OK;
        }

        IFACEMETHOD(get_WasHandler1Invoked)(__out boolean *nativeInvoked)
        {
            if (nativeInvoked == nullptr)
            {
                return E_POINTER;
            }

            *nativeInvoked = m_nativeInvoked;
            return S_OK;
        }

        IFACEMETHOD(InvokeDelegate)(__in IInterface2WithEventHandler *eventhandler, __in HSTRING hString)
        {
            if (eventhandler == nullptr)
            {
                return E_POINTER;
            }

            IInterface2WithEvent *sender = nullptr;
            HRESULT hr = this->QueryInterface(&sender);
            if (FAILED(hr))
            {
                return hr;
            }

            hr = eventhandler->Invoke(sender, hString);
            sender->Release();

            return hr;
        }
    };

    class CIInterface3WithEvent : public Implements<IInterface3WithEvent>
    {
    private:
        Microsoft::WRL::EventSource<IInterface3WithEventHandler> _evtEvent1;
        Microsoft::WRL::EventSource<IInterface3WithEventHandler> _evtEvent5;

    public:
        CIInterface3WithEvent() { }
        ~CIInterface3WithEvent() { }

        IFACEMETHOD(add_Event31)(__in Animals::IInterface3WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent1.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event31)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent1.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I3E1)(__in HSTRING hString) 
        {
			_evtEvent1.InvokeAll(this, hString);
			return S_OK;
		};
        
        IFACEMETHOD(add_Event5)(__in Animals::IInterface3WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent5.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event5)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent5.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I3E5)(__in HSTRING hString) 
        {
			_evtEvent5.InvokeAll(this, hString);
			return S_OK;
		};

        IFACEMETHOD(addEventListener)(__in HSTRING hString, __out HSTRING *outString) 
        {
            WindowsDuplicateString(hString, outString);
            return S_OK;
		};
    };

    class CIInterface4WithEvent : public Implements<IInterface4WithEvent>
    {
    private:
        Microsoft::WRL::EventSource<IInterface4WithEventHandler> _evtEvent1;

    public:
        CIInterface4WithEvent() { }
        ~CIInterface4WithEvent() { }

        IFACEMETHOD(add_Event1)(__in Animals::IInterface4WithEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtEvent1.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_Event1)(__in EventRegistrationToken iCookie)
        {
            return _evtEvent1.Remove(iCookie);
        }

        IFACEMETHOD(InvokeEvent_I4E1)(__in HSTRING hString) 
        {
			_evtEvent1.InvokeAll(this, hString);
			return S_OK;
		};

        IFACEMETHOD(onevent1)(__in HSTRING hString, __out HSTRING *outString) 
        {
            WindowsDuplicateString(hString, outString);
            return S_OK;
		};
    };

    class CIInterfaceWithOnEvent1 : public Implements<IInterfaceWithOnEvent1>
    {

    public:
        CIInterfaceWithOnEvent1() { }
        ~CIInterfaceWithOnEvent1() { }

        IFACEMETHOD(onevent1)(__in HSTRING hString, __out HSTRING *outString) 
        {
            WindowsDuplicateString(hString, outString);
            return S_OK;
		};
    };

    class CIInterfaceWithMiscEventFormat : public Implements<IInterfaceWithMiscEventFormat>
    {
    private:
        Microsoft::WRL::EventSource<IDelegateEventHandler> _evtDelegateEvent;
        Microsoft::WRL::EventSource<IStructEventHandler> _evtStructEvent;
        Microsoft::WRL::EventSource<IInterfaceWithTargetEventHandler> _evtInterfaceWithTargetEvent;
        Microsoft::WRL::EventSource<IStructWithTargetEventHandler> _evtStructWithTargetEvent;
        
    public:
        CIInterfaceWithMiscEventFormat() { }
        ~CIInterfaceWithMiscEventFormat() { }

        IFACEMETHOD(add_DelegateEvent)(__in Animals::IDelegateEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtDelegateEvent.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_DelegateEvent)(__in EventRegistrationToken iCookie)
        {
            return _evtDelegateEvent.Remove(iCookie);
        }

        IFACEMETHOD(InvokeDelegateEvent)(__in IDelegateForDelegateEvent *inValue) 
        {
			_evtDelegateEvent.InvokeAll(this, inValue);
			return S_OK;
		};

        IFACEMETHOD(add_StructEvent)(__in Animals::IStructEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtStructEvent.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_StructEvent)(__in EventRegistrationToken iCookie)
        {
            return _evtStructEvent.Remove(iCookie);
        }

        IFACEMETHOD(InvokeStructEvent)(__in StructForStructEvent inValue) 
        {
			_evtStructEvent.InvokeAll(this, inValue);
			return S_OK;
		};

        IFACEMETHOD(add_InterfaceWithTargetEvent)(__in Animals::IInterfaceWithTargetEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtInterfaceWithTargetEvent.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_InterfaceWithTargetEvent)(__in EventRegistrationToken iCookie)
        {
            return _evtInterfaceWithTargetEvent.Remove(iCookie);
        }

        IFACEMETHOD(InvokeInterfaceWithTargetEvent)() 
        {
			_evtInterfaceWithTargetEvent.InvokeAll(this, this);
			return S_OK;
		};

        IFACEMETHOD(add_StructWithTargetEvent)(__in Animals::IStructWithTargetEventHandler *eventHandler, __out EventRegistrationToken *pCookie)
        {
            return _evtStructWithTargetEvent.Add(eventHandler, pCookie);
        }

        IFACEMETHOD(remove_StructWithTargetEvent)(__in EventRegistrationToken iCookie)
        {
            return _evtStructWithTargetEvent.Remove(iCookie);
        }

        IFACEMETHOD(InvokeStructWithTargetEvent)(__in StructForStructWithTargetEvent inValue) 
        {
			_evtStructWithTargetEvent.InvokeAll(this, inValue);
			return S_OK;
		};

        IFACEMETHOD(target)(__in int inValue, __out int *outValue) 
        {
            if (outValue == NULL)
            {
                return E_POINTER;
            }
            *outValue = inValue;
            return S_OK;
		};
    };

    class RC1WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface1WithEvent, CIInterface2WithEvent>
    {
        InspectableClass(L"Animals.RC1WithEvent", BaseTrust);

    public:
        RC1WithEventServer() { }
        ~RC1WithEventServer() { }
    };

    class RC2WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface2WithEvent>
    {
        InspectableClass(L"Animals.RC2WithEvent", BaseTrust);

    public:
        RC2WithEventServer() { }
        ~RC2WithEventServer() { }
    };

    class RC2WithEventFactory :
		public Microsoft::WRL::ActivationFactory<CIInterface1WithEvent>
	{
	public:
        // IActivationFactory
		IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable) {
			HRESULT hr = S_OK;

            ComPtr<IInspectable> spI2;
			*ppInspectable = nullptr;
			ComPtr<RC2WithEventServer> spObj = Make<RC2WithEventServer>();

			hr = spObj.As(&spI2);
			if (SUCCEEDED(hr))
			{
				*ppInspectable = spI2.Detach();
			}

			return hr;
		};
	};

    class RC3WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface3WithEvent>
    {
        InspectableClass(L"Animals.RC3WithEvent", BaseTrust);

    public:
        RC3WithEventServer() { }
        ~RC3WithEventServer() { }
    };

    class RC4WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface1WithEvent, CIInterface2WithEvent, CIInterface3WithEvent>
    {
        InspectableClass(L"Animals.RC4WithEvent", BaseTrust);

    public:
        RC4WithEventServer() { }
        ~RC4WithEventServer() { }
    };

    class RC5WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface1WithEvent, CIInterface2WithEvent>
    {
        InspectableClass(L"Animals.RC5WithEvent", BaseTrust);

    public:
        RC5WithEventServer() { }
        ~RC5WithEventServer() { }
    };

    class RC5WithEventFactory :
		public Microsoft::WRL::ActivationFactory<CIInterface3WithEvent>
	{
	public:
        // IActivationFactory
		IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable) {
			HRESULT hr = S_OK;

            ComPtr<IInspectable> spI2;
			*ppInspectable = nullptr;
			ComPtr<RC5WithEventServer> spObj = Make<RC5WithEventServer>();

			hr = spObj.As(&spI2);
			if (SUCCEEDED(hr))
			{
				*ppInspectable = spI2.Detach();
			}

			return hr;
		};
	};

    class RC6WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface1WithEvent, CIInterface2WithEvent, CIInterface3WithEvent, CIInterface4WithEvent>
    {
        InspectableClass(L"Animals.RC6WithEvent", BaseTrust);

    public:
        RC6WithEventServer() { }
        ~RC6WithEventServer() { }
    };

    class RC7WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterface1WithEvent, CIInterfaceWithOnEvent1>
    {
        InspectableClass(L"Animals.RC7WithEvent", BaseTrust);

    public:
        RC7WithEventServer() { }
        ~RC7WithEventServer() { }
    };

    class RC8WithEventServer : public Microsoft::WRL::RuntimeClass<CIInterfaceWithMiscEventFormat>
    {
        InspectableClass(L"Animals.RC8WithEvent", BaseTrust);

    public:
        RC8WithEventServer() { }
        ~RC8WithEventServer() { }
    };
}
