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

#include "pch.h"
#include "Evaluator.h"
#include "Cell.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Tokenizer.h"
#include "Errors.h"
#include "CellAllocator.h"
#include "Scope.h"

#include <iomanip>
#include <locale>

using namespace Jorvik::Scheme;

template<class T>
std::string FormatWithCommas(T value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

static void Repl()
{
    Evaluator eval;

    std::cout << "Jorvik Scheme: Version 1.0" << std::endl << std::endl;
    std::string prompt = "J >> ";
    for (;;) 
    {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        try
        {
            // Tokenize
            Cell* tokenized = eval.Tokenize(line);
            if (Evaluator::TestDebugFlag(Evaluator::Debug))
            {
                std::cout << "Tokenized: " << tokenized << std::endl;
            }

            // Parse
            Cell* parsed = eval.Parse(tokenized);
            if (Evaluator::TestDebugFlag(Evaluator::Debug))
            {
                std::cout << "Parsed: " << parsed << std::endl;
            }
            
            // Interpret
            Cell* interpreted = eval.Interpret(parsed);
            if (interpreted != nullptr && !interpreted->ToString().empty())
            {
                std::cout << interpreted << std::endl;
            }

            // Garbage collect
            CellAllocator::Instance().GarbageCollect(eval.GetGlobalScope());

            // Stats
            if (Evaluator::TestDebugFlag(Evaluator::Debug))
            {
                std::cout << "Free list Size: " << FormatWithCommas(CellAllocator::Instance().GetFreeListSize()) << std::endl;
                std::cout << "Alloc List Size: " << FormatWithCommas(CellAllocator::Instance().GetPoolSize()) << std::endl;

                unsigned int cells = CellAllocator::Instance().GetFreeListSize() + CellAllocator::Instance().GetPoolSize();
                std::cout << "Cells: " << FormatWithCommas(cells) << ", Bytes: " << FormatWithCommas(cells * sizeof(Cell)) << std::endl;
                std::cout << "Cell Size: " << sizeof(Cell) << std::endl;
            }
        }
        catch(const std::runtime_error& err)
        {
            std::cout << err.what() << std::endl;
        }        
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    Repl();
    return 0;
}

