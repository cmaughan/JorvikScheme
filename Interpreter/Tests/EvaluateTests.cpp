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

namespace JorvikEvaluateTests
{

class JorvikEvaluate : public Test
{
public:
    Evaluator eval;
};

#define CHECK_EVAL(a, b) ASSERT_THAT(eval.Interpret(eval.Parse(eval.Tokenize(a)))->ToString(), StrEq(b))
#define JORVIK_EVALUATE(name, a, b)                     \
TEST_F(JorvikEvaluate, name)                          \
{                                                   \
    Cell* tokens = eval.Tokenize(a);    \
    Cell* parse = eval.Parse(tokens);         \
    Cell* interpreted = eval.Interpret(parse); \
    ASSERT_THAT(parse->ToString(), StrNe(interpreted->ToString())); \
    ASSERT_THAT(interpreted->ToString(), StrEq(b));                 \
};

#define CHECK_EVAL_THROW(a) ASSERT_THROW(eval.Interpret(eval.Parse(eval.Tokenize(a))), std::runtime_error)
#define JORVIK_EVALUATE_THROW(name, a)                  \
TEST_F(JorvikEvaluate, name)                            \
{                                                       \
    CHECK_EVAL_THROW(a);                                \
};

JORVIK_EVALUATE_THROW(DefineInvalidArgs, "(define 3 4)");
JORVIK_EVALUATE_THROW(QuoteInvalidArgs, "(quote 1 2)");
JORVIK_EVALUATE_THROW(IfStatementInvalidArgs, "(if 1 2 3 4)");
JORVIK_EVALUATE_THROW(LambdaBadArgs, "(lambda 3 3)");
JORVIK_EVALUATE_THROW(LambdaNoBody, "(lambda (x) )");
JORVIK_EVALUATE_THROW(DefineTwiceCallIncorrect, "(begin (define (twice x) (* 2 x)) (twice 2 2))");
JORVIK_EVALUATE_THROW(EmptyListInvalid, "()");
JORVIK_EVALUATE_THROW(NotAProcedure, "(1 2)");

JORVIK_EVALUATE(LambdaReturnsLambda, "((lambda (x) (+ x x)) 3)", "6");
JORVIK_EVALUATE(Quasiquote, "`(+ 2 2)", "(+ 2 2)");
JORVIK_EVALUATE(DefineTwice, "(define (twice x) (*2 x))", "");
JORVIK_EVALUATE(DefineTwiceWorks, "(begin (define (twice x) (* 2 x)) (twice 2))", "4");
JORVIK_EVALUATE(DefineAList, "(define lyst (lambda items items))", "");
JORVIK_EVALUATE(DefineAListUseIt, "(begin (define lyst (lambda items items)) (lyst 1 2 3 (+ 2 2)))", "(1 2 3 4)");
JORVIK_EVALUATE(IfAlt, "(if 1 2)", "2");
JORVIK_EVALUATE(IfConseq, "(if (= 3 4) 2)", "<procedure>");
JORVIK_EVALUATE(MakeAList, "(list 1 '(2 3) 4)", "(1 (2 3) 4)");

// Simple evaluation: input -> output
// No bignum support yet: JORVIK_EVALUATE(Quote, "(quote (testing 1 (2.0) -3.14e159))", "(testing 1 (2.0) -3.14e159)");
JORVIK_EVALUATE(Add, "(+ 2 2)", "4");
JORVIK_EVALUATE(AddMultiply, "(+ (* 2 100) (* 1 10))", "210");
JORVIK_EVALUATE(IfGreater, "(if (> 6 5) (+ 1 1) (+ 2 2))", "2");
JORVIK_EVALUATE(IfLess, "(if (< 6 5) (+ 1 1) (+ 2 2))", "4");
JORVIK_EVALUATE(Begin, "(begin (define x 1) (set! x (+ x 1)) (+ x 1))", "3");
JORVIK_EVALUATE(DefineLambda, "((lambda (x) (+ x x)) 5)", "10");
JORVIK_EVALUATE(DefineVariable, "(define x 3)", "");
JORVIK_EVALUATE(DefineAndReadVariable, "(begin (define x 3) x)", "3");
JORVIK_EVALUATE(AddVariableToItself, "(begin (define x 3) (+ x x))", "6");
JORVIK_EVALUATE(LambdaAsArg, "(begin (define sum (lambda (arg) (+ arg arg))) (define proc (lambda (arg) (arg 2))) (proc sum))", "4");
JORVIK_EVALUATE(List, "(list '(3 4) '(5 6))", "((3 4) (5 6))");
JORVIK_EVALUATE(Car, "(car '(3 4 5))", "3");
JORVIK_EVALUATE(Cdr, "(cdr '(3 4 5))", "(4 5)");
JORVIK_EVALUATE(CdrSingle, "(cdr '(3 4))", "(4)");
JORVIK_EVALUATE(Cons, "(cons '(5) '(6))", "((5) 6)");
JORVIK_EVALUATE(ConsImproperList, "(cons 1 (cons 2 3))", "(1 2 . 3)");
JORVIK_EVALUATE(QuoteX, "(quote x)", "x");
JORVIK_EVALUATE(ShortQuoteX, "'x", "x");
JORVIK_EVALUATE(QuoteList, "(quote (1 2 three))", "(1 2 three)");

const char* multiLine = R"(
'(1 ;test comments ' ;skip this line                
     2 ; more ; comments ; ) )      
     3) ; final comment",)";

