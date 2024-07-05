/*
 * This file is part of sidplayfp, a console SID player.
 *
 * Copyright 2014-2016 Leandro Nini
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#ifndef TYPES_H
#define TYPES_H

#include "sidfstream.h"

# define SID_WIFSTREAM sid_wifstream
# define SID_WOFSTREAM sid_wofstream
# define SID_IFSTREAM sid_ifstream
# define SID_OFSTREAM sid_ofstream

# define SID_STRING std::string
# define SID_STRINGTREAM std::stringstream
# define SID_COUT std::cout
# define SID_CERR std::cerr

# define TCHAR   char
# define TEXT(x) x
# define SEPARATOR "/"

#endif
