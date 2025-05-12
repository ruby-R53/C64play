/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2024-2025 Enki Costa
 * Copyright 2011-2024 Leandro Nini
 * Copyright 2000-2001 Simon White
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

#include "player.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <new>

using std::cerr;
using std::endl;

#include <unistd.h>

#include "utils.h"
#include "keyboard.h"
#include "audio/AudioDrv.h"
#include "audio/wav/WavFile.h"
#include "ini/types.h"

#include "sidcxx.h"

#include <sidplayfp/sidbuilder.h>
#include <sidplayfp/SidInfo.h>
#include <sidplayfp/SidTuneInfo.h>

#include <unordered_map>

using filter_map_t      = std::unordered_map<std::string, double>;
using filter_map_iter_t = std::unordered_map<std::string, double>::const_iterator;

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
#  include <sidplayfp/builders/residfp.h>
const char ConsolePlayer::RESIDFP_ID[] = "ReSIDfp";
#endif

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
#  include <sidplayfp/builders/resid.h>
const char ConsolePlayer::RESID_ID[] = "ReSID";
#endif

// tables used for displaying the register dump
uint16_t freqTablePal[] = {
	// C	   C#	   D	   D#	   E	   F	   F#	   G	   G#	   A	   A#	   B
	0x0117, 0x0127, 0x0139, 0x014b, 0x015f, 0x0174, 0x018a, 0x01a1, 0x01ba, 0x01d4, 0x01f0, 0x020e, // 1
	0x022d, 0x024e, 0x0271, 0x0296, 0x02be, 0x02e8, 0x0314, 0x0343, 0x0374, 0x03a9, 0x03e1, 0x041c, // 2
	0x045a, 0x049c, 0x04e2, 0x052d, 0x057c, 0x05cf, 0x0628, 0x0685, 0x06e8, 0x0752, 0x07c1, 0x0837, // 3
	0x08b4, 0x0939, 0x09c5, 0x0a5a, 0x0af7, 0x0b9e, 0x0c4f, 0x0d0a, 0x0dd1, 0x0ea3, 0x0f82, 0x106e, // 4
	0x1168, 0x1271, 0x138a, 0x14b3, 0x15ee, 0x173c, 0x189e, 0x1a15, 0x1ba2, 0x1d46, 0x1f04, 0x20dc, // 5
	0x22d0, 0x24e2, 0x2714, 0x2967, 0x2bdd, 0x2e79, 0x313c, 0x3429, 0x3744, 0x3a8d, 0x3e08, 0x41b8, // 6
	0x45a1, 0x49c5, 0x4e28, 0x52cd, 0x57ba, 0x5cf1, 0x6278, 0x6853, 0x6e87, 0x751a, 0x7c10, 0x8371, // 7
	0x8b42, 0x9389, 0x9c4f, 0xa59b, 0xaf74, 0xb9e2, 0xc4f0, 0xd0a6, 0xdd0e, 0xea33, 0xf820, 0xffff, // 8
};

uint16_t freqTableNtsc[] = {
	// C	   C#	   D	   D#	   E	   F	   F#	   G	   G#	   A	   A#	   B
	0x010c, 0x011c, 0x012d, 0x013f, 0x0152, 0x0166, 0x017b, 0x0192, 0x01aa, 0x01c3, 0x01de, 0x01fa, // 1
	0x0218, 0x0238, 0x025a, 0x027e, 0x02a4, 0x02cc, 0x02f7, 0x0324, 0x0354, 0x0386, 0x03bc, 0x03f5, // 2
	0x0431, 0x0471, 0x04b5, 0x04fc, 0x0548, 0x0598, 0x05ee, 0x0648, 0x06a9, 0x070d, 0x0779, 0x07ea, // 3
	0x0862, 0x08e2, 0x096a, 0x09f8, 0x0a90, 0x0b30, 0x0bdc, 0x0c90, 0x0d52, 0x0e1a, 0x0ef2, 0x0fd4, // 4
	0x10c4, 0x11c4, 0x12d4, 0x13f0, 0x1520, 0x1660, 0x17b8, 0x1920, 0x1aa4, 0x1c34, 0x1de4, 0x1fa8, // 5
	0x2188, 0x2388, 0x25a8, 0x27e0, 0x2a40, 0x2cc0, 0x2f70, 0x3240, 0x3548, 0x3868, 0x3bc8, 0x3f50, // 6
	0x4310, 0x4710, 0x4b50, 0x4fc0, 0x5480, 0x5980, 0x5ee0, 0x6480, 0x6a90, 0x70d0, 0x7790, 0x7ea0, // 7
	0x8620, 0x8e20, 0x96a0, 0x9f80, 0xa900, 0xb300, 0xbdc0, 0xc900, 0xd520, 0xe1a0, 0xef20, 0xfd40, // 8
};

