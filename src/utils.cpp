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

#include "utils.h"

#include <cstdlib>

#ifdef UNICODE
#  define _tgetenv _wgetenv
#else
#  define _tgetenv getenv
#endif

SID_STRING utils::getPath(const char* id, const char* def) {
    SID_STRING returnPath;

    char *path = std::getenv(id);
    if (!path) {
        path = std::getenv("HOME");
        if (!path)
            throw error();
        returnPath.append(path).append(def);
    } else
        returnPath.append(path);

    return returnPath;
}

SID_STRING utils::getDataPath() {
	return getPath("XDG_DATA_HOME", "/.local/share");
}

SID_STRING utils::getConfigPath() {
	return getPath("XDG_CONFIG_HOME", "/.config");
}
