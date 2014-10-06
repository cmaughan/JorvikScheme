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
#include "Intrinsics.h"
#include "Scope.h"
#include "Errors.h"
#include "Evaluator.h"
#include "Symbol.h"

namespace Jorvik
{
namespace Scheme
{

// We declare c++ function lambdas and add them to cells in the global scope.
// They are looked up by symbol name.  This macro just hides the crud below.
#define BEGIN_FUNC(sym) pScope->AddVariable(Sym::Symbol(#sym), Cell::Procedure([=](Cell* args) {
#define END_FUNC }))

void Intrinsics::Add(Scope* pScope)
{
    AddInternalOperands(pScope);
    AddMathOperators(pScope);
    AddListOperands(pScope);
    AddPredicates(pScope);
}


void Intrinsics::AddPredicates(Scope*pScope)
{
    BEGIN_FUNC(null?)
        return Cell::Boolean(args->Car()->IsNull());
    END_FUNC;

    BEGIN_FUNC(not)
        if (args->Car()->GetType() & Cell::BoolType)
        {
            return args;
        }
        return Cell::Boolean(false);
    END_FUNC;
}

void Intrinsics::AddMathOperators(Scope* pScope)
{
    BEGIN_FUNC(+)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            pCell = pCell->Add(pCurrent->Car());
            pCurrent = pCurrent->Cdr();
        }
        return pCell;
    END_FUNC;

    BEGIN_FUNC(-)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            pCell = pCell->Subtract(pCurrent->Car());
            pCurrent = pCurrent->Cdr();
        }
        return pCell;
    END_FUNC;

    BEGIN_FUNC(*)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            pCell = pCell->Multiply(pCurrent->Car());
            pCurrent = pCurrent->Cdr();
        }
        return pCell;
    END_FUNC;

    BEGIN_FUNC(/)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            pCell = pCell->Divide(pCurrent->Car());
            pCurrent = pCurrent->Cdr();
        }
        return pCell;
    END_FUNC;

    BEGIN_FUNC(<)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            if (!pCell->Less(pCurrent->Car()))
            {
                return Cell::Boolean(false);
            }
            pCurrent = pCurrent->Cdr();
        }
        return Cell::Boolean(true);
    END_FUNC;

    BEGIN_FUNC(>)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            if (!pCell->Greater(pCurrent->Car()))
            {
                return Cell::Boolean(false);
            }
            pCurrent = pCurrent->Cdr();
        }
        return Cell::Boolean(true);
    END_FUNC;

    BEGIN_FUNC(=)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            if (!pCell->Equal(pCurrent->Car()))
            {
                return Cell::Boolean(false);
            }
            pCurrent = pCurrent->Cdr();
        }
        return Cell::Boolean(true);
    END_FUNC;

    BEGIN_FUNC(<=)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            if (!pCell->Less(pCurrent->Car()) && !pCell->Equal(pCurrent->Car()))
            {
                return Cell::Boolean(false);
            }
            pCurrent = pCurrent->Cdr();
        }
        return Cell::Boolean(true);
    END_FUNC;

    BEGIN_FUNC(>=)
        Cell* pCell = args->Car();
        Cell* pCurrent = args->Cdr();
        while (pCurrent != nullptr && pCurrent->Car())
        {
            if (!pCell->Greater(pCurrent->Car()) && !pCell->Equal(pCurrent->Car()))
            {
                return Cell::Boolean(false);
            }
            pCurrent = pCurrent->Cdr();
        }
        return Cell::Boolean(true);
    END_FUNC;
    
}


void Intrinsics::AddListOperands(Scope* pScope)
{
    BEGIN_FUNC(cons)
        THROW_ERROR_IF(args->Length() != 2, args, "Arguments !=2 to cons");
        return Cell::Pair(args->Car(), args->Cdr()->Car());
    END_FUNC;

    BEGIN_FUNC(car)
        THROW_ERROR_IF(args->Length() != 1, args, "Arguments !=1 to car");
        return args->Car()->Car();
    END_FUNC;

    BEGIN_FUNC(cdr)
        THROW_ERROR_IF(args->Length() != 1, args, "Arguments !=1 to cdr");
        return args->Car()->Cdr();
    END_FUNC;

    BEGIN_FUNC(length)
        return Cell::Integer(args->Car()->Length());
    END_FUNC;
}


void Intrinsics::AddInternalOperands(Scope* pScope)
{
    BEGIN_FUNC(#<void>)
        UNUSED(args);
        return Cell::Void();
    END_FUNC;

    BEGIN_FUNC(display)
        while (args && args->Car())
        {
            std::cout << args->Car();
            args = args->Cdr();
        }
        std::cout << std::endl;
        return Cell::Void();
    END_FUNC;    
        
    BEGIN_FUNC(debug)
        if (args->Length() == 0)
        {
            return Cell::Void();
        }

        while (args && args->Car())
        {
            if (args->Car()->GetType() & Cell::IntegerType)
            {
                if (args->Car()->GetInteger() == 0)
                {
                    Jorvik::Scheme::Evaluator::ClearDebugFlag(Evaluator::Debug);
                }
                else
                {
                    Jorvik::Scheme::Evaluator::SetDebugFlag(Evaluator::Debug);
                }
            }

            args = args->Cdr();
        }
        return Cell::Void();
    END_FUNC;

    BEGIN_FUNC(symbols)
        UNUSED(args);
        Sym::Dump();
        return Cell::Void();
    END_FUNC;

    BEGIN_FUNC(variables)
        UNUSED(args);
        UNUSED(args);
        std::ostringstream str;
        str << *pScope;
        std::cout << str.str();
        return Cell::Void();
    END_FUNC;
}
}
}

