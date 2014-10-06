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

#include <string>

namespace Jorvik
{
namespace Scheme
{

class Sym
{
public:
    Sym(const std::string& str) 
        : _symbol(str)
    {
    }

    static const Sym* Symbol(const char* pszValue) 
    {
        return Sym::Symbol(std::string(pszValue));
    }

    static const Sym* Symbol(const std::string& value)
    {
        tTextToSymbol::iterator itrFound = MapTextToSymbol.find(value);
        if (itrFound != MapTextToSymbol.end())
        {
            return itrFound->second;
        }
        Sym* pSym = new Sym(value);
        MapTextToSymbol[value] = pSym; 
        return pSym;
    }

    static bool IsSymbol(const std::string& str)
    {
        tTextToSymbol::iterator itrFound = MapTextToSymbol.find(str);
        if (itrFound != MapTextToSymbol.end())
        {
            return true;
        }
        return false;
    }

    static void Dump()
    {
        for (auto val : MapTextToSymbol)
        {
            std::cout << val.first << std::endl;
        }
    }

    operator const std::string& () const { return _symbol; } 

private:
    typedef std::map<std::string, const Sym*> tTextToSymbol;
    static tTextToSymbol MapTextToSymbol;
    std::string _symbol;
};

} // Scheme
} // Jorvik