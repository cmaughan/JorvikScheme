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
#include "pch.h"
#include "CellAllocator.h"
#include "Cell.h"
#include "Scope.h"

namespace Jorvik
{
namespace Scheme
{

CellAllocator::CellAllocator()
    : _marked(true),
    _freeList(nullptr),
    _allocList(nullptr),
    _numFreeList(0),
    _numAllocList(0)
{
}

CellAllocator::~CellAllocator()
{
    Cell* pFree = _freeList;
    while (pFree != nullptr)
    {
        Cell* pNext = pFree->_pAllocatorNext;
        delete pFree;
        pFree = pNext;
    }

    Cell* pAlloc = _allocList;
    while (pAlloc != nullptr)
    {
        Cell* pNext = pAlloc->_pAllocatorNext;
        delete pAlloc;
        pAlloc = pNext;
    }

    _freeList = nullptr;
    _allocList = nullptr;
}

CellAllocator& CellAllocator::Instance()
{
    static CellAllocator alloc;
    return alloc;
}

// Use the current mark to mark all cells we can reach from this one.
void CellAllocator::Mark(Cell* cell)
{
    cell->_mark = _marked;

    if (cell->_car)
    {
        Mark(cell->_car);
    }

    if (cell->_cdr)
    {
        Mark(cell->_cdr);
    }
}

// Two strategies currently.
// We can either immediately delete any orphaned cells, or
// we can throw them onto a free pool.
// This means the heap will grow to the maximum required,
// and then allocation will be fast at the expense of 
// memory.  Otherwise, it will keep memory free, but cost a bit more per allocation
void CellAllocator::AddToFreeList(Cell* pCell)
{
#ifdef USE_FREE_LIST
    if (pCell->_type & DeleteNoGC)
    {
        delete pCell;
    }
    else
    {
        pCell->_pAllocatorNext = _freeList;
        _freeList = pCell;
        _numFreeList++;
        pCell->FreeMemory();
    }
#else
    delete pCell;
#endif
    _numAllocList--;
}

void CellAllocator::GarbageCollect(Scope* pScope)
{
    // Mark globals
    Mark(Cell::Void());
    Mark(Cell::EmptyList());

    // Mark all the symbols in the scope.
    const Scope::tmapSymbolToCell& symbols = pScope->GetSymbols();    
    for(auto var : symbols)
    {
        Mark(var.second);
    }

    // Return all unmarked cells to the free list
    Cell* pCell = _allocList;
    while (pCell && pCell->_mark != _marked)
    {
        _allocList = pCell->_pAllocatorNext;
        
        AddToFreeList(pCell);

        pCell = _allocList;
    }

    if (pCell)
    {
        Cell* pPrevious = pCell;
        pCell = pCell->_pAllocatorNext;

        while(pCell)
        {
            Cell* pNext = pCell->_pAllocatorNext;
            if (pCell->_mark != _marked)
            {
                pPrevious->_pAllocatorNext = pNext;
                AddToFreeList(pCell);            
            }
            else
            {
                pPrevious = pCell;
            }
            pCell = pNext;
        }
    }

    // Change the mark so we don't have to reset the marks; we flip the sense of what it means 
    // to be marked each time round.
    _marked = !_marked;
}

Cell& CellAllocator::Alloc()
{
    Cell* pCell;

    // Use the first on the free list.
    if (_freeList != nullptr)
    {
        pCell = _freeList;
        pCell->_cdr = nullptr;
        pCell->_car = nullptr;
        _freeList = _freeList->_pAllocatorNext;
        _numFreeList--;
    }
    else
    {
        pCell = new Cell();
    }

    pCell->_mark = !_marked;
    
    pCell->_pAllocatorNext = _allocList;
    _allocList = pCell;
    _numAllocList++;

    return *pCell;
}

unsigned int CellAllocator::GetFreeListSize() const
{
    return _numFreeList; 
}

unsigned int CellAllocator::GetPoolSize() const
{
    return _numAllocList; 
}

} // Scheme
} // Jorvik