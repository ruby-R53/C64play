/*
 * This file is part of C64play, a console player for SID tunes.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef SIDCXX11_H
#define SIDCXX11_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_CXX20
#  define HAVE_CXX17
#  define LIKELY [[ likely ]]
#  define UNLIKELY [[ unlikely ]]
#else
#  define LIKELY
#  define UNLIKELY
#endif

#ifndef HAVE_CXX17
#  error "This is not a C++17 compiler!"
#endif

#endif
