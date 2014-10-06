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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;
using namespace Jorvik::Scheme;

namespace JorvikParseTests
{

class JorvikParse : public Test
{
public:
    Evaluator eval;
};

// Check that input -> output is correct
// The parser comes after the tokenizer, and Parses/fixes expressions
// As well as implementing macros in future
#define JORVIK_PARSE(name, a, b)                        \
TEST_F(JorvikParse, name)                               \
{                                                       \
    ASSERT_THAT(eval.Parse(eval.Tokenize(a))->ToString(), StrEq(b));    \
};

// Check for throw during bad token stream
#define JORVIK_PARSE_THROW(name, a)                     \
TEST_F(JorvikParse, name)                               \
{                                                       \
    ASSERT_THROW(eval.Parse(eval.Tokenize(a)), std::runtime_error);     \
};

JORVIK_PARSE(DefineLambdaMultiBody, "(define (sum a) (+ a a) a)", "(_define sum (_lambda (a) (_begin (+ a a) a)))");
JORVIK_PARSE(LambdaSingleArg, "(lambda a (+ a a))", "(_lambda a (+ a a))");
JORVIK_PARSE(LambdaMultiBody, "(lambda a (+ a a) (* a a))", "(_lambda a (_begin (+ a a) (* a a)))")
JORVIK_PARSE(LambdaMultiArgMultiBody, "(lambda (a b) (+ a a) (* a a))", "(_lambda (a b) (_begin (+ a a) (* a a)))")
JORVIK_PARSE(DefineToLambda, "(define (add a b) (+ a b))", "(_define add (_lambda (a b) (+ a b)))");
JORVIK_PARSE(IfParsesWithNone, "(if #f 1)", "(_if #f 1)");
JORVIK_PARSE_THROW(IfIncorrectArguments, "(if a)");
JORVIK_PARSE_THROW(TooManyQuoteArgs, "(quote 2 2)");
JORVIK_PARSE(BeginExpandsAllExpressions, "(begin (define (sum a) (+ a a)) '(a b))", "(_begin (_define sum (_lambda (a) (+ a a))) (_quote (a b)))");
JORVIK_PARSE(BeginEmptyReturnsEmpty, "(begin)", "");

#ifndef SUPPORT_QUASIQUOTE_UNQUOTE_SPLICE
JORVIK_PARSE(QuasiBecomesQuote, "`(hello)", "(_quote (hello))");
#else
JORVIK_PARSE(QuasiquoteComplex, "'(hello ,(+ 2 2))", "'(hello (+ 2 2))");
JORVIK_PARSE(QuasiquoteUnquoteAtom, "`'2", "(cons (_quote _quote) (cons (_quote 2) (_quote ())))"); // (_unquote (_quote ,2)) = (_quote 2)
JORVIK_PARSE(QuasiquoteAtom, "`2", "(_quote 2)");
#endif

}; // JorvikTokenizeTests

#endif