uint8_t* loadRom(const SID_STRING &romPath, const int size) {
	SID_IFSTREAM is(romPath.c_str(), std::ios::binary);

	if (is.is_open()) {
		try {
			uint8_t *buffer = new uint8_t[size];

			is.read((char*)buffer, size);
			if (!is.fail()) {
				is.close();

				return buffer;
			}

			delete [] buffer;
		}
		catch (std::bad_alloc const &ba) {}
	}

	return nullptr;
}


uint8_t* loadRom(const SID_STRING &romPath, const int size, const TCHAR defaultRom[]) {
	// Try to load given ROM
	if (!romPath.empty()) {
		uint8_t* buffer = loadRom(romPath, size);
		if (buffer)
			return buffer;
	}

	// Fallback to default ROM path
	try {
		SID_STRING dataPath(utils::getDataPath());

		dataPath.append(SEPARATOR).append("C64play")
				.append(SEPARATOR).append(defaultRom);

		if (::access(dataPath.c_str(), R_OK) != 0) {
			dataPath = PKGDATADIR;
			dataPath.append(defaultRom);
		}

		return loadRom(dataPath, size);
	}
	catch (utils::error const &e) {
		return nullptr;
	}
}


ConsolePlayer::ConsolePlayer(const char * const name) :
	m_name(name),
	m_tune(nullptr),
	m_state(playerStopped),
	m_outfile(nullptr),
	m_filename(""),

	songlengthDB(false),
	m_cpudebug(false)

