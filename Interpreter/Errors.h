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
#include "Cell.h"

namespace Jorvik
{
namespace Scheme
{

#define FORMAT(ITEMS)                                               \
( ( dynamic_cast<ostringstream &> (                                 \
        ostringstream() . seekp( 0, ios_base::cur ) << ITEMS )      \
) . str() )

#define THROW_ERROR(cells, text)     \
{                       \
    std::ostringstream strStream;   \
    strStream << text << std::endl << "In Expression: " << cells << std::endl;  \
    throw std::runtime_error(strStream.str().c_str());  \
}

#define THROW_ERROR_IF(pred, cells, text)     \
{                       \
    if ((pred))        \
    {                   \
        THROW_ERROR(cells, text) \
    }                                                       \
}

}
}