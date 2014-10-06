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
#include "Evaluator.h"
#include "Parser.h"
#include "Cell.h"
#include "Errors.h"

namespace Jorvik
{
namespace Scheme
{

// Append one list of cells onto another, with optional applied procedure
// A helper until I implement append/list for cells#
Cell* Parser::AppendCells(Cell* pSource, Cell* pTarget, tParseProc proc)
{
    while(pSource && pSource->Car())
    {
        /*
        if (proc != nullptr)
        {
            pSource = pSoruce-.proc(pSource->Car());
        }*/
        pTarget = pTarget->Append(pSource->Car());
       
        pSource = pSource->Cdr();
    }
    return pTarget;
}



// (if p c) => (if p c None) : (if pred conseq alt)
// Check(size == 4)
Cell* Parser::Parse_If(Cell* cell)
{
    if (cell->Length() == 3)
    {
        cell = cell->Append(Cell::Void());
    }

    // (if a b c)
    THROW_ERROR_IF(cell->Length() != 4, cell, "'if' does not take " << cell->Length() << " arguments" );
    return cell;
}

// Check (size == 2)
Cell* Parser::Parse_Quote(Cell* cell)
{
    THROW_ERROR_IF(cell->Length() != 2, cell, "'quote' does not take " << cell->Length() << " arguments");
    return cell;
}

// Check(args == 3)
// Check(setting a symbol)
// Make a list, Parseing the set argument
Cell* Parser::Parse_Set(Cell* pSet)
{
    // (set! Symbol n)
    THROW_ERROR_IF(pSet->Length() != 3, pSet, "'set' does not take " << pSet->Length() << " arguments" );
    THROW_ERROR_IF(!(pSet->Cdr()->Car()->GetType() & Cell::SymbolType), pSet, "Can only set! a symbol");
                
    // Parse the third entry
    Cell* pRet = Cell::Pair(pSet->Car());
    pRet = pRet->Append(pSet->Cdr()->Car());

    return pRet->Append(Parse_Cell(pSet->Cdr()->Cdr()->Car()));
}

Cell* Parser::Parse_Define(Cell* cell, bool topLevel)
{
    UNUSED(topLevel);
    THROW_ERROR_IF(cell->Length() < 3, cell, "'define' does not take " << cell->Length() << " arguments");

    // Example: (define (add a b) (+ a b))
    // (define add (lambda (a b) (+ a b)))
    Cell* pParams = cell->Cdr()->Car();
    Cell* pBody = cell->Cdr()->Cdr();
               
    // (define (f args..) body)
    // Parse the arguments, if necessary, and call back into the Parse
    if (pParams->GetType() & Cell::PairType)
    {
        // f
        Cell* f = pParams->Car();

        // (Args)
        Cell* args = pParams->Cdr();

        // (define)
        Cell* pDefine = Cell::Pair(Cell::Symbol(Sym::Symbol("_define")), nullptr);
        
        // (lambda)
        Cell* pLambda = Cell::Pair(Cell::Symbol(Sym::Symbol("_lambda")), nullptr);
        
        // (lambda (args)
        pLambda = pLambda->Append(args);

        // (lambda (args) (body)
        // Body is a list of expressions. ((+ a a) ...)
        while (pBody && pBody->Car())
        {
            pLambda = pLambda->Append(pBody->Car());
            pBody = pBody->Cdr();
        }
        
        // (define f)
        pDefine = pDefine->Append(f);

        // (define f (lamda ...))
        pDefine = pDefine->Append(pLambda);

        return Parse_Cell(pDefine);
    }
    // A parsed define, ready to store
    else
    {
        THROW_ERROR_IF(cell->Length()!= 3, cell, "'define' does not take " << cell->Length() << " arguments");
        THROW_ERROR_IF(!(pParams->GetType() & Cell::SymbolType), cell, "Expected Symbol in define");
        
        pBody = Parse_Cell(pBody->Car());

        // (define)
        Cell* pDefine = Cell::Pair(Cell::Symbol(Sym::Symbol("_define")), nullptr);

        // (define (params)
        pDefine = pDefine->Append(pParams);

        // (define (params) (pBody))
        pDefine = pDefine->Append(pBody);

        return pDefine;
    }
}

// Evaluate all items in a begin
Cell* Parser::Parse_Begin(Cell* cell, bool topLevel)
{
    UNUSED(topLevel);
    if (cell->Length() == 1)
    {
        return Cell::Void();
    }
    Cell* pBegin = Cell::Pair(Cell::Symbol(Sym::Symbol("_begin")));

    Cell* pCurrent = cell->Cdr();
    while (pCurrent && pCurrent->Car())
    {
        pBegin = pBegin->Append(Parse_Cell(pCurrent->Car()));
        pCurrent = pCurrent->Cdr();
    }
    return pBegin;
}

// Quasiquote.
// Was initially part of the parser, now will be done as a macro
Cell* Parser::Parse_Quasiquote(Cell* pQuasi)
{
    Cell* pQuote = Cell::Pair(Cell::Symbol(Sym::Symbol("_quote")), pQuasi);
    return pQuote;
}

// (lambda (x) e1 e2) => (lamba (a) (begin e1 e2))
Cell* Parser::Parse_Lambda(Cell* cell)
{
    THROW_ERROR_IF(cell->Length() < 3, cell, "'lambda' does not take " << cell->Length() << " arguments");
    
    Cell* pArgs = cell->Cdr()->Car();
    if (pArgs->IsPair())
    {
        Cell* pCurrent = pArgs;
        while(pCurrent && pCurrent->Car())
        {
            THROW_ERROR_IF(!(pCurrent->Car()->GetType() & Cell::SymbolType), cell,  "'lambda' arg is not a symbol: " << pCurrent);
            pCurrent = pCurrent->Cdr();
        }
    }
    else
    {
        THROW_ERROR_IF(!(pArgs->GetType() & Cell::SymbolType), cell, "'lambda' arg is not a symbol or list: " << pArgs);
    }

    // (_lambda (args) ... (body) / (begin (body) (body))
    Cell* pLambda(Cell::Pair(Cell::Symbol(Sym::Symbol("_lambda")), nullptr));
    pLambda->Append(pArgs);
 
    Cell* pBody = cell->Cdr()->Cdr();
    if (pBody->Length() == 1)
    {
        // Parse the single body argument, no need for a begin
        pLambda->Append(Parse_Cell(pBody->Car()));
    }
    else
    {
        // Append all the body arguments as a begin
        Cell* pBegin = Cell::Pair(Cell::Symbol(Sym::Symbol("_begin")));
        pBegin = AppendCells(pBody, pBegin);
        pBegin = Parse_Cell(pBegin);

        pLambda->Append(pBegin); 
    }

    return pLambda;
}

// Parse all cells in a list. i.e. look at the CAR in the list of CDRs
Cell* Parser::Parse_Cells(Cell* cell, bool topLevel)
{
    if (cell == nullptr || cell->Car() == nullptr)
    {
        return cell;
    }
    return Cell::Pair(cell->Car(), Parse_Cell(cell->Cdr(), topLevel));
}

// Parse a cell
Cell* Parser::Parse_Cell(Cell* cell, bool topLevel)
{   
    // Parse the cells and check for errors along the way.
    if (cell->IsPair())
    {
        // Parse the pair
        if (cell->IsNull())
        {
            // Empty list
            return cell;
        }
        else if (cell->Car()->GetType() & Cell::SymbolType)
        {
            const Sym* symbol = cell->Car()->GetSymbol();
            if (symbol == Sym::Symbol("_quote"))
            {
                return Parse_Quote(cell);
            }
            else if (symbol == Sym::Symbol("_if"))
            {
                return Parse_If(cell);
            }
            else if (symbol == Sym::Symbol("_set!"))
            {
                return Parse_Set(cell);
            }
            else if (symbol == Sym::Symbol("_begin"))
            {
                return Parse_Begin(cell, topLevel);
            }
            else if (symbol == Sym::Symbol("_define"))
            {
                return Parse_Define(cell,topLevel);
            }
            else if (symbol == Sym::Symbol("_lambda"))
            {
                return Parse_Lambda(cell);
            }
            else if (symbol == Sym::Symbol("_quasiquote"))
            {
                THROW_ERROR_IF(cell->Length() != 2, cell, "Quasiquote does not take " << cell->Length() << " arguments");
                return Parse_Quasiquote(cell->Cdr());
            }
        }
    }
    
    // Not a list, just return it.
    return cell;
}

}
}