{
	std::memset(m_registers, 0, 32*3);

	// Other defaults
	m_filter.enabled = true;
	m_driver.device  = nullptr;
	m_driver.sid	 = EMU_RESIDFP;
	m_timer.start	 = 0;
	m_timer.length	 = 0; // infinite play time by default
	m_timer.valid	 = false;
	m_timer.starting = false;
	m_track.first	 = 0;
	m_track.selected = 0;
	m_track.loop	 = false;
	m_track.single	 = false;
	m_speed.current  = 1;
	m_speed.max		 = 32;

	// Read default configuration
	m_iniCfg.read();
	m_engCfg = m_engine.config();

	{	// Load ini settings
		IniConfig::audio_section	 audio	   = m_iniCfg.audio();
		IniConfig::emulation_section emulation = m_iniCfg.emulation();

		// INI Configuration Settings
		m_engCfg.forceC64Model   = emulation.modelForced;
		m_engCfg.defaultC64Model = emulation.modelDefault;
		m_engCfg.defaultSidModel = emulation.sidModel;
		m_engCfg.forceSidModel   = emulation.forceModel;
		m_engCfg.ciaModel        = emulation.ciaModel;
		m_engCfg.frequency       = audio.sampleRate;
		m_engCfg.samplingMethod  = emulation.samplingMethod;
		m_engCfg.fastSampling    = emulation.fastSampling;
		m_channels               = audio.channels;
		m_bitDepth               = audio.bitDepth;
		m_filter.enabled         = emulation.filter;
		m_filter.bias            = emulation.bias;
		m_filter.filterCurve6581 = emulation.filterCurve6581;

#ifdef FEAT_FILTER_RANGE
		m_filter.filterRange6581 = emulation.filterRange6581;
#endif

		m_filter.filterCurve8580 = emulation.filterCurve8580;

#ifdef FEAT_CW_STRENGTH
		m_combinedWaveformsStrength = emulation.combinedWaveformsStrength;
#endif

		if (emulation.powerOnDelay >= 0)
			m_engCfg.powerOnDelay = emulation.powerOnDelay;

		if (!emulation.engine.empty()) {
			if (emulation.engine.compare("RESIDFP") == 0) {
				m_driver.sid = EMU_RESIDFP;
			}
			else if (emulation.engine.compare("RESID") == 0) {
				m_driver.sid = EMU_RESID;
			}
			else if (emulation.engine.compare("NONE") == 0) {
				m_driver.sid = EMU_NONE;
			}
		}
	}

	m_verboseLevel = m_iniCfg.playercfg().verboseLevel;
	m_quietLevel   = m_iniCfg.playercfg().quietLevel;

	createOutput(OUT_NULL, nullptr);
	createSidEmu(EMU_NONE, nullptr);

	uint8_t *kernalRom	= loadRom(m_iniCfg.playercfg().kernalRom, 8192, "kernal");
	uint8_t *basicRom	= loadRom(m_iniCfg.playercfg().basicRom, 8192, "basic");
	uint8_t *chargenRom = loadRom(m_iniCfg.playercfg().chargenRom, 4096, "chargen");
	m_engine.setRoms(kernalRom, basicRom, chargenRom);

	delete [] kernalRom;
	delete [] basicRom;
	delete [] chargenRom;
}

std::string ConsolePlayer::getFileName(const SidTuneInfo *tuneInfo, const char* ext) {
	std::string title;

	if (m_outfile != nullptr) {
		title = m_outfile;
		if (title.compare("-") != 0 &&
			title.find_last_of('.') == std::string::npos)
			title.append(ext);
	} else {
		// Generate a name for the wav file
		title = tuneInfo->dataFileName();

		title.erase(title.find_last_of('.'));

		// Change name based on subtune
		if (tuneInfo->songs() > 1) {
			std::ostringstream sstream;
			sstream << "[" << tuneInfo->currentSong() << "]";
			title.append(sstream.str());
		}

		title.append(ext);
	}

	return title;
}

// Create the output object to process sound buffer
bool ConsolePlayer::createOutput(OUTPUTS driver, const SidTuneInfo *tuneInfo) {
	// Remove old audio driver
	m_driver.null.close();
	m_driver.selected = &m_driver.null;

	if (m_driver.device != nullptr) {
		if (m_driver.device != &m_driver.null)
			delete m_driver.device;

		m_driver.device = nullptr;
	}

	// Create audio driver
	switch (driver) {
	case OUT_NULL:
		m_driver.device = &m_driver.null;
	break;

	case OUT_SOUNDCARD:
		try {
			m_driver.device = new audioDrv();
		}
		catch (std::bad_alloc const &ba) {
			m_driver.device = nullptr;
		}
	break;

	case OUT_WAV:
		try {
			std::string title = getFileName(tuneInfo, WavFile::extension());
			WavFile* wav = new WavFile(title);
			if (m_driver.info && (tuneInfo->numberOfInfoStrings() == 3))
				wav->setInfo(tuneInfo->infoString(0), tuneInfo->infoString(1),
							 tuneInfo->infoString(2));
			m_driver.device = wav;
		}
		catch (std::bad_alloc const &ba) {
			m_driver.device = nullptr;
		}
	break;

	default:
		break;
	}

	// Audio driver failed
	if (!m_driver.device) {
		m_driver.device = &m_driver.null;
		displayError("ERROR: not enough memory!");

		return false;
	}

	uint_least8_t tuneChannels = (tuneInfo && (tuneInfo->sidChips() > 1)) ? 2 : 1;

	// Configure with user settings
	m_driver.cfg.sampleRate = m_engCfg.frequency;
	m_driver.cfg.channels	= m_channels ? m_channels : tuneChannels;
	m_driver.cfg.depth		= m_bitDepth;
	m_driver.cfg.bufSize	= 0; // Recalculate

	{	// Open the hardware
		bool err = false;
		if (!m_driver.device->open(m_driver.cfg))
			err = true;

		// Can't open the same driver twice
		if (driver != OUT_NULL) {
			if (!m_driver.null.open(m_driver.cfg))
				err = true;
		}

		if (err) {
			displayError(m_driver.device->getErrorString());

			return false;
		}
	}

	// See what we got
	m_engCfg.frequency = m_driver.cfg.sampleRate;
	switch (m_driver.cfg.channels) {
	case 1:
		m_engCfg.playback = SidConfig::MONO;
		break;

	case 2:
		m_engCfg.playback = SidConfig::STEREO;
		break;

	default:
		cerr << "[" << m_name << "]" << " ERROR: " << m_channels
			 << " audio channels not supported" << endl;

		return false;
	}

	return true;
}


