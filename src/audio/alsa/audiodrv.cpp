/*
 * This file is part of sidplayfp, a console SID player.
 *
 * Copyright 2013-2024 Leandro Nini
 * Copyright 2000-2006 Simon White
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

#ifdef HAVE_ALSA

#include <new>

Audio_ALSA::Audio_ALSA() : AudioBase("ALSA") {
	// Reset everything.
	outOfOrder();
}

Audio_ALSA::~Audio_ALSA() {
	close();
}

void Audio_ALSA::outOfOrder() {
	// Reset everything.
	clearError();
	_audioHandle = nullptr;
}

void Audio_ALSA::checkResult(int err) {
	if (err < 0) {
		throw error(snd_strerror(err));
	}
}

bool Audio_ALSA::open(AudioConfig &cfg) {
	snd_pcm_hw_params_t *hw_params = nullptr;

	try {
		if (_audioHandle != nullptr) {
			throw error("Device already in use");
		}

		checkResult(snd_pcm_open(&_audioHandle, "default", SND_PCM_STREAM_PLAYBACK, 0));

		checkResult(snd_pcm_hw_params_malloc(&hw_params));

		checkResult(snd_pcm_hw_params_any(_audioHandle, hw_params));

		checkResult(snd_pcm_hw_params_set_access(_audioHandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));

		checkResult(snd_pcm_hw_params_set_format(_audioHandle, hw_params, SND_PCM_FORMAT_S16_LE));

		checkResult(snd_pcm_hw_params_set_channels(_audioHandle, hw_params, cfg.channels));

		checkResult(snd_pcm_hw_params_set_rate_near(_audioHandle, hw_params, &cfg.sampleRate, nullptr);

		snd_pcm_uframes_t buffer_size = cfg.sampleRate / 5;
		checkResult(snd_pcm_hw_params_set_buffer_size_near(_audioHandle, hw_params, &buffer_size));
		cfg.bufSize = buffer_size;

		snd_pcm_uframes_t period_size = buffer_size / 3;
		checkResult(snd_pcm_hw_params_set_period_size_near(_audioHandle, hw_params, &period_size, nullptr));

		checkResult(snd_pcm_hw_params(_audioHandle, hw_params));

		snd_pcm_hw_params_free(hw_params);
		hw_params = nullptr;

		try {
			_sampleBuffer = new short[snd_pcm_frames_to_bytes(_audioHandle, buffer_size)/2];
		}
		catch (std::bad_alloc const &ba) {
			throw error("Unable to allocate memory for sample buffers.");
		}

		// Setup internal Config
		_settings = cfg;
		return true;
	}
	catch(error const &e) {
		setError(e.message());

		if (hw_params)
			snd_pcm_hw_params_free(hw_params);
		if (_audioHandle != nullptr)
			close();

		return false;
	}
}

// Close an opened audio device, free any allocated buffers and
// reset any variables that reflect the current state.
void Audio_ALSA::close() {
	if (_audioHandle != nullptr) {
		snd_pcm_close(_audioHandle);
		delete[] _sampleBuffer;
		outOfOrder ();
	}
}

bool Audio_ALSA::write(uint_least32_t size) {
	if (_audioHandle == nullptr) {
		setError("Device not open.");
		return false;
	}

	snd_pcm_sframes_t err = snd_pcm_writei(_audioHandle, _sampleBuffer, size);
	if (err < 0) {
		err = snd_pcm_recover(_audioHandle, err, 0);
		if (err < 0) {
			setError(snd_strerror(err));
			return false;
		}
	}
	return true;
}

#endif // HAVE_ALSA
