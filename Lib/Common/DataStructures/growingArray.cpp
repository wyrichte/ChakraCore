#include <stdafx.h>

namespace JsUtil
{
    GrowingUint32HeapArray* GrowingUint32HeapArray::Create(int _length)
    {
        return HeapNew(GrowingUint32HeapArray, &HeapAllocator::Instance, _length);
    }

}
