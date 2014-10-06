//
// Copyright (c) 2014 Chris Maughan
// All rights reserved.
// http://www.chrismaughan.com, 
// http://www.github.com/cmaughan
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#pragma once

namespace Jorvik
{
namespace Scheme
{

class Cell;
class Scope;

// A simple allocator, and a mark & sweep garbage collector.
// The garbage collector marks all 'in use' cells, and references to other cells.
// It then frees any cells that aren't marked.
// This GC has the limitation that it can only run after all expressions have been evaluated.
// It cannot run during execution of parse/interpret/tokenize, unless extra defence is added to 'dangling'
// cells which wouldn't get marked.
// For my purposes this is acceptable, but YMMV.
class CellAllocator
{
public:
    CellAllocator();
    ~CellAllocator();

    static CellAllocator& Instance();
    void GarbageCollect(Scope* pScope);

    Cell& Alloc();

    unsigned int GetFreeListSize() const;
    unsigned int GetPoolSize() const; 

private:
    void Mark(Cell* pCell);
    void AddToFreeList(Cell* pCell);

private:
    Cell* _allocList;
    Cell* _freeList;

    bool _marked;
    unsigned int _numFreeList;
    unsigned int _numAllocList;
};

} // Scheme
} // Jorvik
