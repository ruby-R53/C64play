/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2024-2025 Enki Costa
 * Copyright 2011-2023 Leandro Nini
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

#include "codeConvert.h"

#include <cctype>
#include <cstring>
#include <cmath>

#include <iostream>
#include <iomanip>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::dec;
using std::hex;
using std::flush;
using std::setw;
using std::setfill;
using std::string;

#include <sidplayfp/SidInfo.h>
#include <sidplayfp/SidTuneInfo.h>

const char *noteName[] = {
	"---",
	"C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
	"C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
	"C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
	"C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
	"C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
	"C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
	"C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
	"C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
};

const char SID6581[] = "MOS6581";
const char SID8580[] = "CSG8580";

constexpr uint_least8_t tableWidth = 58;

const char info_file[] = "Writing audio file";
const char info_play[] = "Prev. [j] Pause [k] Next [l] Go to [g] Quit [q]";

const char* getModel(SidTuneInfo::model_t model) {
	switch (model) {
	default:
	case SidTuneInfo::SIDMODEL_UNKNOWN:
		return "Unknown";
	case SidTuneInfo::SIDMODEL_6581:
		return SID6581;
	case SidTuneInfo::SIDMODEL_8580:
		return SID8580;
	case SidTuneInfo::SIDMODEL_ANY:
		return "Any";
	}
}

const char* getModel(SidConfig::sid_model_t model) {
	switch (model) {
	default:
	case SidConfig::MOS6581:
		return SID6581;
	case SidConfig::MOS8580:
		return SID8580;
	}
}

int getModelInt(SidTuneInfo::model_t model) {
	switch (model) {
	default:
	case SidTuneInfo::SIDMODEL_UNKNOWN:
	case SidTuneInfo::SIDMODEL_ANY:
		return Chip::UNKNOWN_ANY;
	case SidTuneInfo::SIDMODEL_6581:
		return Chip::MOS6581;
	case SidTuneInfo::SIDMODEL_8580:
		return Chip::CSG8580;
	}
}

int getModelInt(SidConfig::sid_model_t model) {
	switch (model) {
	default:
	case SidConfig::MOS6581:
		return Chip::MOS6581;
	case SidConfig::MOS8580:
		return Chip::CSG8580;
	}
}

const char* getClock(SidTuneInfo::clock_t clock) {
	switch(clock) {
	default:
	case SidTuneInfo::CLOCK_UNKNOWN:
		return "Unknown";
	case SidTuneInfo::CLOCK_PAL:
		return "PAL";
	case SidTuneInfo::CLOCK_NTSC:
		return "NTSC";
	case SidTuneInfo::CLOCK_ANY:
		return "Any";
	}
}

const char* getCia(SidConfig::cia_model_t ciaModel) {
	switch(ciaModel) {
	default:
	case SidConfig::MOS6526:
		return "MOS6526";
	case SidConfig::MOS8521:
		return "MOS8521";
	}
}

string trimString(const char* str, unsigned int maxLen) {
	string data(str);

	// avoid too long file names
	if (data.length() > maxLen) {
		data.resize(maxLen - 3);
		data.append("...");
	}

	return data;
}

string ConsolePlayer::getNote(uint16_t freq) {
	if (freq) {
		uint16_t distance = 0xffff;
		string	 result("");
		for (uint8_t i = 0; i < 96; ++i) {
			uint16_t d = std::abs(freq - m_freqTable[i]);
			if (d < distance)
				distance = d;
			// check if frequency matches exactly or off by a bit
			// so that a ~ is displayed or not
			else if (d <= (m_freqTable[i] - m_freqTable[i-1])*(m_freqTable[i] / m_freqTable[i-1])) {
				result += noteName[i];
				return result;
			} else {
				result	= "~";
				result += noteName[i];
				return result;
			}
		}
		return noteName[96]; // 12 notes times 8 octaves
	}

	return noteName[0];
}

