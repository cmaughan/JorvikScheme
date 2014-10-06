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

#include "../Evaluator.h"
#include "../Tokenizer.h"
#include "../Interpreter.h"
#include "../Cell.h"
#include "../Parser.h"
#include "../Errors.h"
#include "../Scope.h"

#include "googletest/include/gtest/gtest.h"
#include "googlemock/include/gmock/gmock.h"

using namespace ::testing;
using namespace Jorvik::Scheme;

namespace JorvikTokenizeTests
{

class JorvikTokenize : public Test
{
public:
    Evaluator eval;
};

// Check that input -> output is correct
#define JORVIK_TOKENIZE(name, a, b)                     \
TEST_F(JorvikTokenize, name)                            \
{                                                       \
    ASSERT_THAT(eval.Tokenize(a)->ToString(), StrEq(b));    \
};

// Check for throw during bad token stream
#define JORVIK_TOKENIZE_THROW(name, a)                  \
TEST_F(JorvikTokenize, name)                            \
{                                                       \
    ASSERT_THROW(eval.Tokenize(a), std::runtime_error);     \
};

// Tokenizing

// Failure cases
JORVIK_TOKENIZE_THROW(IncompleteMissingBracket, "(+ 2 2");

// Tokenize conversions
JORVIK_TOKENIZE(CommentIgnored, "; hello", ""); // No expressions
JORVIK_TOKENIZE(Define, "(define a 3)", "(_define a 3)");
JORVIK_TOKENIZE(QuotedExpression, "'(+ 2 2)", "(_quote (+ 2 2))");
JORVIK_TOKENIZE(QuasiQuotedExpression, "`(+ 2 2)", "(_quasiquote (+ 2 2))");
JORVIK_TOKENIZE(LineAfterCommentRead, "; hello\n(+ 2 2)", "(+ 2 2)");
JORVIK_TOKENIZE(CommentOnEnd, "(+ 2 2); hello", "(+ 2 2)");
JORVIK_TOKENIZE(SimpleExpression, "(+ (+ 2 2) (+ 3 3))", "(+ (+ 2 2) (+ 3 3))");
JORVIK_TOKENIZE(Symbol, "ab34234.2343", "ab34234.2343");
JORVIK_TOKENIZE(False, "#f", "#f");
JORVIK_TOKENIZE(True, "#t", "#t");
JORVIK_TOKENIZE(EmbeddedLists, "((1) (2 3 4) (5 6) (7) (8 9))", "((1) (2 3 4) (5 6) (7) (8 9))");

TEST_F(JorvikTokenize, IdentifiersAreValid)
{
    // These from the scheme spec.  They are all valid symbols.
    ASSERT_THAT(*eval.Tokenize("lambda")->GetSymbol(), StrEq("_lambda"));
    ASSERT_THAT(*eval.Tokenize("list->vector")->GetSymbol(), StrEq("list->vector"));
    ASSERT_THAT(*eval.Tokenize("+")->GetSymbol(), StrEq("+"));
    ASSERT_THAT(*eval.Tokenize("<=?")->GetSymbol(), StrEq("<=?"));
    ASSERT_THAT(*eval.Tokenize("->string")->GetSymbol(), StrEq("->string"));
    ASSERT_THAT(*eval.Tokenize("|two words|")->GetSymbol(), StrEq("|two words|"));
    ASSERT_THAT(*eval.Tokenize("string-of-words")->GetSymbol(), StrEq("string-of-words"));
    ASSERT_THAT(*eval.Tokenize("q")->GetSymbol(), StrEq("q"));
    ASSERT_THAT(*eval.Tokenize("+soup+")->GetSymbol(), StrEq("+soup+"));
    ASSERT_THAT(*eval.Tokenize("V17a")->GetSymbol(), StrEq("V17a"));
    ASSERT_THAT(*eval.Tokenize("a34kTMNs")->GetSymbol(), StrEq("a34kTMNs"));
    ASSERT_THAT(*eval.Tokenize("...")->GetSymbol(), StrEq("..."));
    ASSERT_THAT(*eval.Tokenize("two\\x20;words")->GetSymbol(), StrEq("two\\x20;words"));
}

TEST_F(JorvikTokenize, EscapedString)
{
    ASSERT_THAT(eval.Tokenize("\"escaped\"")->GetString(), StrEq("escaped"));
}

TEST_F(JorvikTokenize, ToStringContainsEscapedString)
{
    ASSERT_THAT(eval.Tokenize("(\"escaped\" \"two\")")->ToString(), StrEq("(\"escaped\" \"two\")"));
}

// Ensure the tokenizer converts these to bool types
TEST_F(JorvikTokenize, RecognizesBoolean)
{
    ASSERT_THAT(eval.Tokenize("#f")->GetBool(), Eq(false));
    ASSERT_THAT(eval.Tokenize("#t")->GetBool(), Eq(true));
    ASSERT_THROW(eval.Tokenize("#f")->GetSymbol(), std::runtime_error );
}

// Ensure the tokenizer converts these to integer types
TEST_F(JorvikTokenize, RecognizesInteger)
{
    ASSERT_THAT(eval.Tokenize("12345")->GetInteger(), Eq(12345));
    ASSERT_THAT(eval.Tokenize("-12345")->GetInteger(), Eq(-12345));
}

// Ensure the tokenizer converts these to float type
TEST_F(JorvikTokenize, RecognizesFloat)
{
    ASSERT_THAT(eval.Tokenize(".12345")->GetFloat(), Eq((tCellFloat).12345));
    ASSERT_THAT(eval.Tokenize(".12345")->GetFloat(), Eq((tCellFloat).12345));

    ASSERT_THAT(eval.Tokenize("1.12345")->GetFloat(), Eq((tCellFloat)1.12345));
    ASSERT_THAT(eval.Tokenize("12345.12345")->GetFloat(), Eq((tCellFloat)12345.12345));
    
    ASSERT_THAT(eval.Tokenize("-.12345")->GetFloat(), Eq((tCellFloat)-.12345));
    ASSERT_THAT(eval.Tokenize("-1.12345")->GetFloat(), Eq((tCellFloat)-1.12345));
}

// (+ a b) == (car.cdr.cdr.Cdr : list(+, a, b))
TEST_F(JorvikTokenize, HasListOfAtoms)
{
    const Jorvik::Scheme::Cell& cell = *eval.Tokenize("(+ a b)");
    ASSERT_THAT(cell.Length(), Eq(3));
    ASSERT_THAT(*cell.Car()->GetSymbol(), StrEq("+"));
    ASSERT_THAT(*cell.Cdr()->Car()->GetSymbol(), StrEq("a"));
    ASSERT_THAT(*cell.Cdr()->Cdr()->Car()->GetSymbol(), StrEq("b"));
}

}; // JorvikTokenizeTests

#endif