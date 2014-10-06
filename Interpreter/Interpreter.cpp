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
#include "Evaluator.h"
#include "Intrinsics.h"
#include "Interpreter.h"
#include "Errors.h"

namespace Jorvik
{
namespace Scheme
{
  
Interpreter::Interpreter(Evaluator* pScheme)
    : _pScheme(pScheme)
{
    // Add intrinsic functions we support
    Intrinsics::Add(pScheme->GetGlobalScope());
}

// Build a Cell containing the list entries, Interpreting them as we go
Cell* Interpreter::InterpretList(Cell* pList, std::shared_ptr<Scope>& pScope)
{
    Cell* pRet = Cell::EmptyList();
    while(pList && pList->Car())
    {
        pRet = pRet->Append(Interpret(pList->Car(), pScope));
        pList = pList->Cdr();
    }
    return pRet;
}

// The main intepreter.  Loops over the cells evaluating as it goes.
Cell* Interpreter::Interpret(Cell* cell, std::shared_ptr<Scope> pScope)
{   
    // Try to loop to reduce stack depth
    for(;;)
    {
        if (cell->GetType() & Cell::SymbolType)
        {
            // Found a symbol, return it.
            Cell* found = pScope->FindVariable(cell->GetSymbol());
            THROW_ERROR_IF(found == nullptr, cell, "Variable not found: " << (const std::string)*cell->GetSymbol());
            return found;
        }
        else if (cell->IsAtom())
        {
            return cell;
        }
        else if (cell->GetType() & Cell::PairType)
        {
            THROW_ERROR_IF(cell->Length() == 0, cell, "() invalid");
        
            // Check the symbol for a known one.
            if (cell->Car()->GetType() & Cell::SymbolType)
            {
                // Note that Parse will already have done some work for us to reduce expressions to 
                // symbols where appropriate
                const Sym* sym = cell->Car()->GetSymbol();
            
                // Return the quoted expression
                if (sym == Sym::Symbol("_quote"))
                {
                    return cell->Cdr()->Car();
                }
                // Handle the if/then/else branch
                else if (sym == Sym::Symbol("_if"))
                {
                    Cell* testResult = Interpret(cell->Cdr()->Car(), pScope);

                    // Anything that isn't the boolean false is 'true'
                    if ((testResult->GetType() & Cell::BoolType) && 
                        (!testResult->GetBool()))
                    {
                        // alt
                        cell = cell->Cdr()->Cdr()->Cdr()->Car();
                    }
                    else
                    {
                        // conseq
                        cell = cell->Cdr()->Cdr()->Car();
                    }
                    continue;
                }
                // Set a variable.
                else if (sym == Sym::Symbol("_set!"))
                {
                    Cell* pSet = Interpret(cell->Cdr()->Cdr()->Car(), pScope); 
                    Cell* pSymbol = cell->Cdr()->Car();
                    THROW_ERROR_IF(!(pSymbol->GetType() & Cell::SymbolType), pSymbol, "Not a symbol in set: " << pSymbol);
                    THROW_ERROR_IF(!pScope->SetVariable(pSymbol->GetSymbol(), pSet), pSet, "Could not set variable: " << (std::string)*pSymbol->GetSymbol());
                    return Cell::Void();
                }
                // Define a variable
                else if (sym == Sym::Symbol("_define"))
                {
                    Cell* pDefine = Interpret(cell->Cdr()->Cdr()->Car(), pScope); 
                    Cell* pSymbol = cell->Cdr()->Car();
                    THROW_ERROR_IF(!(pSymbol->GetType() & Cell::SymbolType), pSymbol, "Not a symbol in set: " << pSymbol);
                    pScope->AddVariable(pSymbol->GetSymbol(), pDefine);
                    return Cell::Void();
                }
                // Creates a lambda function from args and body
                else if (sym == Sym::Symbol("_lambda"))
                {
                    // We already parsed and created the lambda,
                    // turn it into a function we can call.
                    // lambda pArgs, pBody , scope
                    return Cell::Lambda(cell->Cdr()->Car(), cell->Cdr()->Cdr()->Car(), pScope);
                }
                // Generate a list of expressions to evaluate in the begin
                else if (sym == Sym::Symbol("_begin"))
                {
                    THROW_ERROR_IF(cell->Length() < 2, cell,  "Not enough args in begin: " << cell);
             
                    // Interpret the list, and return the last result.
                    Cell* pRes = InterpretList(cell->Cdr(), pScope);
                    Cell* pLastResult = pRes;
                    while(pLastResult && 
                        (pLastResult->Cdr() != nullptr))
                    {
                        pLastResult = pLastResult->Cdr();
                    }
                    return pLastResult->Car(); 
                }
            }
        
            // Procedure and args, all evaluated.
            Cell* proc = Interpret(cell->Car(), pScope);
            Cell* args = InterpretList(cell->Cdr(), pScope);
    
            // If a lambda, loop around and evaluate the body at the new scope.
            if (proc->GetType() & Cell::LambdaType)
            {
                Cell* params = proc->Car();
                Cell* body = proc->Cdr()->Car();

                // Alloc a scope, because we we are going to make the lambda right now.
                pScope = std::shared_ptr<Scope>(new Scope(params, args, proc->GetScope()));
                cell = body;

                if (Evaluator::TestDebugFlag(Evaluator::Debug))
                {
                    if (pScope.get() != _pScheme->GetGlobalScope())
                    {
                        std::cout << "Lambda Scope: " << std::endl << pScope;
                        std::cout << "Lambda P: " << params << " A: " << args << " B: " << body << std::endl << std::endl;
                    }
                }

                continue;
            }
            // An intrinsic procedure - just call it.
            else if (proc->GetType() & Cell::ProcedureType)
            {
                if (Evaluator::TestDebugFlag(Evaluator::Debug))
                {
                    std::cout << "Procedure Scope: " << std::endl << *pScope;
                    std::cout << proc << " " << args << " " << std::endl << std::endl;
                }

                return proc->GetProcedure()(args);
            }
            else
            {
                // Not sure why we got here...
                THROW_ERROR(proc, "Is not a procedure: " << proc);
            }
        }
        break;
    }
    
    // If not a list, return the atom
    return cell;
}

}
}