// Create SID emulation
bool ConsolePlayer::createSidEmu(SIDEMUS emu, const SidTuneInfo *tuneInfo) {
	// Remove old driver and emulation
	if (m_engCfg.sidEmulation) {
		sidbuilder *builder   = m_engCfg.sidEmulation;
		m_engCfg.sidEmulation = nullptr;
		m_engine.config(m_engCfg);
		delete builder;
	}

	// Now set it up
	switch (emu) {
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
	case EMU_RESIDFP:
	{
		try {
			ReSIDfpBuilder *rs = new ReSIDfpBuilder(RESIDFP_ID);

			m_engCfg.sidEmulation = rs;
			if (!rs->getStatus()) goto createSidEmu_error;
			rs->create(m_engine.info().maxsids());
			if (!rs->getStatus()) goto createSidEmu_error;

#ifdef FEAT_CW_STRENGTH
			rs->combinedWaveformsStrength(m_combinedWaveformsStrength);
#endif

			bool is6581 = (
				(m_engCfg.defaultSidModel == SidConfig::MOS6581)
			    && (m_engCfg.forceSidModel ||
				   (tuneInfo->sidModel(0) != SidTuneInfo::SIDMODEL_8580))
				|| (tuneInfo->sidModel(0) == SidTuneInfo::SIDMODEL_6581)
			);

#ifdef FEAT_FILTER_RANGE
			// 6581 filter range control
			if (is6581) {
				if (m_frange.has_value()) {
					m_filter.filterRange6581 = m_frange.value();
				}

				if ((m_filter.filterRange6581 < 0.0)
				    || (m_filter.filterRange6581 > 1.0)) {
					cerr << "Invalid 6581 filter range: "
					     << m_filter.filterRange6581 << endl;

					exit(EXIT_FAILURE);
				}

				rs->filter6581Range(m_filter.filterRange6581);
#endif

				// 6581 filter curve control
				if (m_fcurve.has_value()) {
					m_filter.filterCurve6581 = m_fcurve.value();
				}

				if ((m_filter.filterCurve6581 < -2.0)
				    || (m_filter.filterCurve6581 > 2.0)) {
					cerr << "Invalid 6581 filter curve: "
					     << m_filter.filterCurve6581 << endl;

					exit(EXIT_FAILURE);
				}

				rs->filter6581Curve(m_filter.filterCurve6581);
			} else {
				// 8580 filter curve control
				if (m_fcurve.has_value()) {
					m_filter.filterCurve8580 = m_fcurve.value();
				}

				if ((m_filter.filterCurve8580 < 0.0)
				    || (m_filter.filterCurve8580 > 1.0)) {
					cerr << "Invalid 8580 filter curve: "
					     << m_filter.filterCurve8580 << endl;

					exit(EXIT_FAILURE);
				}

				rs->filter8580Curve(m_filter.filterCurve8580);
			}
		}

		catch (std::bad_alloc const &ba) {}
		break;
	}
#endif // HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
	case EMU_RESID:
	{
		try {
			ReSIDBuilder *rs = new ReSIDBuilder(RESID_ID);

			m_engCfg.sidEmulation = rs;
			if (!rs->getStatus()) goto createSidEmu_error;
			rs->create(m_engine.info().maxsids());
			if (!rs->getStatus()) goto createSidEmu_error;

			rs->bias(m_filter.bias);
		}
		catch (std::bad_alloc const &ba) {}
		break;
	}
#endif // HAVE_SIDPLAYFP_BUILDERS_RESID_H

	default:
		// Emulation not yet handled
		// This default case results in the default
		// emulation
		break;
	}

	if (!m_engCfg.sidEmulation) {
		if (emu > EMU_DEFAULT) { // No SID emulation?
			displayError("ERROR: not enough memory!");
			return false;
		}
	}

#ifndef FEAT_FILTER_DISABLE
	if (m_engCfg.sidEmulation) {
		// set up SID filter
		m_engCfg.sidEmulation->filter(m_filter.enabled);
	}
#endif

	return true;

createSidEmu_error:
	displayError(m_engCfg.sidEmulation->error());
	delete m_engCfg.sidEmulation;
	m_engCfg.sidEmulation = nullptr;

	return false;
}

