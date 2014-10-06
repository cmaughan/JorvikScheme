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

class Tokenizer;
class Parser;
class Interpreter;
class Cell;
class Scope;

// A custom error returned when the expression is not complete
class incomplete_expression_error : public std::runtime_error
{
public:
    incomplete_expression_error(const char* pszError)
        : runtime_error(pszError)
    {
    }
};

class Evaluator
{
public: 
    enum DebugFlag
    {
        Debug = 1
    };

    Evaluator();

    Cell* Evaluate(const std::string& input);

    Cell* Tokenize(const std::string& input);
    Cell* Parse(Cell* cell);
    Cell* Interpret(Cell* cell);
    
    Scope* GetGlobalScope() { return _globalScope.get(); }
    
    static const bool TestDebugFlag(DebugFlag flag) { return DebugFlags & flag ? true : false; }
    static void SetDebugFlag(DebugFlag flag) { DebugFlags |= (unsigned int)flag; }
    static void ClearDebugFlag(DebugFlag flag) { DebugFlags &= (unsigned int)~flag; }
    

private:
    // Setup
    void AddSymbols();
    void AddIntrinsics();

private:
    static unsigned int DebugFlags; 

    std::shared_ptr<Scope> _globalScope;
    std::unique_ptr<Parser> _parser;
    std::unique_ptr<Tokenizer> _tokenizer;
    std::unique_ptr<Interpreter> _interpreter;
};

} // Scheme
} // Jorvik