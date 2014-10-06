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

#include "Symbol.h"
#include <functional>
#include <memory>
#include <vector>

namespace Jorvik
{
namespace Scheme
{

class Scope;
class CellAllocator;

typedef long long tCellInteger;
typedef float tCellFloat;

class Cell
{
public:    
    enum TypeFlags 
    {
        PairType = (1 << 0),
        SymbolType = (1 << 1),
        StringType = (1 << 2),
        IntegerType = (1 << 3),
        FloatType = (1 << 4),
        ProcedureType = (1 << 5),
        LambdaType = (1 << 6),
        BoolType = (1 << 7),
        AtomType = (1 << 8),
        DeleteNoGC = (1 << 9)
    };

    typedef std::function<Cell*(Cell* list)> tProc;
    
    // Static create for the cell pool
    static void StaticInit();
    static void StaticDestroy();

    // Constructor
    Cell();
    ~Cell();

    // Call to create a cell
    static Cell* EmptyList(); 
    static Cell* Pair(Cell* car = nullptr, Cell* cdr = nullptr);
    static Cell* Integer(tCellInteger value);
    static Cell* Float(tCellFloat value);
    static Cell* Symbol(const Sym* symbol);
    static Cell* String(const char* string);
    static Cell* Procedure(tProc procedure, const char* pszTypeName = nullptr);
    static Cell* Boolean(bool val);
    static Cell* Lambda(Cell* pArgs, Cell* pBody, std::shared_ptr<Scope>& pScope);
        
    Cell* Add(Cell* rhs) const;
    Cell* Multiply(Cell* rhs) const;
    Cell* Subtract(Cell* rhs) const;
    Cell* Divide(Cell* rhs) const;

    void AppendInternal(Cell* cell);
    Cell* Append(Cell* cell);
    Cell* Cons(Cell* cell);

    void FreeMemory();

    Cell* Cdr() const;
    Cell* Car() const;

    bool IsPair() const;
    bool IsNull() const;
    bool IsAtom() const;
    bool IsLambda() const;
    bool IsSymbol() const;

    // Length of list
    unsigned int Length() const; 

    // Const operators
    bool Less(Cell* rhs) const;
    bool Greater(Cell* rhs) const;
    bool Equal(Cell* rhs) const;
    
    // Convert this cell and its contained cells to an expression
    std::string ToString() const;
    std::string TypeToString() const;
    void ToString(std::ostringstream& str) const;
    void ToAtomString(std::ostringstream& str) const;
    void ToListEntryString(std::ostringstream& str) const;
    
    // Accessors
    unsigned int GetType() const;
    bool GetBool() const;
    const Sym* GetSymbol() const;
    const std::string& GetString() const;
    tCellInteger GetInteger() const;
    tCellFloat GetFloat() const;
    tProc GetProcedure() const;
    Scope* GetScope() const;
    
    static Cell* Void();

protected:

    friend std::ostream& operator << (std::ostream& stream, Cell* cell);
    
protected:

    // Variant type
    unsigned short _type;
    bool _mark;
    Cell* _cdr;
    Cell* _car;

    // Possible values
    union
    {
        bool _bool;
        tCellInteger _integer;
        tCellFloat _float;
        tProc* _pProcedure;    
        std::string* _pString;
        const Sym* _pSymbol;
        std::shared_ptr<Scope>* _ppScope;
    };
            
    // Allocator and garbage collector
    friend CellAllocator;
    Cell* _pAllocatorNext; 

};



}
}