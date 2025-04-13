/*
 * This file is part of C64play, a SID player engine.
 *
 * Copyright 2025 Enki Costa
 * Copyright 2014-2015 Leandro Nini
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SIDCXX11_H
#define SIDCXX11_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_CXX17
#  define HAVE_CXX14
#  define MAYBE_UNUSED [[ maybe_unused ]]
#else
#  define MAYBE_UNUSED
#endif

#ifdef HAVE_CXX14
#  define HAVE_CXX11
#endif

#ifndef HAVE_CXX11
#  error "This is not a C++11 compiler"
#endif

#endif
