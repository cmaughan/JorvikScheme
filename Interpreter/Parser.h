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

#include <functional>

namespace Jorvik
{

namespace Scheme
{

class Evaluator;

// Given a cell, will Parse it without interpreting it.
// Checks for errors and makes simplified expressions into full expressions, such 
// as converting (define (a foo) (+ foo foo)) into (define a (lambda (foo) (+ foo foo))
// Parsing can be done 'up front', and the returned cells interpreted later on.
class Parser
{
public: 
    Parser(Evaluator* pScheme)
        : _pScheme(pScheme)
    {
    }

    Cell* Parse(Cell* cell) { return Parse_Cell(cell, true); }
    Cell* Parse_Cell(Cell* cell, bool topLevel = false);
    Cell* Parse_Cells(Cell* cell, bool topLevel = false);
    
    // Expressions the parser manipulates
    Cell* Parse_If(Cell* cell);
    Cell* Parse_Quote(Cell* cell);
    Cell* Parse_Set(Cell* cell);
    Cell* Parse_Define(Cell* cell, bool topLevel);
    Cell* Parse_Begin(Cell* cell, bool topLevel);
    Cell* Parse_Lambda(Cell* cell);
    Cell* Parse_Quasiquote(Cell* cell);

    typedef std::function<Cell*(Cell* cell)> tParseProc;
    Cell* AppendCells(Cell* pSource, Cell* pTarget, tParseProc proc = nullptr);

private:
    Evaluator* _pScheme;
};

}
}
