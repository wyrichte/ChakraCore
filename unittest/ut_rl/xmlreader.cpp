/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "rl.h"
#include "HostSysInfo.h"

#import "msxml6.dll" named_guids raw_interfaces_only


#define CHECKHR(x) {hr = x; if (FAILED(hr)) goto CleanUp;}
#define SAFERELEASE(p) {if (p) {(p)->Release(); p = NULL;}}


namespace Xml
{


MSXML2::IXMLDOMDocument *pDoc = NULL;


Node * Node::TopNode;


//-----------------------------------------------------------------------------
//
// Description:
//
//    Constructor for Attribute class.
//
//
//-----------------------------------------------------------------------------

Attribute::Attribute
(
   Char * name,
   Char * value
)
   : Name(name)
   , Value(value)
   , Next(NULL)
{}

Char *
Attribute::GetValue
(
   const Char * name
)
{
   for (Attribute * p = this; p != NULL; p = p->Next)
   {
      if (strcmp(p->Name, name) == 0)
      {
         return p->Value;
      }
   }

   return NULL;
}

void
Attribute::Dump()
{
   for (Attribute * attr = this;
        attr != NULL;
        attr = attr->Next)
   {
      printf("%s=\"%s\"", attr->Name, attr->Value);
      if (attr->Next != NULL)
      {
         printf(" ");
      }
   }
   printf("\n");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Constructor for Node class.
//
//
//-----------------------------------------------------------------------------

Node::Node
(
   Char * name,
   Attribute * attributeList
)
   : Name(name)
   , AttributeList(attributeList)
   , Next(NULL)
   , ChildList(NULL)
   , Data(NULL)
//   , LineNumber(Xml::LineNumber)
{}

Node *
Node::GetChild
(
   const Char * name
)
{
   for (Node * p = this->ChildList; p != NULL; p = p->Next)
   {
      if (strcmp(p->Name, name) == 0)
      {
         return p;
      }
   }

   return NULL;
}

Char *
Node::GetAttributeValue
(
   const Char * name
)
{
   return this->AttributeList->GetValue(name);
}

void
Node::Dump
(
   int indent
)
{
   for (int i = 0; i < indent; i++)
   {
      printf("   ");
   }

   printf("Node %s ", this->Name);
   this->AttributeList->Dump();
   if (this->Data != NULL)
   {
      for (int i = 0; i <= indent; i++)
      {
         printf("   ");
      }
      printf("Data: %s\n", this->Data);
   }
   else
   {
      for (Node * child = this->ChildList;
           child != NULL;
           child = child->Next)
      {
         child->Dump(indent + 1);
      }
   }
}

void
Node::Dump()
{
   this->Dump(0);
}

Char *
ConvertBSTR
(
   BSTR bstr
)
{
   int len = ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, bstr, -1,
      NULL, 0, NULL, NULL);

   Char * newStr = new char[len + 1];

   ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, bstr, -1,
      newStr, len + 1, NULL, NULL);

   return newStr;
}

