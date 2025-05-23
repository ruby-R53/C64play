/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2024-2025 Enki Costa
 * Copyright 2013-2016 Leandro Nini
 * Copyright 2008 Antti Lankila
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

#include "audiodrv.h"

#include <new>
#include <pulse/error.h>

Audio_Pulse::Audio_Pulse() :
	AudioBase("PULSE") {
	outOfOrder();
}

Audio_Pulse::~Audio_Pulse() {
	close();
}

void Audio_Pulse::outOfOrder() {
	_sampleBuffer = nullptr;
	clearError();
}

bool Audio_Pulse::open(AudioConfig &cfg) {
	pa_sample_spec pacfg = {};

	pacfg.channels = cfg.channels;
	pacfg.rate	   = cfg.sampleRate;
	pacfg.format   = PA_SAMPLE_S16LE;

	// Set bit depth and encoding type.
	int err;
	_audioHandle = pa_simple_new(
		nullptr,
		"C64play",
		PA_STREAM_PLAYBACK,
		nullptr,
		"Playing",
		&pacfg,
		nullptr,
		nullptr,
		&err
	);

	try {
		if (! _audioHandle) {
			throw error(pa_strerror(err));
		}

		cfg.bufSize = 4096;

		try {
			_sampleBuffer = new short[cfg.bufSize];
		}
		catch (std::bad_alloc const &ba) {
			throw error("Unable to allocate memory for sample buffers!");
		}

		_settings = cfg;

		return true;
	}
	catch(error const  &e) {
		setError(e.message());

		if (_audioHandle)
			pa_simple_free(_audioHandle);

		_audioHandle = nullptr;

		return false;
	}
}

// Close an opened audio device, free any allocated buffers and
// reset any variables that reflect the current state.
void Audio_Pulse::close() {
	if (_audioHandle != nullptr) {
		pa_simple_free(_audioHandle);
		_audioHandle = nullptr;
	}

	if (_sampleBuffer != nullptr) {
		delete [] _sampleBuffer;
		outOfOrder();
	}
}

bool Audio_Pulse::write(uint_least32_t size) {
	if (_audioHandle == nullptr) {
		setError("Device not open.");

		return false;
	}

	int err;
	if (pa_simple_write(_audioHandle, _sampleBuffer, size * 2, &err) < 0) {
		setError(pa_strerror(err));
		return false;
	}

	return true;
}
