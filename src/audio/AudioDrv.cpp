/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2000 Simon White
 * Copyright 2013-2016 Leandro Nini
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "AudioDrv.h"

// Unix Sound Drivers
#ifdef HAVE_PULSE
#  include "pulse/audiodrv.h"
#endif
#ifdef HAVE_ALSA
#  include "alsa/audiodrv.h"
#endif
#ifdef HAVE_OSS
#  include "oss/audiodrv.h"
#endif

// Warn if a sound driver is not found
// and fall back to the null driver
#ifndef AudioDriver
#  warning Audio hardware not recognised, only null driver will be available.
#  include "null/null.h"
#  define HAVE_NULL
#endif

bool audioDrv::open(AudioConfig &cfg) {
    bool res = false;
#ifdef HAVE_PULSE
    if(!res) {
        audio.reset(new Audio_Pulse());
        res = audio->open(cfg);
    }
#endif
#ifdef HAVE_ALSA
    if(!res) {
        audio.reset(new Audio_ALSA());
        res = audio->open(cfg);
    }
#endif
#ifdef HAVE_OSS
    if(!res) {
        audio.reset(new Audio_OSS());
        res = audio->open(cfg);
    }
#endif
#ifdef HAVE_NULL
    if(!res) {
        audio.reset(new Audio_Null());
        res = audio->open(cfg);
    }
#endif
    return res;
}