// Display console menu
void ConsolePlayer::menu() {
	std::ios_base::sync_with_stdio(false);

	if (m_quietLevel > 1) {
		if (m_driver.file)
			cerr << info_file;
		else
			cerr << ((m_quietLevel < 3) ? info_play : "Playing...");
		return;
	}

	const SidInfo	  &info		= m_engine.info ();
	const SidTuneInfo *tuneInfo = m_tune.getInfo();

	// New Page
	if (m_iniCfg.console().ansi) {
		cerr << "\x1b[2J";	 // Clear screen
		cerr << "\x1b[40m";  // black background
		cerr << "\x1b[0;0H"; // Move cursor to 0,0
		cerr << "\x1b[?25l"; // and hide it
	}

	consoleTable(tableStart);
	consoleTable(tableMiddle);
	consoleColor(white, true);

	string header;
	header.reserve(tableWidth);

	// the title first
	header.append("C64play - SID tune player");
	cerr << setw(tableWidth/2 + header.length()/2) << header << endl;
	consoleTable(tableMiddle);
	consoleColor(white, false);

	// then the description
	header.clear();
	header.append("Version " VERSION ", using ")
		  .append(info.name()).append(" v").append(info.version());

	cerr << setw(tableWidth/2 + header.length()/2) << header << endl;

	const unsigned int n = tuneInfo->numberOfInfoStrings();
	if (n) {
		codeConvert codeset;

		consoleTable(tableSeparator);
		consoleTable(tableMiddle);
		consoleColor(cyan, true);
		cerr << " Name         : ";
		consoleColor(white, false);
		cerr << codeset.convert(tuneInfo->infoString(0)) << endl;
		if (n > 1) {
			consoleTable(tableMiddle);
			consoleColor(cyan, true);
			cerr << " Artist(s)    : ";
			consoleColor(white, false);
			cerr << codeset.convert(tuneInfo->infoString(1)) << endl;
			consoleTable(tableMiddle);
			consoleColor(cyan, true);
			cerr << " Copyright    : ";
			consoleColor(white, false);
			cerr << codeset.convert(tuneInfo->infoString(2)) << endl;
		}
	}

	for (unsigned int i = 0; i < tuneInfo->numberOfCommentStrings(); ++i) {
		consoleTable(tableMiddle);
		consoleColor(cyan, true);
		cerr << " Comment      : ";
		consoleColor(magenta, false);
		cerr << tuneInfo->commentString(i) << endl;
	}

	consoleTable(tableSeparator);

	if (m_verboseLevel) {
		consoleTable(tableMiddle);
		consoleColor(green, true);
		cerr << " Format       : ";
		consoleColor(white, false);
		cerr << tuneInfo->formatString() << endl;
		consoleTable(tableMiddle);
		consoleColor(green, true);
		cerr << " File Name    : ";
		consoleColor(white, false);
		cerr << trimString(tuneInfo->dataFileName(), 41) << endl;

		// Second file is only sometimes present
		if (tuneInfo->infoFileName()) {
			consoleTable(tableMiddle);
			consoleColor(green, true);
			cerr << "              : ";
			consoleColor(white, false);
			cerr << tuneInfo->infoFileName() << endl;
		}

		consoleTable(tableMiddle);
		consoleColor(green, true);
		cerr << " Condition    : ";
		consoleColor(white, false);
		cerr << m_tune.statusString() << endl;
	}

	consoleTable(tableMiddle);
	consoleColor(green, true);

	if (tuneInfo->songs() == 1) {
		cerr << " On Loop?     : ";
		consoleColor(white, false);

		cerr << (m_track.loop ? "Yes" : "No");
	} else {
		cerr << " Subtune      : ";
		consoleColor(white, false);

		cerr << tuneInfo->currentSong() << '/' << tuneInfo->songs() << " "
			 << "(starting subtune: " << tuneInfo->startSong() << ")"
			 << (m_track.loop ? " - looping" : "");
	}

	cerr << endl;

	if (m_verboseLevel) {
		consoleTable(tableMiddle);
		consoleColor(green, true);
		cerr << " Video Signal : ";
		consoleColor(white, false);
		cerr << getClock(tuneInfo->clockSpeed()) << endl;
	}

	consoleTable(tableMiddle);
	consoleColor(green, true);
	cerr << " Duration     : ";
	consoleColor(white, false);
	if (m_timer.stop) {
		const uint_least32_t seconds = (m_timer.stop - m_fadeoutLen) / 1000;
		cerr << setw(2) << setfill('0') << ((seconds / 60) % 100)
			 << ':' << setw(2) << setfill('0') << (seconds % 60)
			 << '.' << setw(3) << m_timer.stop % 1000;

		if (m_fadeoutLen != 0)
			cerr << " (+" << (m_fadeoutLen / 1000) << "s fade out)";
	}
	else if (m_timer.valid)
		cerr << "Infinite";
	else if (!songlengthDB) {
		consoleColor(yellow, false);
		cerr << "Songlength DB not found!";
	} else {
		consoleColor(red, false);
		cerr << "Unknown";
	}

	if (m_timer.start) { // Show offset
		const uint_least32_t seconds = m_timer.start / 1000;
		cerr << " (+" << setw(2) << setfill('0') << ((seconds / 60) % 100)
			 << ':' << setw(2) << setfill('0') << (seconds % 60) << ")";
	}

	cerr << endl;

	if (m_verboseLevel) {
		consoleTable(tableSeparator);

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << " Subroutines  : ";
		consoleColor(white, false);

		// Display PSID Driver location
		cerr << hex;
		cerr.setf(std::ios::uppercase);
		cerr << "DRIVER: ";
		if (info.driverAddr() == 0)
			cerr << "Not present!";
		else
			cerr << "$"  << setw(4) << setfill('0') << info.driverAddr()
				 << "-$" << setw(4) << setfill('0')
				 << info.driverAddr() + (info.driverLength() - 1);

		if (tuneInfo->playAddr() == 0xffff)
			cerr << ", SYS: $" << setw(4) << setfill('0')
				 << tuneInfo->initAddr();
		else
			cerr << ", INIT: $" << setw(4) << setfill('0')
				 << tuneInfo->initAddr();
		cerr << endl;

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << "              : ";
		consoleColor(white, false);
		cerr << "LOAD  : $" << setw(4) << setfill('0')
			 << tuneInfo->loadAddr()
			 << "-$"		<< setw(4) << setfill('0')
			 << tuneInfo->loadAddr() + (tuneInfo->c64dataLen() - 1);

		if (tuneInfo->playAddr() != 0xffff)
			cerr << ", PLAY: $" << setw(4) << setfill('0')
				 << tuneInfo->playAddr();

		cerr << endl;

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << " SID Details  : ";
		consoleColor(white, false);

		for (int i = 0; i < tuneInfo->sidChips(); ++i) {
			if (i >= 1) {
				consoleTable(tableMiddle);
				consoleColor(yellow, true);
				cerr << "              : ";
				consoleColor(white, false);
			}
			
			if (tuneInfo->sidChips() > 1)
				cerr << "SID #" << i+1 << ": ";
				
			cerr << "$" << tuneInfo->sidChipBase(i)
				 << ", model: " << getModel(tuneInfo->sidModel(i))
				 << endl;
		}
		cerr << dec;
		cerr.unsetf(std::ios::uppercase);

		consoleTable(tableSeparator);

		if (m_verboseLevel > 1) {
			consoleTable(tableMiddle);
			consoleColor(yellow, true);
			cerr << " Delay        : ";
			consoleColor(white, false);
			cerr << info.powerOnDelay() << " cycles at power-on" << endl;
		}

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << " Timing       : ";
		consoleColor(white, false);
		cerr << info.speedString() << endl;

		if (m_verboseLevel > 1) {
			consoleTable(tableMiddle);
			consoleColor(yellow, true);
			cerr << " CIA Model    : ";
			consoleColor(white, false);
			cerr << getCia(m_engCfg.ciaModel) << endl;
		}

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << " Channels     : ";
		consoleColor(white, false);
		cerr << (info.channels() == 1 ? "Mono" : "Stereo") << endl;

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << " SID Model    : ";
		consoleColor(white, false);
		cerr << getModel(m_engCfg.defaultSidModel) << " "
			 << (m_engCfg.forceSidModel ? "(forced)" : "(default)")
			 << endl;

		consoleTable(tableMiddle);
		consoleColor(yellow, true);
		cerr << " Filter       : ";
		consoleColor(white, false);
		cerr << (m_filter.enabled ? "Enabled" : "Disabled") << endl;

		int tuneModel = getModelInt(tuneInfo->sidModel(0));
		int cfgModel  = getModelInt(m_engCfg.defaultSidModel);

		if (m_filter.enabled) {
			// check if filter curve is provided by the command line
			// or by the config file
			consoleTable(tableMiddle);
			consoleColor(yellow, true);
			cerr << " Filter Curve : ";
			consoleColor(white, false);

			if (!m_engCfg.forceSidModel && tuneModel != Chip::UNKNOWN_ANY) {
				switch(tuneModel) {
				default:
				case Chip::MOS6581:
					cerr << (m_fcurve.has_value() ? m_fcurve.value() : m_filter.filterCurve6581) << endl;
					break;
				case Chip::CSG8580:
					cerr << (m_fcurve.has_value() ? m_fcurve.value() : m_filter.filterCurve8580) << endl;
					break;
				}
			}

			if (tuneModel == Chip::UNKNOWN_ANY || m_engCfg.forceSidModel) {
				switch(cfgModel) {
				default:
				case Chip::MOS6581:
					cerr << (m_fcurve.has_value() ? m_fcurve.value() : m_filter.filterCurve6581) << endl;
					break;
				case Chip::CSG8580:
					cerr << (m_fcurve.has_value() ? m_fcurve.value() : m_filter.filterCurve8580) << endl;
					break;
				}
			}
		}

		if (!m_engCfg.forceSidModel && tuneModel != Chip::UNKNOWN_ANY) {
			switch(tuneModel) {
#ifdef FEAT_FILTER_RANGE
			// same with filter range here
			case Chip::MOS6581:
				if (m_filter.enabled) {
					consoleTable(tableMiddle);
					consoleColor(yellow, true);
					cerr << " Filter Range : ";
					consoleColor(white, false);
					cerr << (m_frange.has_value() ? m_frange.value() : m_filter.filterRange6581)
						 << endl;
				}
				break;
#endif
			case Chip::CSG8580:
				consoleTable(tableMiddle);
				consoleColor(yellow, true);
				cerr << " DigiBoost    : ";
				consoleColor(white, false);
				cerr << (m_engCfg.digiBoost ? "Enabled" : "Disabled")
					 << endl;
				break;
			}
		}

		if (tuneModel == Chip::UNKNOWN_ANY || m_engCfg.forceSidModel) {
			switch(cfgModel) {
#ifdef FEAT_FILTER_RANGE
			case Chip::MOS6581:
				if (m_filter.enabled) {
					consoleTable(tableMiddle);
					consoleColor(yellow, true);
					cerr << " Filter Range : ";
					consoleColor(white, false);
					cerr << (m_frange.has_value() ? m_frange.value() : m_filter.filterRange6581)
						 << endl;
				}
				break;
#endif
			case Chip::CSG8580:
				consoleTable(tableMiddle);
				consoleColor(yellow, true);
				cerr << " DigiBoost    : ";
				consoleColor(white, false);
				cerr << (m_engCfg.digiBoost ? "Enabled" : "Disabled")
					 << endl;
				break;
			}
		}
	}

	const char* romDesc = info.kernalDesc();

	consoleTable(tableSeparator);

	consoleTable(tableMiddle);
	consoleColor(magenta, true);
	cerr << " Kernal ROM   : ";
	if (std::strlen(romDesc) == 0) {
		consoleColor(red, false);
		cerr << "None - some tunes won't properly play!";
	} else {
		consoleColor(white, false);
		cerr << romDesc;
	}
	cerr << endl;

	romDesc = info.basicDesc();

	consoleTable(tableMiddle);
	consoleColor(magenta, true);
	cerr << " BASIC ROM    : ";
	if (std::strlen(romDesc) == 0) {
		consoleColor(red, false);
		cerr << "None - BASIC tunes won't play!";
	} else {
		consoleColor(white, false);
		cerr << romDesc;
	}
	cerr << endl;

	romDesc = info.chargenDesc();

	consoleTable(tableMiddle);
	consoleColor(magenta, true);
	cerr << " Chargen ROM  : ";
	if (std::strlen(romDesc) == 0) {
		consoleColor(yellow, false);
		cerr << "None";
	} else {
		consoleColor(white, false);
		cerr << romDesc;
	}
	cerr << endl;

	if (m_quietLevel >= 1) {
		consoleTable(tableEnd);
		cerr << info_play;
		return;
	}

	if (m_verboseLevel > 1) {
		consoleTable(tableSeparator);
		consoleTable(tableMiddle);
		const uint8_t movLines = (m_verboseLevel > 2) ?
#ifdef FEAT_NEW_PLAY_API
								 (m_engine.installedSIDs() * 6):
								 (m_engine.installedSIDs() * 3);
#else
								 (tuneInfo->sidChips() * 6):
								 (tuneInfo->sidChips() * 3);
#endif

		cerr << " Voice   Note   PW    Control Registers     Waveform(s)"
			 << '\n';

		for (int i = 0; i < movLines; ++i) {
			consoleTable(tableMiddle);
			cerr << '\n'; // reserve space for each voice's status
		}
	}

	consoleTable(tableEnd);

	if (m_driver.file)
		cerr << info_file << ": ";
	else
		cerr << info_play << " Time: ";

	cerr << flush;
}

