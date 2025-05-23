/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2024-2025 Enki Costa
 * Copyright 2013-2017 Leandro Nini
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

#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "ini/types.h"

class utils {
public:
	class error {};

private:
	static SID_STRING getPath(const char* id, const char* def);

public:
	static SID_STRING getDataPath();
	static SID_STRING getConfigPath();
};

#endif