bool ConsolePlayer::open(void) {
	if ((m_state & ~playerFast) == playerRestart) {
		if (m_quietLevel < 2)
			cerr << '\n';

		if (m_state & playerFast)
			m_driver.selected->reset();

		m_state = playerStopped;
	}

	// Select the required song
	m_track.selected = m_tune.selectSong(m_track.selected);
	if (!m_engine.load(&m_tune)) {
		displayError(m_engine.error());

		return false;
	}

	// Get tune details
	const SidTuneInfo *tuneInfo = m_tune.getInfo();
	if (!m_track.single)
		m_track.songs = tuneInfo->songs();

	if (!createOutput(m_driver.output, tuneInfo))
		return false;

	if (!createSidEmu(m_driver.sid, tuneInfo))
		return false;

	// Configure engine with settings
	if (!m_engine.config(m_engCfg)) { // Config failed?
		displayError(m_engine.error());

		return false;
	}

#ifdef FEAT_FILTER_DISABLE
	m_engine.filter(0, m_filter.enabled);
	m_engine.filter(1, m_filter.enabled);
	m_engine.filter(2, m_filter.enabled);
#endif

	m_freqTable = (
		(
			(m_engCfg.defaultC64Model == SidConfig::NTSC) &&
		    (m_engCfg.forceC64Model ||
				(tuneInfo->clockSpeed() != SidTuneInfo::CLOCK_PAL)
			)
		) || (tuneInfo->clockSpeed() == SidTuneInfo::CLOCK_NTSC)
	) ? freqTableNtsc : freqTablePal;

#ifdef FEAT_NEW_PLAY_API
	m_mixer.initialize(m_engine.installedSIDs(), m_engCfg.playback == SidConfig::STEREO);
#endif

	// Start the player. Do this by fast
	// forwarding to the start position
	m_driver.selected = &m_driver.null;
#ifdef FEAT_NEW_PLAY_API
	m_mixer.setFastForward(m_speed.current);
#else
	m_engine.fastForward(100 * m_speed.current);
#endif

	m_engine.mute(0, 0, m_mute_channel[0]);
	m_engine.mute(0, 1, m_mute_channel[1]);
	m_engine.mute(0, 2, m_mute_channel[2]);
	m_engine.mute(1, 0, m_mute_channel[3]);
	m_engine.mute(1, 1, m_mute_channel[4]);
	m_engine.mute(1, 2, m_mute_channel[5]);
	m_engine.mute(2, 0, m_mute_channel[6]);
	m_engine.mute(2, 1, m_mute_channel[7]);
	m_engine.mute(2, 2, m_mute_channel[8]);

#ifdef FEAT_SAMPLE_MUTE
	m_engine.mute(0, 3, m_mute_samples[0]);
	m_engine.mute(1, 3, m_mute_samples[1]);
	m_engine.mute(2, 3, m_mute_samples[2]);
#endif

	// As yet we don't have a required songlength
	// so try the songlength database or keep the default
	if (!m_timer.valid) {
		const int_least32_t length = m_database.lengthMs(m_tune);

		if (length > 0)
			m_timer.length = length;
	}

	// Set up the play timer
	m_timer.stop = m_timer.length;

	if (m_timer.valid) { // Length relative to start
		if (m_timer.stop > 0)
			m_timer.stop += m_timer.start;
	} else { // Make sure start time dosen't exceed end time
		if ((m_timer.stop != 0) && (m_timer.start >= m_timer.stop)) {
			displayError("ERROR: start time exceeds the song's duration!");
			return false;
		}
	}

	m_timer.current  = ~0;
	m_timer.starting = true;
	m_state = playerRunning;

	// Update display
	menu();
	updateDisplay();

	return true;
}