JORVIK_EVALUATE(MutliLineComments, multiLine, "(1 2 3)");

// Multiline programs
TEST_F(JorvikEvaluate, VariableScopes)
{
    CHECK_EVAL("(define ((account bal) amt) (set! bal (+ bal amt)) bal)", "");
    CHECK_EVAL("(define a1 (account 100))", "");
    CHECK_EVAL("((account 100) 0)", "100");
    CHECK_EVAL("(a1 0)", "100");
    CHECK_EVAL("(a1 10)", "110");
    CHECK_EVAL("(a1 10)", "120");
}

TEST_F(JorvikEvaluate, Lambdas)
{
    CHECK_EVAL("(define twice (lambda (x) (* 2 x)))", "");
    CHECK_EVAL("(twice 5)", "10");
    CHECK_EVAL("(define compose (lambda (f g) (lambda (x) (f (g x)))))", "");
    CHECK_EVAL("((compose list twice) 5)", "(10)");
    CHECK_EVAL("(define repeat (lambda (f) (compose f f)))", "");
    CHECK_EVAL("((repeat twice) 5)", "20");
    CHECK_EVAL("((repeat (repeat twice)) 5)", "80");
};

TEST_F(JorvikEvaluate, Factorial)
{
    CHECK_EVAL("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", "");
    CHECK_EVAL("(fact 3)", "6");
    // No bignum support yet : CHECK_EVAL("(fact 50)", "30414093201713378043612608166064768844377641568960512000000000000");
    CHECK_EVAL("(fact 12)", "479001600"); // no bignums; this is as far as we go with 32 bits
};

TEST_F(JorvikEvaluate, Abs)
{
    CHECK_EVAL("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", "");
    CHECK_EVAL("(list (abs -3) (abs 0) (abs 3))", "(3 0 3)");
};

TEST_F(JorvikEvaluate, Zip)
{
    CHECK_EVAL(("(define combine (lambda (f)"
                 "(lambda (x y)"
                    "(if (null? x) (quote ())"
                    "(f (list (car x) (car y))"
                    "((combine f) (cdr x) (cdr y)))))))"), "");
    CHECK_EVAL("(define zip (combine cons))", "");
    CHECK_EVAL("(zip (list 1 2 3 4) (list 5 6 7 8))", "((1 5) (2 6) (3 7) (4 8))");
};

TEST_F(JorvikEvaluate, RiffShuffle)
{
    CHECK_EVAL("(define (append l m) (if (null? l) m (cons (car l) (append (cdr l) m))))", "");
    CHECK_EVAL("(define compose (lambda (f g) (lambda (x) (f (g x)))))", "");
    CHECK_EVAL("(define repeat (lambda (f) (compose f f)))", "");

    CHECK_EVAL(("(define combine (lambda (f)"
                 "(lambda (x y)"
                    "(if (null? x) (quote ())"
                    "(f (list (car x) (car y))"
                    "((combine f) (cdr x) (cdr y)))))))"), "");
    
    CHECK_EVAL(("(define riff-shuffle (lambda (deck) (begin"
            "(define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq))))))"
            "(define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (cdr seq)))))"
            "(define mid (lambda (seq) (/ (length seq) 2)))"
            "((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))"), "");
    CHECK_EVAL("(riff-shuffle (list 1 2 3 4 5 6 7 8))", "(1 5 2 6 3 7 4 8)");
    CHECK_EVAL("((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))",  "(1 3 5 7 2 4 6 8)");
    CHECK_EVAL("(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))", "(1 2 3 4 5 6 7 8)");
};


}; // JorvikEvaluateaTests

#endif
