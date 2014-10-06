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
#include "Cell.h"
#include "Errors.h"
#include "CellAllocator.h"
#include "Scope.h"

namespace Jorvik
{
namespace Scheme
{

// Empty and Void
static Cell* g_pVoid = nullptr;
static Cell* g_pEmptyList = nullptr;

Cell* Cell::EmptyList()
{
    return g_pEmptyList;
}

Cell* Cell::Void()
{
    return g_pVoid;
}

// Init for all cells
void Cell::StaticInit()
{
    g_pVoid = Cell::Symbol(Sym::Symbol("#<void>"));
    g_pEmptyList = Cell::Pair();
}

void Cell::StaticDestroy()
{
    delete g_pVoid;
    delete g_pEmptyList;
}

// Constructor
Cell::Cell()
    : _ppScope(nullptr),
    _car(nullptr),
    _cdr(nullptr),
    _pAllocatorNext(nullptr)
{
}

Cell::~Cell()
{
    FreeMemory();
}

// A simple pair
Cell* Cell::Pair(Cell* car, Cell* cdr)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = PairType;
    cell._car = const_cast<Cell*>(car);
    cell._cdr = const_cast<Cell*>(cdr);
    return &cell;
}

// A lambda function with scope
Cell* Cell::Lambda(Cell* pArgs, Cell* pBody, std::shared_ptr<Scope>& pScope)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = LambdaType;
    cell._car = const_cast<Cell*>(pArgs);
    cell._cdr = const_cast<Cell*>(Cell::Pair(pBody));
    cell._ppScope = new std::shared_ptr<Scope>(pScope);
    return &cell;
}

void Cell::AppendInternal(Cell* add) 
{
    if (Cdr() != nullptr)
    {
        Cdr()->AppendInternal(add);
    }
    else
    {
        _cdr = const_cast<Cell*>(Cell::Pair(add));
    }
    return;
}

// This append is allowed to mutate the current list.
// It's currently the only point of mutation.
Cell* Cell::Append(Cell* add)
{
    // An empty list is replaced by a new list with the item as the first item
    if (IsNull())
    {
        return Pair(add);
    }
    AppendInternal(add);
    return this;
}

Cell* Cell::String(const char* pszValue)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = (StringType | AtomType);
    if (pszValue == nullptr)
    {
        cell._pString = new std::string();
    }
    else
    {
        cell._pString = new std::string(pszValue);
    }
    return &cell;
}

Cell* Cell::Symbol(const Sym* value)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = SymbolType | AtomType;
    cell._pSymbol = value;
    return &cell;
}

Cell* Cell::Procedure(tProc procedure, const char* pszTypeName)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = ProcedureType;
    cell._pProcedure = new tProc(procedure);
    if (pszTypeName)
    {
        cell._car = const_cast<Cell*>(Cell::Symbol(Sym::Symbol(pszTypeName)));
    }
    return &cell;
}

Cell* Cell::Boolean(bool val)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = (BoolType | AtomType);
    cell._bool = val;
    return &cell;
}

Cell* Cell::Integer(tCellInteger val)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = (IntegerType | AtomType);
    cell._integer = val;
    return &cell;
}

Cell* Cell::Float(tCellFloat val)
{
    Cell& cell = CellAllocator::Instance().Alloc();
    cell._type = (FloatType | AtomType);
    cell._float = val;
    return &cell;
}

void Cell::FreeMemory()
{
    if (_type == Cell::LambdaType)
    {
        if (_ppScope)
        {
            delete _ppScope;
            _ppScope = nullptr;
        }
    }
    else if (_type == Cell::StringType)
    {
        if (_pString)
        {
            delete _pString;
            _pString = nullptr;
        }
    }
    else if (_type == Cell::ProcedureType)
    {
        if (_pProcedure)
        {
            delete _pProcedure;
            _pProcedure = nullptr;
        }
    }
}

// Cons adds an expression onto the beginning of existing list
// ... It creates a pair where car is itself and cdr is the next item.
// this->Cons(a) == (cons this a)
Cell* Cell::Cons(Cell* cell)
{
    return Cell::Pair(this, cell);
}


Cell* Cell::Car() const
{
    THROW_ERROR_IF(!IsPair() && !IsLambda(), this, "Not a pair in Car()");
    return _car;
}

Cell* Cell::Cdr() const
{
    THROW_ERROR_IF(!IsPair() && !IsLambda(), this, "Not a pair in Cdr()");
    return _cdr;
}