void ConsolePlayer::close() {
	m_engine.stop();
	if (m_state == playerExit) { // Natural finish
		if (m_driver.file)
			cerr << (char) 7; // Ring bell when done
	} else // Destroy buffers
		m_driver.selected->reset();

	// Shutdown drivers, etc
	createOutput   (OUT_NULL, nullptr);
	createSidEmu   (EMU_NONE, nullptr);
	m_engine.load  (nullptr);
	m_engine.config(m_engCfg);

	if (m_quietLevel < 2) {
		// Correctly leave ANSI mode and get prompt to
		// end up in a suitable location
		if (m_iniCfg.console().ansi) {
			cerr << "\x1b[?25h";
			cerr << "\x1b[0m";
		}

		cerr << endl;
	}
}

// Out play loop to be externally called
bool ConsolePlayer::play() {
	// prepare for playback
	uint_least32_t retSize = 0;

	if (m_state == playerRunning) LIKELY {
		updateDisplay();

		const uint_least32_t length = getBufSize();
		short *buffer = m_driver.selected->buffer(); // Fill buffer

#ifdef FEAT_NEW_PLAY_API
		m_mixer.begin(buffer, length);
		short* buffers[3];
		m_engine.buffers(buffers);

		do {
			int samples = m_engine.play(2000);
			
			if (samples < 0) UNLIKELY {
				cerr << m_engine.error();
				m_state = playerError;
				return false;
			}

			m_mixer.doMix(buffers, samples);
		} while (!m_mixer.isFull());

		retSize = length;
#else
		retSize = m_engine.play(buffer, length);

		if ((retSize < length) || !m_engine.isPlaying()) UNLIKELY {
			cerr << m_engine.error();
			m_state = playerError;

			return false;
		}
#endif
	}
	else if (m_state == playerPaused)
		usleep(100000);

	switch (m_state) {
	LIKELY case playerRunning:
		if (!m_driver.selected->write(retSize)) UNLIKELY {
			cerr << m_driver.selected->getErrorString();
			m_state = playerError;

			return false;
		}

	case playerPaused: // fall-through
		// Check for a keypress (rate depends on buffer size).
		// Don't do this for high quiet levels as chances are
		// we are under remote control.
		if ((m_quietLevel < 3) && _kbhit())
			decodeKeys();

		return true;

	default:
		if (m_quietLevel < 3)
			cerr << '\n';

		m_engine.stop();
		break;
	}

	return false;
}


void ConsolePlayer::stop() {
	m_state = playerStopped;
	m_engine.stop();
}


