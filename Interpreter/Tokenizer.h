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

#include <regex>

namespace Jorvik 
{
namespace Scheme 
{

class Sym;
class Evaluator;

class Tokenizer
{
public:
    Tokenizer(Evaluator* pScheme);
    Cell* Tokenize(const std::string& input);

    static const std::regex& TokenizerRegex() { return regexTokens; }
    static const std::regex& TokenizerNumberRegex() { return regexNumber; }

private:
    std::string NextToken();
    Cell* TokenizeToken(const std::string& token);
    Cell* Atom(const std::string& token) const;
    bool IsNumber(const std::string& token, bool& isFloat) const;

private:
    static const std::regex regexTokens;
    static const std::regex regexNumber;    
    std::sregex_iterator _tokenItr;

    Evaluator* _pScheme;

    // Mappings for the tokenizer.
    std::map<std::string, const Sym*> _quoteMappings;
};

} // Scheme
} // Jorvik