// null? is defined only for lists.
// null? is true iff an empty list
bool Cell::IsNull() const
{
    if (IsPair() &&
        _car == nullptr &&
        _cdr == nullptr)
    {
        return true;
    }

    return false;
}

// pair? if we point to something else
bool Cell::IsPair() const
{
    return (_type & PairType);
}

bool Cell::IsLambda() const
{
    return (_type & LambdaType) ? true : false;
}

bool Cell::IsSymbol() const
{
    return (_type & SymbolType) ? true : false;
}

// atom? is define as not pair? and not null?
bool Cell::IsAtom() const
{
    return (_type & AtomType) ? true : false;
}

// TODO - detect circular lists
unsigned int Cell::Length() const
{    
    unsigned int length = 0;
    const Cell* pCurrent = this;
    while (pCurrent != nullptr && pCurrent->Car())
    {
        length++;
        pCurrent = pCurrent->_cdr;
    }
    return length;
}

Cell* Cell::Add(Cell* rhs) const
{
    Cell& ret = CellAllocator::Instance().Alloc();

    // Automatic promote to float.
    if (_type & IntegerType && rhs->_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = static_cast<tCellFloat>(_integer) + rhs->_float;
    }
    else if (_type & IntegerType && rhs->_type & IntegerType)
    {
        ret._type = IntegerType;
        ret._integer = _integer + rhs->_integer;
    }
    else if (_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = _float + (rhs->_type & FloatType ? rhs->_float : rhs->_integer);
    }
    else
    {
        throw std::runtime_error("Type not supported in operator +");
    }
    return &ret;
}

Cell* Cell::Subtract(Cell* rhs) const
{
    Cell& ret = CellAllocator::Instance().Alloc();

    // Automatic promote to float.
    if (_type & IntegerType && rhs->_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = static_cast<tCellFloat>(_integer) - rhs->_float;
    }
    else if (_type & IntegerType && rhs->_type & IntegerType)
    {
        ret._type = IntegerType;
        ret._integer = _integer - rhs->_integer;
    }
    else if (_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = _float - (rhs->_type & FloatType ? rhs->_float : rhs->_integer);
    }
    else
    {
        throw std::runtime_error("Type not supported in operator -");
    }
    return &ret;
}

Cell* Cell::Multiply(Cell* rhs) const
{
    Cell& ret = CellAllocator::Instance().Alloc();

    // Automatic promote to float.
    if (_type & IntegerType && rhs->_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = static_cast<tCellFloat>(_integer) * rhs->_float;
    }
    else if (_type & IntegerType && rhs->_type & IntegerType)
    {
        ret._type = IntegerType;
        ret._integer = _integer * rhs->_integer;
    }
    else if (_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = _float * (rhs->_type & FloatType ? rhs->_float : rhs->_integer);
    }
    else
    {
        throw std::runtime_error("Type not supported in operator *");
    }
    return &ret;
}

Cell* Cell::Divide(Cell* rhs) const
{
    Cell& ret = CellAllocator::Instance().Alloc();

    // Automatic promote to float.
    if (_type & IntegerType && rhs->_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = static_cast<tCellFloat>(_integer) / rhs->_float;
    }
    else if (_type & IntegerType && rhs->_type & IntegerType)
    {
        ret._type = FloatType;
        ret._float = _integer / (tCellFloat)rhs->_integer;
    }
    else if (_type & FloatType)
    {
        ret._type = FloatType;
        ret._float = _float / (rhs->_type & FloatType ? rhs->_float : rhs->_integer);
    }
    else
    {
        throw std::runtime_error("Type not supported in operator /");
    }
    return &ret;
}

bool Cell::Less(Cell* rhs) const
{
    if (_type & IntegerType)
    {
        if (rhs->_type & IntegerType)
        {
            return GetInteger() < rhs->GetInteger();
        }
        else if (rhs->_type & FloatType)
        {
            return GetInteger() < rhs->GetFloat();
        }
    }
    else if (_type & FloatType)
    {
        if (rhs->_type & IntegerType)
        {
            return GetFloat() < rhs->GetInteger();
        }
        else if (rhs->_type & FloatType)
        {
            return GetFloat() < rhs->GetFloat();
        }
    }
    return false;
}