uint_least32_t ConsolePlayer::getBufSize() {
	// get audio configuration
	const uint_least32_t bytesPerMillis =
		(m_driver.cfg.depth / 8 *
		 m_driver.cfg.channels *
		 m_driver.cfg.sampleRate) / 1000;

	// Switch audio drivers.
	if (m_timer.starting && (m_timer.current >= m_timer.start)) UNLIKELY {
		m_timer.starting  = false;
		m_driver.selected = m_driver.device;
		memset(m_driver.selected->buffer(), 0, m_driver.cfg.bufSize);
#ifdef FEAT_NEW_PLAY_API
		m_mixer.clear();
		m_mixer.setFastForward(1);
#else
		m_engine.fastForward(100);
#endif
		m_speed.current = 1;

		if (m_cpudebug)
			m_engine.debug(true, nullptr);
	}
	else if ((m_timer.stop != 0) && (m_timer.current >= m_timer.stop)) UNLIKELY {
		m_state = playerExit;

		if (m_track.loop) { m_state = playerRestart; }
		else if (m_track.single) { return 0; }

		// Move to next track
		++m_track.selected;
		
		if (m_track.selected > m_track.songs)
			m_track.selected = 1;
		
		if (m_track.selected == m_track.first)
			return 0;
		
		m_state = playerRestart;
	} else {
		uint_least32_t remaining = m_timer.stop - m_timer.current;
		uint_least32_t bufSize	 = remaining * bytesPerMillis;

		if (bufSize < m_driver.cfg.bufSize)
			return bufSize;
	}

	return m_driver.cfg.bufSize;
}


void ConsolePlayer::updateDisplay() {
	const uint_least32_t milliseconds = m_engine.timeMs();
	const uint_least32_t seconds = milliseconds / 1000;

	if (!m_quietLevel) {
		if (m_verboseLevel > 1)
			refreshRegDump();

		if (seconds != (m_timer.current / 1000)) {
			cerr << std::setw(2) << std::setfill('0')
				 << ((seconds / 60) % 100) << ':' << std::setw(2)
				 << std::setfill('0') << (seconds % 60) << std::flush;

			// this hack has to be done because for some
			// reason at both level 1 and 0 it appends to
			// the timer instead of overwriting it
			if (m_verboseLevel <= 1)
				cerr << "\b\b\b\b\b";
		}
	}

	m_timer.current = milliseconds;
}

void ConsolePlayer::displayError(const char *error) {
	cerr << m_name << ": " << error << endl;
}

