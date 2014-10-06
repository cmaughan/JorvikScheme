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
#include "Tokenizer.h"
#include "Evaluator.h"
#include "Scope.h"

// This tokenizer uses a regex to extract all tokens used by the parser.
// It also recognizes quoting, and builds linked Cells ready for parsing.
// Atoms are turned into floats, integers, strings, symbols & booleans.
// See the TokenizeTests for examples of what is expected.
namespace Jorvik
{
namespace Scheme
{

// Regex for tokenizer
// 1. (WHITESPACE)
// 2. GROUP(
// 3. BOTH (,@) : unquote-splicing
// 4. OR ONE ('`,) : quote, quasiquote, unquote 
// 5. OR ONE (") THEN ANYNUM (NOGROUP) (ONE (\) OR (NONE_OF(\")) THEN (") - matches a string
// 6. OR ONE (|) THEN (ANY*) THEN (|) - matches a symbol surrounded by ||
// 7. OR ONE (;) THEN MANY (ANY.) : comment - note that '.' doesn't include newline
// 8. OR MANY (NONE_OFF (WHITESPACE,'"`,) : text, digits, operators, etc.
// 9. END_GROUP)
const std::regex Tokenizer::regexTokens(R"(\s*(,@|[('`,)]|"(?:[\\].|[^\\"])*"|\|.*\||;.*|[^\s('"`,)]*))");

// Regex to match any type of number (+/- num . num e- num)
const std::regex Tokenizer::regexNumber(R"(^[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)");

Tokenizer::Tokenizer(Evaluator* pScheme)
    : _pScheme(pScheme)
{
    // Add some mappings for quoting to symbols
    _quoteMappings["'"] = pScheme->GetGlobalScope()->FindVariable(Sym::Symbol("quote"))->GetSymbol();
    _quoteMappings["`"] = pScheme->GetGlobalScope()->FindVariable(Sym::Symbol("quasiquote"))->GetSymbol();
#ifdef SUPPORT_QUASIQUOTE_UNQUOTE_SPLICE
    _quoteMappings[","] = pScheme->GetGlobalScope()->FindVariable(Sym::Symbol("unquote"))->GetSymbol();
    _quoteMappings[",@"] = pScheme->GetGlobalScope()->FindVariable(Sym::Symbol("unquote-splicing"))->GetSymbol();
#endif

}

// Retrieve the next token from the input iterator
// Skips comments.
std::string Tokenizer::NextToken() 
{
    std::sregex_iterator it_end;
    while(_tokenItr != it_end) 
    {
        if ((*_tokenItr).size() > 1)
        {
            std::string token = (*_tokenItr)[1].str();
            if (!token.empty() && token[0] != ';')
            {
                _tokenItr++;
                return token;
            }
        }
        _tokenItr++;
    }

    return "";
}

// A number in scheme is anything containing only number digits.
bool Tokenizer::IsNumber(const std::string& str, bool& isFloat) const
{
    // Use the regex to detect, and note the '.' for floats.
    if (std::regex_search(str, regexNumber))
    {
        isFloat = (str.find('.') != std::string::npos);
        return true;
    }

    return false;
}

// An atom is bool, float, int, symbol or string
Cell* Tokenizer::Atom(const std::string& token) const
{
    if (token == "#t")
    {
        return Cell::Boolean(true);
    }
    else if (token == "#f")
    {
        return Cell::Boolean(false);
    }

    // If it only contains number pieces, then it is a number.
    bool isFloat;
    if (IsNumber(token, isFloat))
    {
        if (isFloat)
        {
            try
            {
                tCellFloat val = (tCellFloat)std::stod(token);
                return Cell::Float(val);
            }
            catch(...)
            {
                throw new std::runtime_error(std::string("Not a Float: " + token));
            }
        }
        else
        {
            try
            {
                tCellInteger val = (tCellInteger)std::stoll(token);
                return Cell::Integer(val);
            }
            catch(...)
            {
                throw new std::runtime_error(std::string("Not an Integer: " + token));
            }
        }
    }
    
    // Check symbol table and return any mappings from symbol->symbol.
    // We ignore mappings to functions at the tokenize stage
    auto cell = _pScheme->GetGlobalScope()->FindVariable(Sym::Symbol(token));
    if (cell != nullptr &&
        cell->GetType() & Cell::SymbolType)
    {
        return cell;
    }

    if (token[0] == '"')
    {
        return Cell::String(token.substr(1, token.length() - 2).c_str());
    }
    return Cell::Symbol(Sym::Symbol(token));
}

// Parse a given token
// We are building a linked list of pairs...

//    P
//    +  -
//    *  P
//       + -
//       2 P
//         + -
//         2 0
//
// == (* 2 2)
// CDR() = P(2 2)

// P   
// +   -          
// P   P       
// +-  + 
// A0  P      
//     +   -
//     B   P
//     +-  +   -
//     P   P   P
//     +-  +-  +-
//     C0  D0  00
//
// == ((A) (B C) (D))
// CAR() == P+-A0 == (A)
// CDR() == P+P+B... == ((B C) (D))
// CAR(CDR() == (B C) 
//

Cell* Tokenizer::TokenizeToken(const std::string& token)
{
    if (token[0] == '(')
    {
        // Return this cell as the next.
        // It's an open bracket, so we know it's going in as the CAR
        // We start with an empty list, of course.
        Cell* pRet = Cell::EmptyList();

        // Append the next token's contents to our list.
        for(;;)
        {
            std::string nextToken = NextToken();

            // Should at least see a bracket.
            if (nextToken.empty())
            {
                // An expression without a closer.  A syntax error.
                throw incomplete_expression_error("Unexpected end of file while parsing expression");
            }
            else if (nextToken[0] == ')')
            {
                // Completed the list
                return pRet;
            }
            else
            {
                // Append to our current list.
                pRet = pRet->Append(TokenizeToken(nextToken));
            }
        }
        // Unreachable
    }
    else if (token[0] == ')')
    {
        throw std::runtime_error("Unexpected ')'");
    }
  
    // Handle ('`@, ...), etc.
    // Quotes a new list
    auto itr = _quoteMappings.find(token);
    if (itr != _quoteMappings.end())
    {
        // (_quotesymbol ...)
        return Cell::Pair(Cell::Symbol(itr->second), Cell::Pair(TokenizeToken(NextToken())));       
    }
  
    // Must be an atom
    return Atom(token);
}

// Given a string, return our list of cells.
Cell* Tokenizer::Tokenize(const std::string& input)
{            
    _tokenItr = std::sregex_iterator(std::begin(input), std::end(input), regexTokens);

    Cell* pCell = TokenizeToken(NextToken());    
    if (pCell->IsSymbol() && pCell->GetSymbol() == Sym::Symbol(""))
    {
        return Cell::Void();
    }
    return pCell;
}


} // Jorvik
} // Scheme