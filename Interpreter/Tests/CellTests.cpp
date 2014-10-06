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

#ifdef TARGET_TESTS

#include "../Cell.h"
#include "../Evaluator.h"
#include "../Tokenizer.h"
#include "../Interpreter.h"
#include "../Parser.h"
#include "../Errors.h"
#include "../Scope.h"

#include "googletest/include/gtest/gtest.h"
#include "googlemock/include/gmock/gmock.h"

using namespace ::testing;
using namespace Jorvik::Scheme;

namespace JorvikCellTests
{

class JorvikCell : public Test
{
public:
    Evaluator eval;
};

// (cons '() 0)
// Cell(())->Cons(0)
TEST_F(JorvikCell, ConsToEmpty)
{
    Cell* pCell = Cell::EmptyList();
    pCell = pCell->Cons(Cell::Integer(0));
    ASSERT_THAT(pCell->ToString(), StrEq("(() . 0)"));
};

TEST_F(JorvikCell, IntegerConsEmpty)
{
    Cell* pCell = Cell::Integer(0);
    pCell = pCell->Cons(Cell::EmptyList());
    ASSERT_THAT(pCell->ToString(), StrEq("(0)"));
};

TEST_F(JorvikCell, ConsTwoEmptyLists)
{
    Cell* pCell = Cell::EmptyList();
    pCell = pCell->Cons(Cell::EmptyList());
    ASSERT_THAT(pCell->ToString(), StrEq("(())"));
};

TEST_F(JorvikCell, ConsThreeEmptyLists)
{
    // (cons (cons '() '()) '())
    Cell* pCell = Cell::EmptyList();
    pCell = pCell->Cons(Cell::EmptyList());
    pCell = pCell->Cons(Cell::EmptyList());
    ASSERT_THAT(pCell->ToString(), StrEq("((()))"));
};

TEST_F(JorvikCell, ConsAListOfThree)
{
    // (cons 0 (cons 1 (cons 2 '())))
    Cell* pCell = Cell::Integer(0)->Cons(Cell::Integer(1)->Cons(Cell::Integer(2)->Cons(Cell::EmptyList())));
    ASSERT_THAT(pCell->ToString(), StrEq("(0 1 2)"));
};

TEST_F(JorvikCell, ConsToAList)
{
    // (cons '(0 1 2) 3)
    Cell* pCell = Cell::Integer(0)->Cons(Cell::Integer(1)->Cons(Cell::Integer(2)->Cons(Cell::EmptyList())));
    pCell = pCell->Cons(Cell::Integer(3));
    ASSERT_THAT(pCell->ToString(), StrEq("((0 1 2) . 3)"));
};

TEST_F(JorvikCell, ConsAListToAList)
{
    // (cons '(0 1 2) '(3))
    Cell* pCell = Cell::Integer(0)->Cons(Cell::Integer(1)->Cons(Cell::Integer(2)->Cons(Cell::EmptyList())));
    pCell = pCell->Cons(Cell::Pair(Cell::Integer(3)));
    ASSERT_THAT(pCell->ToString(), StrEq("((0 1 2) 3)"));
};

TEST_F(JorvikCell, ConsAListToAListOtherWay)
{
    // (cons '(3) '(0 1 2))
    Cell* pCell = (Cell::Pair(Cell::Integer(3)));
    pCell = pCell->Cons(Cell::Integer(0)->Cons(Cell::Integer(1)->Cons(Cell::Integer(2)->Cons(Cell::EmptyList()))));
    
    ASSERT_THAT(pCell->ToString(), StrEq("((3) 0 1 2)"));
};
}; // JorvikCellTests

#endif