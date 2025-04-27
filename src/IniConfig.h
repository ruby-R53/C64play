/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2011-2021 Leandro Nini
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

#ifndef INICONFIG_H
#define INICONFIG_H

#include "ini/types.h"
#include "ini/iniHandler.h"

#include "sidlib_features.h"

#include <sidplayfp/sidplayfp.h>
#include <sidplayfp/SidConfig.h>

/*
 * C64play's config file reader
 */
class IniConfig {
public:
	struct player_section {
		SID_STRING	   database;
		uint_least32_t playLength;
		uint_least32_t recordLength;
		SID_STRING	   kernalRom;
		SID_STRING	   basicRom;
		SID_STRING	   chargenRom;
		int			   verboseLevel;
		int			   quietLevel;
	};

	struct console_section { // [Console] section
		bool ansi;
		char topLeft;
		char topRight;
		char bottomLeft;
		char bottomRight;
		char vertical;
		char horizontal;
		char junctionLeft;
		char junctionRight;
	};

	struct audio_section { // [Audio] section
		int sampleRate; // in Hz
		int channels;
		int bitDepth;
	};

	struct emulation_section { // [Emulation] section
		SID_STRING					 engine;
		SidConfig::c64_model_t		 modelDefault;
		bool						 modelForced;
		SidConfig::sid_model_t		 sidModel;
		bool						 forceModel;
		SidConfig::cia_model_t		 ciaModel;
		bool						 digiboost;
		bool						 filter;
		double						 bias;
		double						 filterCurve6581;

#ifdef FEAT_FILTER_RANGE
		double						 filterRange6581;
#endif

		double						 filterCurve8580;

#ifdef FEAT_CW_STRENGTH
		SidConfig::sid_cw_t			 combinedWaveformsStrength;
#endif

		int							 powerOnDelay;
		SidConfig::sampling_method_t samplingMethod;
		bool						 fastSampling;
	};

protected:
	struct player_section	 player_s;
	struct console_section	 console_s;
	struct audio_section	 audio_s;
	struct emulation_section emulation_s;

protected:
	void clear();

	void readPlayer   (iniHandler &ini);
	void readConsole  (iniHandler &ini);
	void readAudio	  (iniHandler &ini);
	void readEmulation(iniHandler &ini);

private:
	SID_STRING m_fileName;

public:
	IniConfig ();
	~IniConfig();

	SID_STRING getFilename() const { return m_fileName; }

	void read();

	// Configuration sections for C64play
	const player_section&	 playercfg() { return player_s;    }
	const console_section&	 console  () { return console_s;   }
	const audio_section&	 audio	  () { return audio_s;	   }
	const emulation_section& emulation() { return emulation_s; }
};

#endif // INICONFIG_H