void ConsolePlayer::refreshRegDump() {
	const SidTuneInfo* tuneInfo = m_tune.getInfo();
	const unsigned int movLines = (m_verboseLevel > 2) ?
#ifdef FEAT_NEW_PLAY_API
								  (m_engine.installedSIDs() * 6 + 1):
								  (m_engine.installedSIDs() * 3 + 1);
#else
								  (tuneInfo->sidChips() * 6 + 1):
								  (tuneInfo->sidChips() * 3 + 1);
#endif

	// Moves cursor for updating the displays
	cerr << "\x1b[" << movLines << "F";

	for (uint_least8_t j = 0; j < tuneInfo->sidChips(); ++j) {
		uint8_t* registers = m_registers[j];
		uint8_t  oldCtl[3] = { registers[0x04],
							   registers[0x0b],
							   registers[0x12] };

		if (m_engine.getSidStatus(j, registers)) {
			oldCtl[0] ^= registers[0x04];
			oldCtl[1] ^= registers[0x0b];
			oldCtl[2] ^= registers[0x12];

			for (int i = 0; i < 3; ++i) {
				consoleTable(tableMiddle);

				cerr << "   " << (j*3 + i+1) << "    ";

				consoleColor(cyan, false);

				// Note field
				cerr << setw(4) << setfill(' ')
					 << getNote(registers[0x00 + i * 0x07] |
							   (registers[0x01 + i * 0x07] << 8));

				// Pulse Width field
				cerr << hex << "   $" << setw(3) << setfill('0')
					 << (registers[0x02 + i * 0x07] |
						((registers[0x03 + i * 0x07] & 0x0f) << 8))

					 << "  ";

				// - control registers -
				// gate changed ?
				consoleColor((oldCtl[i] & 0x01) ? green : yellow, false);
				// gate on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x01) ? "GATE" : "gate")

					 << " ";

				// sync changed ?
				consoleColor((oldCtl[i] & 0x02) ? green : yellow, false);
				// sync on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x02) ? "SYNC" : "sync")

					 << " ";

				// ring changed ?
				consoleColor((oldCtl[i] & 0x04) ? green : yellow, false);
				// ring on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x04) ? "RING" : "ring")

					 << " ";

				// test changed ?
				consoleColor((oldCtl[i] & 0x08) ? green : yellow, false);
				// test on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x08) ? "TEST" : "test")

					 << "  ";

				// - waveform registers -
				// triangle changed ?
				consoleColor((oldCtl[i] & 0x10) ? green : blue, false);
				// triangle on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x10) ? "TRI" : "___")

					 << " ";

				// sawtooth changed ?
				consoleColor((oldCtl[i] & 0x20) ? green : blue, false);
				// sawtooth on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x20) ? "SAW" : "___")

					 << " ";

				// pulse changed ?
				consoleColor((oldCtl[i] & 0x40) ? green : blue, false);
				// pulse on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x40) ? "PUL" : "___")

					 << " ";

				// noise changed ?
				consoleColor((oldCtl[i] & 0x80) ? green : blue, false);
				// noise on ?
				cerr << ((registers[0x04 + i * 0x07] & 0x80) ? "NOI" : "___")

					 << '\n';
			}
		} else {
			for (int i = 0; i < 3; ++i) {
				consoleTable(tableMiddle);
				cerr << "???\n";
			}
		}
	}

	if (m_verboseLevel <= 2)
		consoleTable(tableEnd);

	else if (m_verboseLevel > 2) {
		for (int j = 0; j < tuneInfo->sidChips(); ++j) {
			uint8_t* registers = m_registers[j];
			string miscInfo;
			miscInfo.reserve(tableWidth);
			miscInfo.append("M. Vol.    Filters    F. Chn.  F. Res.  Cutoff");

			consoleTable(tableSeparator);
			consoleTable(tableMiddle);

			// number each SID chip if needed
			if (tuneInfo->sidChips() > 1) {
				cerr << " SID #" << (j + 1) << ":  " << miscInfo << '\n';
			} else
				cerr << setw(tableWidth/2 + miscInfo.length()/2) << miscInfo << '\n';

			consoleTable(tableMiddle);

			// master volume display
			consoleColor(yellow, false);
			cerr << hex
                 << ((tuneInfo->sidChips() >= 2) ? "            " : "        ")
				 << "$" << (registers[0x18] & 0x0f);

			// the filters!
			cerr << "     "
				 << ((registers[0x18] & 0x10) ? "LP" : "lp")
				 << " "
				 << ((registers[0x18] & 0x20) ? "BP" : "bp")
				 << " "
				 << ((registers[0x18] & 0x40) ? "HP" : "hp")
				 << " "
				 << ((registers[0x18] & 0x80) ? "3O" : "3o");

			// see which voices are being filtered
			cerr << "    "
				 << ((registers[0x17] & 0x01) ? "1" : "-")
				 << ((registers[0x17] & 0x02) ? "2" : "-")
				 << ((registers[0x17] & 0x04) ? "3" : "-");

			// filter resonance display
			cerr << "      "
				 << "$" << (registers[0x17] >> 4);

			// filter cutoff frequency display
			cerr << "      "
				 << "$" << setw(3) << setfill('0')
				 << (registers[0x16] << 4 | registers[0x15] & 0x07)
			
				 << '\n';
		}
		consoleTable(tableEnd);
	} else
		cerr << '\r';

	cerr << dec;

	if (m_driver.file)
		cerr << info_file << ": ";
	else
		cerr << info_play << " Time: ";

	cerr << flush;
}

