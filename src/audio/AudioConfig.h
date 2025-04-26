/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2024-2025 Enki Costa
 * Copyright 2000 Simon White
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

#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <stdint.h>

class AudioConfig {
public:
    uint32_t sampleRate; // sample rate (Hz)
    uint8_t  depth;      // bit depth
    uint8_t  channels;   // audio channels
    uint16_t bufSize;    // sample buffer size

	// defaults
    AudioConfig() :
        sampleRate(48000),
        depth(16),
        channels(1),
        bufSize(0) {}

	uint_least32_t getBufBytes() const { return bufSize * channels * (depth/8); }
};

#endif  // AUDIOCONFIG_H