// Keyboard handling
void ConsolePlayer::decodeKeys() {
	while (_kbhit()) {
		const int action = keyboard_decode();
		if (action == A_INVALID)
			continue;

		switch (action) {
		case A_RIGHT_ARROW:
			m_state = playerFastRestart;
			if (!m_track.single) {
				++m_track.selected;
				if (m_track.selected > m_track.songs)
					m_track.selected = 1; // wrap around if reaching the end
			}
		break;

		case A_LEFT_ARROW:
			m_state = playerFastRestart;
			if (!m_track.single) {
				--m_track.selected;
				if (m_track.selected < 1) // same thing different direction
					m_track.selected = m_track.songs;
			}
		break;

		case A_REPLAY:
			m_state = playerFastRestart;
		break;

		case A_UP_ARROW:
		case A_INCREASE:
			m_speed.current *= 2;

			if (m_speed.current > m_speed.max)
				m_speed.current = m_speed.max;

#ifdef FEAT_NEW_PLAY_API
			m_mixer.setFastForward(m_speed.current);
#else
			m_engine.fastForward(100 * m_speed.current);
#endif
		break;

		case A_DOWN_ARROW:
		case A_DECREASE:
			if (m_speed.current > 1)
				m_speed.current /= 2;

#ifdef FEAT_NEW_PLAY_API
			m_mixer.setFastForward(m_speed.current);
#else
			m_engine.fastForward(100 * m_speed.current);
#endif
		break;

		case A_RESTORE:
			m_speed.current = 1;
#ifdef FEAT_NEW_PLAY_API
			m_mixer.setFastForward(1);
#else
			m_engine.fastForward(100);
#endif
		break;

		case A_HOME:
			m_state = playerFastRestart;
			m_track.selected = 1;
		break;

		case A_END:
			m_state = playerFastRestart;
			m_track.selected = m_track.songs;
		break;

		case A_PAUSE:
			if (m_state == playerPaused) {
				cerr << "\b\b\b\b\b\b\b\b";
				// wipe every character out here
				cerr << "        ";
				cerr << "\b\b\b\b\b\b\b\b";
				m_state = playerRunning;
			} else {
				cerr << "(paused)";
				m_state = playerPaused;
				m_driver.selected->pause();
			}
		break;

		case A_GOTO:
			if (m_track.single || m_track.songs == 1)
				break;

			m_state = playerStopped;
			cerr << "\x1b[2K\r";
			cerr << "Jumping to subtune: ";
			keyboard_disable_raw();
			std::cin >> m_track.selected;
			keyboard_enable_raw();
			cerr << "\x1b[2K\r" << "\x1b[1A";
			if (m_track.selected <= m_track.songs) {
				m_state = playerFastRestart;
			} else {
				cerr << "Subtune #" << m_track.selected << " not found!";
				sleep(1);
			}
		break;

		case A_TOGGLE_VOICE1:
			m_mute_channel.flip(0);
			m_engine.mute(0, 0, m_mute_channel[0]);
		break;

		case A_TOGGLE_VOICE2:
			m_mute_channel.flip(1);
			m_engine.mute(0, 1, m_mute_channel[1]);
		break;

		case A_TOGGLE_VOICE3:
			m_mute_channel.flip(2);
			m_engine.mute(0, 2, m_mute_channel[2]);
		break;

		case A_TOGGLE_VOICE4:
			m_mute_channel.flip(3);
			m_engine.mute(1, 0, m_mute_channel[3]);
		break;

		case A_TOGGLE_VOICE5:
			m_mute_channel.flip(4);
			m_engine.mute(1, 1, m_mute_channel[4]);
		break;

		case A_TOGGLE_VOICE6:
			m_mute_channel.flip(5);
			m_engine.mute(1, 2, m_mute_channel[5]);
		break;

		case A_TOGGLE_VOICE7:
			m_mute_channel.flip(6);
			m_engine.mute(2, 0, m_mute_channel[6]);
		break;

		case A_TOGGLE_VOICE8:
			m_mute_channel.flip(7);
			m_engine.mute(2, 1, m_mute_channel[7]);
		break;

		case A_TOGGLE_VOICE9:
			m_mute_channel.flip(8);
			m_engine.mute(2, 2, m_mute_channel[8]);
		break;

#ifdef FEAT_SAMPLE_MUTE
		case A_TOGGLE_SAMPLE1:
			m_mute_samples.flip(0);
			m_engine.mute(0, 3, m_mute_samples[0]);
		break;

		case A_TOGGLE_SAMPLE2:
			m_mute_samples.flip(1);
			m_engine.mute(1, 3, m_mute_samples[1]);
		break;

		case A_TOGGLE_SAMPLE3:
			m_mute_samples.flip(2);
			m_engine.mute(2, 3, m_mute_samples[2]);
		break;
#endif

		case A_TOGGLE_FILTER:
			m_filter.enabled = !m_filter.enabled;

#ifdef FEAT_FILTER_DISABLE
			m_engine.filter(0, m_filter.enabled);
			m_engine.filter(1, m_filter.enabled);
			m_engine.filter(2, m_filter.enabled);
#else
			m_engCfg.sidEmulation->filter(m_filter.enabled);
#endif
		break;

		case A_QUIT:
			m_state = playerFastExit;
			return;
		break;
		}
	}
}
