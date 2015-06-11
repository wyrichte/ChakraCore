//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "StdAfx.h"

using namespace WEX::Common;

namespace JsrtUnitTests
{
  String GetCurrentModulePath()
  {
      HMODULE hmod;

      if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          reinterpret_cast<LPCWSTR>(GetCurrentModulePath), &hmod))
      {
          VERIFY_FAIL(L"GetCurrentModulePath: Failed loading test module handle");
      }
      
      WCHAR fullpath[_MAX_PATH];

      if (GetModuleFileNameW(hmod, fullpath, _MAX_PATH) == _MAX_PATH)
      {
          VERIFY_FAIL(L"GetCurrentModulePath: Failed getting module path");
      }

      WCHAR drive[_MAX_DRIVE];
      WCHAR dir[_MAX_DIR];
      WCHAR fname[_MAX_FNAME];
      WCHAR ext[_MAX_EXT];

      if (_wsplitpath_s<_MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT>(fullpath, drive, dir, fname, ext) ||
          _wmakepath_s<_MAX_PATH>(fullpath, drive, dir, NULL, NULL))
      {
          VERIFY_FAIL(L"GetCurrentModulePath: Failed computing module path");
      }      

      String path(fullpath);      
      return path;
  }

  LPCWSTR LoadScriptFileWithPath(LPCWSTR filename)
  {
      FILE * file;
      LPCWSTR contents = NULL;      

      // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
      // etc.
      if(_wfopen_s(&file, filename, L"rb"))
      {         
          VERIFY_FAIL(String(L"Cannot open source file: ") + filename);                                
          return NULL;
      }

      // Determine the file length, in bytes.
      fseek(file, 0, SEEK_END);
      UINT lengthBytes = ftell(file);
      fseek(file, 0, SEEK_SET);
      char *rawBytes = (char *)calloc(lengthBytes + 2, sizeof(char));
      if (NULL == rawBytes)
      {
          VERIFY_FAIL(L"Out of memory");            
          return NULL;
      }

      // Read the entire content as a binary block.
      fread((void *)rawBytes, sizeof(char), lengthBytes, file);
      fclose(file);

      // TODO: This is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, 
      // SCSU, BOCU could be wrongly classified as ANSI

      // Read encoding, handling any conversion to Unicode.
      wchar_t *rawCharacters = (wchar_t *)rawBytes;
      bool isUtf8 = false;
      if((0xEF == *rawBytes && 0xBB == *(rawBytes + 1) && 0xBF == *(rawBytes + 2)))
      {
          isUtf8 = true;
      }
      else if (0xFFFE == *rawCharacters || 0x0000 == *rawCharacters && 0xFEFF == *(rawCharacters + 1))
      {
          // unicode unsupported
          delete rawBytes;
          VERIFY_FAIL(L"Unsupported file encoding");
          return NULL;
      }
      else if (0xFEFF == *rawCharacters)
      {
          // unicode LE
          contents = rawCharacters;
      }
      else
      {
          // Assume UTF8
          isUtf8 = true;
      }

      if (isUtf8)
      {
          UINT ansiCharacters = lengthBytes + 1;
          contents = (wchar_t *)calloc(ansiCharacters, sizeof(wchar_t));
          if (NULL == contents)
          {
              delete rawBytes;
              VERIFY_FAIL(L"Out of memory");
              return NULL;
          }

          // Covert to Unicode.
          if (0 == MultiByteToWideChar(CP_UTF8, 0, rawBytes, ansiCharacters,
              (LPWSTR)contents, ansiCharacters))
          {
              delete rawBytes;
              delete contents;
              VERIFY_FAIL(L"Couldn't convert file to Unicode");
              return NULL;
          }        
      }

      return contents;
  }

  LPCWSTR LoadScriptFile(LPCWSTR filename)
  {      
      return LoadScriptFileWithPath(GetCurrentModulePath() + filename);
  }
}