bool Cell::Greater(Cell* rhs) const
{
    if (_type & IntegerType)
    {
        if (rhs->_type & IntegerType)
        {
            return GetInteger() > rhs->GetInteger();
        }
        else if (rhs->_type & FloatType)
        {
            return GetInteger() > rhs->GetFloat();
        }
    }
    else if (_type & FloatType)
    {
        if (rhs->_type & IntegerType)
        {
            return GetFloat() > rhs->GetInteger();
        }
        else if (rhs->_type & FloatType)
        {
            return GetFloat() > rhs->GetFloat();
        }
    }
    return false;
}

bool Cell::Equal(Cell* rhs) const
{
    if (_type & IntegerType)
    {
        if (rhs->_type & IntegerType)
        {
            return GetInteger() == rhs->GetInteger();
        }
        else if (rhs->_type & FloatType)
        {
            return GetInteger() == rhs->GetFloat();
        }
    }
    else if (_type & FloatType)
    {
        if (rhs->_type & IntegerType)
        {
            return GetFloat() == rhs->GetInteger();
        }
        else if (rhs->_type & FloatType)
        {
            return GetFloat() == rhs->GetFloat();
        }
    }
    return false;
}

void Cell::ToAtomString(std::ostringstream& str) const
{
    if (_type & BoolType)
    {
        str << (_bool ? "#t" : "#f");
    }
    else if (_type & FloatType)
    {
        str << std::to_string(_float);
    }
    else if (_type & IntegerType)
    {
        str << std::to_string(_integer);
    }
    else if (_type & SymbolType)
    {
        if (this != g_pVoid)
        {
            str << (std::string)*_pSymbol;
        }
    }
    else if (_type & StringType)
    {
        // Escape the returned string
        str << "\"" << *_pString << "\"";
    }
    if (_type & LambdaType)
    {
        str << "<lambda>";
    }
    else if (_type & ProcedureType)
    {
        if (_car != nullptr)
        {
            str << _car->ToString();
        }
        else
        {
            str << "<procedure>";
        }
    }
}

void Cell::ToListEntryString(std::ostringstream& str) const
{
    if (IsPair())
    {
        if (_car)
        {
            std::string car = _car->ToString();
            if (!car.empty())
            {
                str << " " << car;
            }
            //_car->ToString(str);
        }
        
        if (_cdr)
        {
            _cdr->ToListEntryString(str);
        }
    }
    else
    {
        str << " . ";
        ToAtomString(str);
    }
}

std::string Cell::ToString() const
{
    std::ostringstream str;
    ToString(str);
    return str.str();
}

void Cell::ToString(std::ostringstream& str) const
{
    if (!IsPair() && !IsLambda())
    {
        ToAtomString(str);
    }
    else
    {
        str << "(";
        if (_car)
        {
            _car->ToString(str);
        }

        if (_cdr)
        {
            _cdr->ToListEntryString(str);
        }
        str << ")";
    }
}

std::string Cell::TypeToString() const
{
    switch(_type)
    {
    case SymbolType:
        return "symbol";
    case StringType:
        return "string";
    case IntegerType:
        return "integer";
    case FloatType:
        return "float";
    case PairType:
        return "list";
    case ProcedureType:
        return "procedure";
    case LambdaType:
        return "lambda";
    case BoolType:
        return "bool";
    default:
        return "<unknown>";
    }
}

unsigned int Cell::GetType() const
{
    return _type; 
}
 
#define CHECK_TYPE(a) if (!(a & _type)) throw std::runtime_error("Unexpected type: " #a);
bool Cell::GetBool() const 
{
    CHECK_TYPE(BoolType);
    return _bool;
}

const Sym* Cell::GetSymbol() const 
{
    CHECK_TYPE(SymbolType);
    return _pSymbol;
}

const std::string& Cell::GetString() const 
{
    CHECK_TYPE(StringType);
    return *_pString;
}

tCellInteger Cell::GetInteger() const 
{
    CHECK_TYPE(IntegerType);
    return _integer;
}

tCellFloat Cell::GetFloat() const
{
    CHECK_TYPE(FloatType);
    return _float;
}


Cell::tProc Cell::GetProcedure() const
{
    return *_pProcedure;
}

Scope* Cell::GetScope() const
{
    if (!_ppScope)
    {
        return nullptr;
    }
    return (*_ppScope).get();
}

std::ostream& operator << (std::ostream& stream, Cell* cell)
{
    stream << cell->ToString();
    return stream;
}

}
}