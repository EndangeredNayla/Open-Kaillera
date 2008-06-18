/******************************************************************************
          .d8888b.   .d8888b.  
         d88P  Y88b d88P  Y88b 
         888    888        888 
88888b.  888    888      .d88P 
888 "88b 888    888  .od888P"  
888  888 888    888 d88P"      
888  888 Y88b  d88P 888"       
888  888  "Y8888P"  888888888              Open Kaillera Arcade Netplay Library
*******************************************************************************
Copyright (c) Open Kaillera Team 2003-2008

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice must be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#pragma once

#include "StaticArray.h"

namespace n02 {

    // unverified

    template <class _BaseType, int _Count>
    class StaticAllocator
    {

    protected:


        _BaseType items[_Count];
        StaticArray<_BaseType*, _Count> unallocated;


    public:

        StaticAllocator()
        {
            resetAllocation();
        }

        inline _BaseType * allocate()
        {
            _BaseType * _riele = unallocated[0];
            unallocated.removei(0);
            return _riele;
        }

        inline void free(_BaseType * element)
        {
            unallocated.add(element);
        }

        inline int allocatedCount()
        {
            return _Count - unallocated.itemsCount();
        }

        inline int freeCount()
        {
            return unallocated.itemsCount();
        }

        void resetAllocation()
        {
            unallocated.clearItems();
            for(int x =0; x < _Count; x++){
                unallocated.addItem(&items[x]);
            }
        }
    };
};