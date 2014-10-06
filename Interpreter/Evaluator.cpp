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
#include "Interpreter.h"
#include "Tokenizer.h"
#include "SchemeInit.h"
#include "Scope.h"
#include "Cell.h"

namespace Jorvik
{
namespace Scheme
{

Sym::tTextToSymbol Sym::MapTextToSymbol;

unsigned int Evaluator::DebugFlags = 0;//Evaluator::Debug;

Evaluator::Evaluator()
    : _globalScope(new Scope())
{
    Cell::StaticInit();

    AddSymbols();
    
    _parser.reset(new Parser(this));
    _interpreter.reset(new Interpreter(this));
    _tokenizer.reset(new Tokenizer(this));

    Evaluate(SchemeInit);
}

void Evaluator::AddSymbols()
{
    // Add our global symbols.  Map "Sym" -> "_Sym"
    const char* sym[] { "debug", "quote", "Parse", "#t", "#f", "if", "set!", "define", "lambda", "begin", "quasiquote", "unquote", "unquote-splicing" };
    std::for_each(std::begin(sym), std::end(sym), [&](const char* s) { GetGlobalScope()->AddVariable(Sym::Symbol(s), Cell::Symbol(Sym::Symbol(std::string("_") + std::string(s)))); } );
}


Cell* Evaluator::Parse(Cell* cell)
{
    return _parser->Parse(cell);
}

Cell* Evaluator::Tokenize(const std::string& input)
{
    return _tokenizer->Tokenize(input);
}

Cell* Evaluator::Interpret(Cell* cell)
{
    return _interpreter->Interpret(cell, _globalScope);
}

Cell* Evaluator::Evaluate(const std::string& input)
{
    return Interpret(_parser->Parse(_tokenizer->Tokenize(input)));
}


} // Scheme
} // Jorvik