Node *
ConvertDoc
(
   MSXML2::IXMLDOMNode * pNode
)
{
    MSXML2::IXMLDOMNode * pChild;
    MSXML2::IXMLDOMNode * pNext;
    BSTR nodeName;
    MSXML2::IXMLDOMNamedNodeMap * pattrs;

    pNode->get_nodeName(&nodeName);

    Char * newNodeName = ConvertBSTR(nodeName);

    ::SysFreeString(nodeName);

    Attribute * attrList = NULL;
    Attribute * attrLast = NULL;

    if (SUCCEEDED(pNode->get_attributes(&pattrs)) && pattrs != NULL)
    {
       Attribute * attrItem = NULL;

       pattrs->nextNode(&pChild);

       while (pChild != NULL)
       {
          BSTR name;
          pChild->get_nodeName(&name);

          Char * newName = ConvertBSTR(name);

          ::SysFreeString(name);

          VARIANT value;
          pChild->get_nodeValue(&value);
          ASSERTNR(value.vt == VT_BSTR);

          Char * newValue = ConvertBSTR(V_BSTR(&value));

          VariantClear(&value);
          pChild->Release();

          attrItem = new Attribute(newName, newValue);
          if (attrLast != NULL)
          {
             attrLast->Next = attrItem;
          }
          else
          {
             attrList = attrItem;
          }
          attrLast = attrItem;

          pattrs->nextNode(&pChild);
       }
       pattrs->Release();
    }

    Node * childList = NULL;
    Node * childLast = NULL;

    pNode->get_firstChild(&pChild);
    while (pChild)
    {
       Node * childItem = ConvertDoc(pChild);

       if (childLast != NULL)
       {
          childLast->Next = childItem;
       }
       else
       {
          childList = childItem;
       }
       childLast = childItem;

       pChild->get_nextSibling(&pNext);
       pChild->Release();
       pChild = pNext;
    }

   Node * newNode = new Node(newNodeName, attrList);
   newNode->ChildList = childList;

   if (childList == NULL)
   {
      BSTR text;
      pNode->get_text(&text);

      newNode->Data = ConvertBSTR(text);

      ::SysFreeString(text);
   }
   else if (childList == childLast)
   {
      // This is a bit ugly but will do.  If we have a single child with data
      // called "#text", then pull the data up to this node.

      if ((childList->Data != NULL)
       && (_stricmp(childList->Name, "#text") == 0))
      {
         newNode->Data = childList->Data;
         newNode->ChildList = NULL;
      }
   }

   return newNode;
}

bool
Init()
{
   HRESULT hr;

   CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);
   hr = CoCreateInstance(HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? 
#if defined (_M_AMD64) || defined(_M_ARM64)
       MSXML2::CLSID_DOMDocument
#else
       MSXML2::CLSID_DOMDocument60
#endif       
       : MSXML2::CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
       MSXML2::IID_IXMLDOMDocument, (void**)&pDoc);

   return hr == 0 ? true : false;
}

Node *
ReadFile
(
   const char * fileName
)
{
   MSXML2::IXMLDOMParseError * pXMLError = NULL;
   VARIANT         vURL;
   VARIANT_BOOL    vb;
   HRESULT         hr;
   BSTR pBURL = NULL;
   WCHAR wszURL[MAX_PATH];
   Node * topNode = NULL;

   ::MultiByteToWideChar(CP_ACP, 0, fileName, -1, wszURL, MAX_PATH);
   pBURL = SysAllocString(wszURL);

   hr = pDoc->put_async(VARIANT_FALSE);
   if (FAILED(hr))
   {
      return NULL;
   }

   // Load xml document from the given URL or file path
   VariantInit(&vURL);
   vURL.vt = VT_BSTR;
   V_BSTR(&vURL) = pBURL;
   pDoc->load(vURL, &vb);

   LONG errorCode = E_FAIL;

   pDoc->get_parseError(&pXMLError);
   pXMLError->get_errorCode(&errorCode);

   if (errorCode != 0)
   {
      long line, linePos;
      LONG errorCode;
      BSTR pBReason;

      pXMLError->get_line(&line);
      pXMLError->get_linepos(&linePos);
      pXMLError->get_errorCode(&errorCode);
      pXMLError->get_reason(&pBReason);

      if (line > 0)
      {
         fprintf(stderr, "Error on line %d, position %d in \"%S\".\n",
            line, linePos, pBURL);
         Fatal("%S", pBReason);
      }
      else
      {
         Fatal("%S: file could not be read", pBURL);
      }
   }
   else
   {
      // Convert the MSXML2 format to the RL XML format to minimize the impact
      // of this changeover.

      MSXML2::IXMLDOMNode* pNode = NULL;
      hr = pDoc->QueryInterface(MSXML2::IID_IXMLDOMNode,(void**)&pNode);
      if (FAILED(hr))
      {
         return NULL;
      }

      topNode = ConvertDoc(pNode);
      pNode->Release();
   }

   pXMLError->Release();

   return topNode;
}


}  // namespace Xml