// Set colour of text on console
void ConsolePlayer::consoleColor(color_t color, bool bold) {
	if (m_iniCfg.console().ansi) {
		const char *mode = "";

		switch (color) {
		case black:   mode = "0"; break;
		case red:	  mode = "1"; break;
		case green:   mode = "2"; break;
		case yellow:  mode = "3"; break;
		case blue:	  mode = "4"; break;
		case magenta: mode = "5"; break;
		case cyan:	  mode = "6"; break;
		case white:   mode = "7"; break;
		}

		const char* bold_c = (bold) ? "1" : "0";
		cerr << "\x1b[" << bold_c << ";40;3" << mode << 'm';
	}
}

// Display menu outline
void ConsolePlayer::consoleTable(table_t table) {
	consoleColor(white, true);
	switch (table) {
	case tableStart:
		cerr << m_iniCfg.console().topLeft << setw(tableWidth)
			 << setfill(m_iniCfg.console().horizontal) << ""
			 << m_iniCfg.console().topRight;
		break;

	case tableMiddle:
		cerr << setw(tableWidth + 1) << setfill(' ') << ""
			 << m_iniCfg.console().vertical << '\r'
			 << m_iniCfg.console().vertical;
		return;

	case tableSeparator:
		cerr << m_iniCfg.console().junctionRight << setw(tableWidth)
			 << setfill(m_iniCfg.console().horizontal) << ""
			 << m_iniCfg.console().junctionLeft;
		break;

	case tableEnd:
		cerr << m_iniCfg.console().bottomLeft << setw(tableWidth)
			 << setfill(m_iniCfg.console().horizontal) << ""
			 << m_iniCfg.console().bottomRight;
		break;
	}

	// Move back to begining of row and skip first char
	cerr << '\n';
}


// Restore ANSI console to defaults
void ConsolePlayer::consoleRestore() {
	if (m_iniCfg.console().ansi) {
		cerr << "\x1b[?25h"; // unhide cursor
		cerr << "\x1b[0m";	 // restore colors
	}
}
