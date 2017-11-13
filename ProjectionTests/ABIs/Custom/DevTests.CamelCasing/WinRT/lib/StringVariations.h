//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

namespace DevTests 
{
    namespace CamelCasing
    {
        inline HSTRING hs(LPCWSTR sz)
        {
            HSTRING hs;
            WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
            return hs;
        }

        inline HSTRING hsDup(HSTRING hs)
        {
            HSTRING hsResult;
            WindowsDuplicateString(hs, &hsResult);
            return hsResult;
        }

        class StringVariationsFactory : 
            public Microsoft::WRL::ActivationFactory<
                ICasingStatic,
                IPrivateStatic,
                IStaticStringVariations>
        {
        private:
            int m_PascalStaticPropertyWritable;
            int m_ALLUPPERCASESTATICPROPERTYWRITABLE;
            int m_camelStaticPropertyWritable;
            int m__PrivatePascalStaticPropertyWritable;
            int m__IInterfacePrivateStaticPropertyWritable;
            int m_NONCASED_CHARSTATIC;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtPascalStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtUPPERCASEStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtIInterfaceStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtCamelStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtPascalPrivateStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtUPPERCASEPrivateStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtIInterfacePrivateStatic;
            Microsoft::WRL::EventSource<IStaticCamelCasingHandler> _evtCamelPrivateStatic;


        public:
            StringVariationsFactory() : 
                m_PascalStaticPropertyWritable(0), 
                m_ALLUPPERCASESTATICPROPERTYWRITABLE(0), 
                m_camelStaticPropertyWritable(0),
                m__PrivatePascalStaticPropertyWritable(0),
                m__IInterfacePrivateStaticPropertyWritable(0),
                m_NONCASED_CHARSTATIC(0)
            {
            }

            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

            // ICasingStatic
            HRESULT STDMETHODCALLTYPE add_PascalStaticEvent(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtPascalStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_PascalStaticEvent(__in EventRegistrationToken iCookie)
            { return _evtPascalStatic.Remove(iCookie); }                       

            HRESULT STDMETHODCALLTYPE add_UPPERCASESTATICEVENT(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtUPPERCASEStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_UPPERCASESTATICEVENT(__in EventRegistrationToken iCookie)
            { return _evtUPPERCASEStatic.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add_IInterfaceStaticEvent(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtIInterfaceStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_IInterfaceStaticEvent(__in EventRegistrationToken iCookie)
            { return _evtIInterfaceStatic.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add_camelStaticEvent(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtCamelStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_camelStaticEvent(__in EventRegistrationToken iCookie)
            { return _evtCamelStatic.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE get_PascalStaticProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasingStatic.PascalStaticProperty"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_PascalStaticPropertyWritable(__out int * value)
            { *value = m_PascalStaticPropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_PascalStaticPropertyWritable(__in int value)
            { m_PascalStaticPropertyWritable = value; return S_OK; }

            HRESULT STDMETHODCALLTYPE get_ALLUPPERCASESTATICPROPERTYWRITABLE(__out int * value)
            { *value = m_ALLUPPERCASESTATICPROPERTYWRITABLE; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_ALLUPPERCASESTATICPROPERTYWRITABLE(__in int value)
            { m_ALLUPPERCASESTATICPROPERTYWRITABLE = value; return S_OK; }
                       
            HRESULT STDMETHODCALLTYPE get_IInterfaceStaticProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasingStatic.IInterfaceStaticProperty"); return S_OK; }
            
            HRESULT STDMETHODCALLTYPE get_camelStaticPropertyWritable(__out int * value)
            { *value = m_camelStaticPropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_camelStaticPropertyWritable(__in int value)
            { m_camelStaticPropertyWritable = value; return S_OK; }

            HRESULT STDMETHODCALLTYPE PascalStaticMethod(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.PascalStaticMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE ALLUPPERCASESTATICMETHOD(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.ALLUPPERCASESTATICMETHOD() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE IInterfaceStaticMethod(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.IInterfaceStaticMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE camelStaticMethod(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.camelStaticMethod() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE F11(HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.F11() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_SMSReceived(HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasingStatic.SMSReceived"); return S_OK; }
            HRESULT STDMETHODCALLTYPE Y4Cb2Cr0(HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.Y4Cb2Cr0() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_UTF8(HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasingStatic.UTF8"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_NONCASED_CHARSTATIC(int * value)
            { *value = m_NONCASED_CHARSTATIC; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_NONCASED_CHARSTATIC(int value)
            { m_NONCASED_CHARSTATIC = value; return S_OK; }
            HRESULT STDMETHODCALLTYPE MSTwoLetterAcronymMethod(HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasingStatic.MSTwoLetterAcronymMethod() Called"); return S_OK; }

            // IPrivateStatic
            HRESULT STDMETHODCALLTYPE add__PrivatePascalStaticEvent(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtPascalPrivateStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__PrivatePascalStaticEvent(__in EventRegistrationToken iCookie)
            { return _evtPascalPrivateStatic.Remove(iCookie); }                       

            HRESULT STDMETHODCALLTYPE add__PRIVATEUPPERCASESTATICEVENT(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtUPPERCASEPrivateStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__PRIVATEUPPERCASESTATICEVENT(__in EventRegistrationToken iCookie)
            { return _evtUPPERCASEPrivateStatic.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add__IInterfacePrivateStaticEvent(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtIInterfacePrivateStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__IInterfacePrivateStaticEvent(__in EventRegistrationToken iCookie)
            { return _evtIInterfacePrivateStatic.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add__privateCamelStaticEvent(__in IStaticCamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtCamelPrivateStatic.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__privateCamelStaticEvent(__in EventRegistrationToken iCookie)
            { return _evtCamelPrivateStatic.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE get__PrivatePascalStaticPropertyWritable(__out int * value)
            { *value = m__PrivatePascalStaticPropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put__PrivatePascalStaticPropertyWritable(__in int value)
            { m__PrivatePascalStaticPropertyWritable = value; return S_OK; }
            
            HRESULT STDMETHODCALLTYPE get__PRIVATEUPPERCASESTATICPROPERTY(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.IPrivateStatic._PRIVATEUPPERCASESTATICPROPERTY"); return S_OK; }

            HRESULT STDMETHODCALLTYPE get__IInterfacePrivateStaticPropertyWritable(__out int * value)
            { *value = m__IInterfacePrivateStaticPropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put__IInterfacePrivateStaticPropertyWritable(__in int value)
            { m__IInterfacePrivateStaticPropertyWritable = value; return S_OK; }

            HRESULT STDMETHODCALLTYPE get__privateCamelStaticProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.IPrivateStatic._privateCamelStaticProperty"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PrivatePascalStaticMethod(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.IPrivateStatic._PrivatePascalStaticMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE _privateCamelStaticMethod(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.IPrivateStatic._privateCamelStaticMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE _PRIVATEUPPERCASESTATICMETHOD(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.IPrivateStatic._PRIVATEUPPERCASESTATICMETHOD() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE _IInterfacePrivateStaticMethod(__out HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.IPrivateStatic._IInterfacePrivateStaticMethod() Called"); return S_OK; }

            // IStaticStringVariations
            IFACEMETHOD(StaticFireEvent)(__in StaticStringVariationsEvent evt);
        };

        class StringVariationsServer :
            public Microsoft::WRL::RuntimeClass<
                DevTests::CamelCasing::ICasing,
                DevTests::CamelCasing::IPrivate,
                DevTests::CamelCasing::IStringVariations
            >
        {
            InspectableClass(L"DevTests.CamelCasing.StringVariations", BaseTrust);

        private:
            StructStringVariations m_Struct;
            int m_PascalPropertyWritable;
            int m_IInterfacePropertyWritable;
            int m__PRIVATEUPPERCASEPROPERTYWRITABLE;
            int m__privateCamelPropertyWritable;
            int m_NONCASED_CHAR;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtPascal;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtUPPERCASE;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtIInterface;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtCamel;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtPascalPrivate;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtUPPERCASEPrivate;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtIInterfacePrivate;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtCamelPrivate;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtF8;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtECDH521;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtUInt16;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtNONCASED;
            Microsoft::WRL::EventSource<ICamelCasingHandler> _evtTwoLetter;


        public:
            StringVariationsServer() : 
                m_PascalPropertyWritable(0), 
                m_IInterfacePropertyWritable(0), 
                m__PRIVATEUPPERCASEPROPERTYWRITABLE(0),
                m__privateCamelPropertyWritable(0),
                m_NONCASED_CHAR(0)
            {
                m_Struct.PascalField = hs(L"StructStringVariations.PascalField");
                m_Struct.UPPERCASEFIELD = hs(L"StructStringVariations.UPPERCASEFIELD");
                m_Struct.IInterfaceField = hs(L"StructStringVariations.IInterfaceField");
                m_Struct.camelField = hs(L"StructStringVariations.camelField");
                
                m_Struct._PrivatePascalField = hs(L"StructStringVariations._PrivatePascalField");
                m_Struct._PRIVATEUPPERCASEFIELD = hs(L"StructStringVariations._PRIVATEUPPERCASEFIELD");
                m_Struct._IInterfacePrivateField = hs(L"StructStringVariations._IInterfacePrivateField");
                m_Struct._privateCamelField = hs(L"StructStringVariations._privateCamelField");

                m_Struct.F12 = hs(L"StructStringVariations.F12");
                m_Struct.HD1080p = hs(L"StructStringVariations.HD1080p");
                m_Struct.SP800108CtrHmacMd5 = hs(L"StructStringVariations.SP800108CtrHmacMd5");
                m_Struct.UInt32 = hs(L"StructStringVariations.UInt32");
                m_Struct.NONCASED_CHAR = hs(L"StructStringVariations.NONCASED_CHAR");
                m_Struct.IPTwoLetterAcronym = hs(L"StructStringVariations.IPTwoLetterAcronym");
            }
            ~StringVariationsServer()
            {
                WindowsDeleteString(m_Struct.PascalField);
                WindowsDeleteString(m_Struct.UPPERCASEFIELD);
                WindowsDeleteString(m_Struct.IInterfaceField);
                WindowsDeleteString(m_Struct.camelField);
               
                WindowsDeleteString(m_Struct._PrivatePascalField);
                WindowsDeleteString(m_Struct._PRIVATEUPPERCASEFIELD);
                WindowsDeleteString(m_Struct._IInterfacePrivateField);
                WindowsDeleteString(m_Struct._privateCamelField);

                WindowsDeleteString(m_Struct.F12);
                WindowsDeleteString(m_Struct.HD1080p);
                WindowsDeleteString(m_Struct.SP800108CtrHmacMd5);
                WindowsDeleteString(m_Struct.UInt32);
                WindowsDeleteString(m_Struct.NONCASED_CHAR);
            }                                

            // ICasing Members
            HRESULT STDMETHODCALLTYPE get_Struct(__out StructStringVariations * value) 
            { 
                if (value == nullptr)
                {
                    return E_POINTER;
                }

                value->PascalField = hsDup(m_Struct.PascalField);
                value->UPPERCASEFIELD = hsDup(m_Struct.UPPERCASEFIELD);
                value->IInterfaceField = hsDup(m_Struct.IInterfaceField);
                value->camelField = hsDup(m_Struct.camelField);
                value->_PrivatePascalField = hsDup(m_Struct._PrivatePascalField);
                value->_PRIVATEUPPERCASEFIELD = hsDup(m_Struct._PRIVATEUPPERCASEFIELD);
                value->_IInterfacePrivateField = hsDup(m_Struct._IInterfacePrivateField);
                value->_privateCamelField = hsDup(m_Struct._privateCamelField);
                value->F12 = hsDup(m_Struct.F12);
                value->HD1080p = hsDup(m_Struct.HD1080p);
                value->SP800108CtrHmacMd5 = hsDup(m_Struct.SP800108CtrHmacMd5);
                value->UInt32 = hsDup(m_Struct.UInt32);
                value->NONCASED_CHAR = hsDup(m_Struct.NONCASED_CHAR);
                value->IPTwoLetterAcronym = hsDup(m_Struct.IPTwoLetterAcronym);

                return S_OK; 
            }

            HRESULT STDMETHODCALLTYPE add_PascalEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtPascal.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_PascalEvent(__in EventRegistrationToken iCookie)
            { return _evtPascal.Remove(iCookie); }                       

            HRESULT STDMETHODCALLTYPE add_UPPERCASEEVENT(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtUPPERCASE.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_UPPERCASEEVENT(__in EventRegistrationToken iCookie)
            { return _evtUPPERCASE.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add_IInterfaceEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtIInterface.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_IInterfaceEvent(__in EventRegistrationToken iCookie)
            { return _evtIInterface.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add_camelEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtCamel.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_camelEvent(__in EventRegistrationToken iCookie)
            { return _evtCamel.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE add_F8Event(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtF8.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_F8Event(__in EventRegistrationToken iCookie)
            { return _evtF8.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE add_ECDH521Event(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtECDH521.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_ECDH521Event(__in EventRegistrationToken iCookie)
            { return _evtECDH521.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE add_UInt16Event(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtUInt16.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_UInt16Event(__in EventRegistrationToken iCookie)
            { return _evtUInt16.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE add_NONCASED_EVENT(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtNONCASED.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_NONCASED_EVENT(__in EventRegistrationToken iCookie)
            { return _evtNONCASED.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE add_UITwoLetterAcronymEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtTwoLetter.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove_UITwoLetterAcronymEvent(__in EventRegistrationToken iCookie)
            { return _evtTwoLetter.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE get_PascalProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasing.PascalProperty"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_PascalPropertyWritable(__out int * value)
            { *value = m_PascalPropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_PascalPropertyWritable(__in int value)
            { m_PascalPropertyWritable = value; return S_OK; }

            HRESULT STDMETHODCALLTYPE get_ALLUPPERCASEPROPERTY(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasing.ALLUPPERCASEPROPERTY"); return S_OK; }
                                      
            HRESULT STDMETHODCALLTYPE get_IInterfacePropertyWritable(__out int * value)
            { *value = m_IInterfacePropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_IInterfacePropertyWritable(__in int value)
            { m_IInterfacePropertyWritable = value; return S_OK; }
             
            HRESULT STDMETHODCALLTYPE get_camelProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasing.camelProperty"); return S_OK; }

            HRESULT STDMETHODCALLTYPE PascalNotationMethod(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.ICasing.PascalNotationMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE ALLUPPERCASEMETHOD(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.ICasing.ALLUPPERCASEMETHOD() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE IInterfaceMethod(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.ICasing.IInterfaceMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE camelCaseMethod(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.ICasing.camelCaseMethod() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE F5(HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasing.F5() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_HD720p(HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasing.HD720p"); return S_OK; }
            HRESULT STDMETHODCALLTYPE SP80056aConcatMd5(HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasing.SP80056aConcatMd5() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_UInt8Array(HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.ICasing.UInt8Array"); return S_OK; }
            HRESULT STDMETHODCALLTYPE get_NONCASED_CHAR(int * value)
            { *value = m_NONCASED_CHAR; return S_OK; }
            HRESULT STDMETHODCALLTYPE put_NONCASED_CHAR(int value)
            { m_NONCASED_CHAR = value; return S_OK; }
            HRESULT STDMETHODCALLTYPE AITwoLetterAcronymMethod(HSTRING * result)
            { *result = hs(L"DevTests.CamelCasing.ICasing.AITwoLetterAcronymMethod() Called"); return S_OK; }

            // IPrivate Members
            HRESULT STDMETHODCALLTYPE add__PrivatePascalEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtPascalPrivate.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__PrivatePascalEvent(__in EventRegistrationToken iCookie)
            { return _evtPascalPrivate.Remove(iCookie); }                       

            HRESULT STDMETHODCALLTYPE add__PRIVATEUPPERCASEEVENT(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtUPPERCASEPrivate.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__PRIVATEUPPERCASEEVENT(__in EventRegistrationToken iCookie)
            { return _evtUPPERCASEPrivate.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add__IInterfacePrivateEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtIInterfacePrivate.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__IInterfacePrivateEvent(__in EventRegistrationToken iCookie)
            { return _evtIInterfacePrivate.Remove(iCookie); }                        
                                      
            HRESULT STDMETHODCALLTYPE add__privateCamelEvent(__in ICamelCasingHandler * clickHandler, __out EventRegistrationToken * pCookie)
            { return _evtCamelPrivate.Add(clickHandler, pCookie); }                        
            HRESULT STDMETHODCALLTYPE remove__privateCamelEvent(__in EventRegistrationToken iCookie)
            { return _evtCamelPrivate.Remove(iCookie); }

            HRESULT STDMETHODCALLTYPE get__PrivatePascalProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.IPrivate._PrivatePascalProperty"); return S_OK; }
                                  
            HRESULT STDMETHODCALLTYPE get__PRIVATEUPPERCASEPROPERTYWRITABLE(__out int * value)
            { *value = m__PRIVATEUPPERCASEPROPERTYWRITABLE; return S_OK; }
            HRESULT STDMETHODCALLTYPE put__PRIVATEUPPERCASEPROPERTYWRITABLE(__in int value)
            { m__PRIVATEUPPERCASEPROPERTYWRITABLE = value; return S_OK; }
                                
            HRESULT STDMETHODCALLTYPE get__IInterfacePrivateProperty(__out HSTRING * value)
            { *value = hs(L"DevTests.CamelCasing.IPrivate._IInterfacePrivateProperty"); return S_OK; }
                             
            HRESULT STDMETHODCALLTYPE get__privateCamelPropertyWritable(__out int * value)
            { *value = m__privateCamelPropertyWritable; return S_OK; }
            HRESULT STDMETHODCALLTYPE put__privateCamelPropertyWritable(__in int value)
            { m__privateCamelPropertyWritable = value; return S_OK; }

            HRESULT STDMETHODCALLTYPE _PrivatePascalMethod(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivate._PrivatePascalMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE _PRIVATEUPPERCASEMETHOD(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivate._PRIVATEUPPERCASEMETHOD() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE _IInterfacePrivateMethod(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivate._IInterfacePrivateMethod() Called"); return S_OK; }
            HRESULT STDMETHODCALLTYPE _privateCamelMethod(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivate._privateCamelMethod() Called"); return S_OK; }

            // IStringVariations
            IFACEMETHOD(FireEvent)(__in StringVariationsEvent evt);
        };

        class OverloadStringVariationsFactory : 
            public Microsoft::WRL::ActivationFactory<
                IOverloadCasingStatic,
                IPrivateOverloadsStatic>
        {
        public:
            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

            // IOverloadCasingStatic
            HRESULT STDMETHODCALLTYPE PascalStaticOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE PascalStaticOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE UPPERCASESTATICOVERLOAD(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.UPPERCASESTATICOVERLOAD() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE UPPERCASESTATICOVERLOADINT(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.UPPERCASESTATICOVERLOAD(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE IInterfaceStaticOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.IInterfaceStaticOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE IInterfaceStaticOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.IInterfaceStaticOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE camelStaticOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.camelStaticOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE camelStaticOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasingStatic.camelStaticOverload(int) Called"); return S_OK; }

            // IPrivateOverloadsStatic
            HRESULT STDMETHODCALLTYPE _PrivatePascalStaticOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._PrivatePascalStaticOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PrivatePascalStaticOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._PrivatePascalStaticOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PRIVATEUPPERCASESTATICOVERLOAD(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._PRIVATEUPPERCASESTATICOVERLOAD() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PRIVATEUPPERCASESTATICOVERLOADINT(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._PRIVATEUPPERCASESTATICOVERLOAD(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _IInterfacePrivateStaticOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._IInterfacePrivateStaticOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _IInterfacePrivateStaticOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._IInterfacePrivateStaticOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _privateCamelStaticOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._privateCamelStaticOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _privateCamelStaticOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloadsStatic._privateCamelStaticOverload(int) Called"); return S_OK; }
        };

        class OverloadStringVariationsServer :
            public Microsoft::WRL::RuntimeClass<
                DevTests::CamelCasing::IOverloadCasing,
                DevTests::CamelCasing::IPrivateOverloads
            >
        {
            InspectableClass(L"DevTests.CamelCasing.OverloadStringVariations", BaseTrust);

        public:
            OverloadStringVariationsServer()
            {
            }
            ~OverloadStringVariationsServer()
            {
            }

            // IOverloadCasing members
            HRESULT STDMETHODCALLTYPE PascalNotationOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.PascalNotationOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE PascalNotationOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.PascalNotationOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE ALLUPPERCASEOVERLOAD(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.ALLUPPERCASEOVERLOAD() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE ALLUPPERCASEOVERLOADINT(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.ALLUPPERCASEOVERLOAD(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE IInterfaceOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.IInterfaceOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE IInterfaceOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.IInterfaceOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE camelCaseOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.camelCaseOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE camelCaseOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IOverloadCasing.camelCaseOverload(int) Called"); return S_OK; }

            // IPrivateOverloads members
            HRESULT STDMETHODCALLTYPE _PrivatePascalOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._PrivatePascalOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PrivatePascalOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._PrivatePascalOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PRIVATEUPPERCASEOVERLOAD(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._PRIVATEUPPERCASEOVERLOAD() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _PRIVATEUPPERCASEOVERLOADINT(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._PRIVATEUPPERCASEOVERLOAD(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _IInterfacePrivateOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._IInterfacePrivateOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _IInterfacePrivateOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._IInterfacePrivateOverload(int) Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _privateCamelOverload(HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._privateCamelOverload() Called"); return S_OK; }

            HRESULT STDMETHODCALLTYPE _privateCamelOverloadInt(int, HSTRING * result) 
            { *result = hs(L"DevTests.CamelCasing.IPrivateOverloads._privateCamelOverload(int) Called"); return S_OK; }
        
        };

    }
}