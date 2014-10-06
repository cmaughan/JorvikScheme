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
#include "Scope.h"
#include "Errors.h"

namespace Jorvik
{

namespace Scheme
{

Scope::Scope()
    : _pOuter(nullptr)
{

}

Scope::~Scope()
{

}

Scope::Scope(Cell* params, Cell* args, Scope* pOuter)
    : _pOuter(pOuter)
{
    Cell* pCurrentArg = args;
  
    if (params->IsPair())
    {
        THROW_ERROR_IF(args->IsPair() &&
            args->Length() > params->Length(), params, "Expected num arguments to match parameters: (" << args << " , " << params << ")");
        
        Cell* pCurrentParam = params;
        while (pCurrentParam && pCurrentParam->Car())
        {
            if (!pCurrentArg || pCurrentArg->IsNull())
            {
                AddVariable(pCurrentParam->Car()->GetSymbol(), Cell::EmptyList());
            }
            else
            {
                AddVariable(pCurrentParam->Car()->GetSymbol(), pCurrentArg->Car());
            }
            pCurrentParam = pCurrentParam->Cdr();
            if (pCurrentArg)
            {
                pCurrentArg = pCurrentArg->Cdr();
            }
        }
    }
    else
    {
        THROW_ERROR_IF(!(params->GetType() & Cell::SymbolType), params, "Expected parameter to be a symbol");
       
        Cell* pNewList = Cell::EmptyList();
        while (pCurrentArg && pCurrentArg->Car())
        {
            pNewList = pNewList->Append(pCurrentArg->Car());
            pCurrentArg = pCurrentArg->Cdr();
        }
        AddVariable(params->GetSymbol(), pNewList);
    }
}

void Scope::AddVariable(const Sym* sym, Cell* cell)
{
    _variables[sym] = cell;
}

Cell* Scope::FindVariable(const Sym* sym)
{
    auto itr = _variables.find(sym);
    if (itr != std::end(_variables))
    {
        return itr->second;
    }
    if (_pOuter != nullptr)
    {
        return _pOuter->FindVariable(sym);
    }
    return nullptr;
}

bool Scope::SetVariable(const Sym* sym, Cell* cell)
{
    auto itr = _variables.find(sym);
    if (itr == std::end(_variables))
    {
        if (_pOuter != nullptr)
        {
            return _pOuter->SetVariable(sym, cell);
        }
        return false;
    }
    itr->second = cell;
    return true;
}

// Dump the scope to an ostream.
std::ostream& operator << (std::ostream& stream, const Scope& scope)
{
    stream << "Scope " << &scope << ", Variables: " << scope._variables.size() << std::endl;
    for(auto var : scope._variables)
    {
        stream << (std::string)*var.first;
        if (!var.second->ToString().empty())
        {
            stream << " : ";
            stream << var.second->ToString();
        }
        else
        {
            if (var.second->GetType() & Cell::ProcedureType)
            {
                stream << " : <intrinsic>";
            }
        }
        stream << std::endl;
    }
    if (scope._pOuter)
    {
        stream << std::endl << "Parent Scope: " << std::endl << *scope._pOuter;
    }
    stream << std::endl;
    return stream;
}

} // Scheme
} // Jorvik