/*
 * This file is part of C64play, a console player for SID tunes.
 *
 * Copyright 2024 Erika Lima
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

#ifdef FEAT_REGS_DUMP_SID
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
#endif

const char SID6581[] = "MOS6581";
const char SID8580[] = "CSG8580";

const unsigned char tableWidth = 58;

const char info_file[] = "Writing audio file";
const char info_play[] = "Prev. [j] Pause [k] Next [l] Quit [q] Go to [g]";

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

string trimString(const char* str, unsigned int maxLen) {
    string data(str);
    // avoid too long file names
    if (data.length() > maxLen) {
        data.resize(maxLen - 3);
        data.append("...");
    }
    return data;
}

#ifdef FEAT_REGS_DUMP_SID
string ConsolePlayer::getNote(uint16_t freq) {
    if (freq) {
        uint16_t distance = 0xffff;
		string   result("");
        for (int i = 0; i < 96; ++i) {
            uint16_t d = abs(freq - m_freqTable[i]);
            if (d < distance)
                distance = d;
			else if (d <= (m_freqTable[i] - m_freqTable[i-1])*(m_freqTable[i] / m_freqTable[i-1])) {
				result += noteName[i];
				return result;
			}
            else {
				result  = "~";
				result += noteName[i];
				return result;
			}
        }
        return noteName[96]; // 12 notes times 8 octaves
    }
    return noteName[0];
}
#endif

// Display console menu
void ConsolePlayer::menu() {
    if (m_quietLevel > 1) {
		if (m_driver.file)
            cerr << info_file;
        else
            cerr << ((m_quietLevel < 3) ? info_play : "Playing...");
        return;
	}

    const SidInfo     &info     = m_engine.info ();
    const SidTuneInfo *tuneInfo = m_tune.getInfo();

    // New Page
    if ((m_iniCfg.console()).ansi) {
        cerr << "\x1b[2J";   // Clear screen
        cerr << "\x1b[40m";  // black background
        cerr << "\x1b[0;0H"; // Move cursor to 0,0
		cerr << "\x1b[?25l"; // and hide it
    }

    consoleTable (tableStart);
    consoleTable (tableMiddle);
    consoleColour(white, true);

	string header;
	header.reserve(tableWidth);

	// the title first
    header.append("C64play - SID tune player");
	cerr << setw(tableWidth/2 + header.length()/2) << header << endl;
    consoleTable (tableMiddle);
    consoleColour(white, false);

	// then the description
    header.clear();
    header.append("C64play v" VERSION ", using ").append(info.name()).append(" v")
	      .append(info.version());
    cerr << setw(tableWidth/2 + header.length()/2) << header << endl;

    const unsigned int n = tuneInfo->numberOfInfoStrings();
    if (n) {
        codeConvert codeset;

        consoleTable (tableSeparator);
        consoleTable (tableMiddle);
        consoleColour(cyan, true);
        cerr << " Name         : ";
        consoleColour(white, false);
        cerr << codeset.convert(tuneInfo->infoString(0)) << endl;
        if (n > 1) {
            consoleTable (tableMiddle);
            consoleColour(cyan, true);
            cerr << " Artist(s)    : ";
            consoleColour(white, false);
            cerr << codeset.convert(tuneInfo->infoString(1)) << endl;
            consoleTable (tableMiddle);
            consoleColour(cyan, true);
            cerr << " Release      : ";
            consoleColour(white, false);
            cerr << codeset.convert(tuneInfo->infoString(2)) << endl;
        }
    }

    for (unsigned int i = 0; i < tuneInfo->numberOfCommentStrings(); ++i) {
        consoleTable (tableMiddle);
        consoleColour(cyan, true);
        cerr << " Comment      : ";
        consoleColour(magenta, false);
        cerr << tuneInfo->commentString(i) << endl;
    }

    consoleTable(tableSeparator);

    if (m_verboseLevel) {
        consoleTable (tableMiddle);
        consoleColour(green, true);
        cerr << " Format       : ";
        consoleColour(white, false);
        cerr << tuneInfo->formatString() << endl;
        consoleTable (tableMiddle);
        consoleColour(green, true);
        cerr << " File(s)      : ";
        consoleColour(white, false);
        cerr << trimString(tuneInfo->dataFileName(), 41) << endl;

        // Second file is only sometimes present
        if (tuneInfo->infoFileName()) {
            consoleTable (tableMiddle);
            consoleColour(green, true);
            cerr << "              : ";
            consoleColour(white, false);
            cerr << tuneInfo->infoFileName() << endl;
        }

        consoleTable (tableMiddle);
        consoleColour(green, true);
        cerr << " Condition    : ";
        consoleColour(white, false);
        cerr << m_tune.statusString() << endl;
    }

    consoleTable (tableMiddle);
    consoleColour(green, true);
    cerr << " Subtune      : ";
    consoleColour(white, false);

    cerr << tuneInfo->currentSong() << '/' << tuneInfo->songs() << " "
		 << "(starting subtune: " << tuneInfo->startSong() << ")";

    if (m_track.loop)
        cerr << " - looping";

    cerr << endl;

    if (m_verboseLevel) {
        consoleTable (tableMiddle);
        consoleColour(green, true);
        cerr << " Video signal : ";
        consoleColour(white, false);
        cerr << getClock(tuneInfo->clockSpeed()) << endl;
    }

    consoleTable (tableMiddle);
    consoleColour(green, true);
    cerr << " Duration     : ";
    consoleColour(white, false);
    if (m_timer.stop) {
        const uint_least32_t seconds = m_timer.stop / 1000;
        cerr << setw(2) << setfill('0') << ((seconds / 60) % 100)
             << ':' << setw(2) << setfill('0') << (seconds % 60)
             << '.' << setw(3) << m_timer.stop % 1000;
    }
    else if (m_timer.valid)
        cerr << "Infinite";
    else if (songlengthDB == SLDB_NONE)
        cerr << "No Songlength DB";
    else
        cerr << "Unknown";
    if (m_timer.start) { // Show offset
        const uint_least32_t seconds = m_timer.start / 1000;
        cerr << " (+" << setw(2) << setfill('0') << ((seconds / 60) % 100)
             << ':' << setw(2) << setfill('0') << (seconds % 60) << ")";
    }
    cerr << endl;

    if (m_verboseLevel) {
        consoleTable (tableSeparator);

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " Addresses    : " << hex;
        cerr.setf(std::ios::uppercase);
        consoleColour(white, false);

        // Display PSID Driver location
        cerr << "DRIVER: ";
        if (info.driverAddr() == 0)
            cerr << "Not present!";
        else
            cerr << "$"  << setw(4) << setfill('0') << info.driverAddr()
                 << "-$" << setw(4) << setfill('0')
				 << info.driverAddr() + (info.driverLength() - 1);

        if (tuneInfo->playAddr() == 0xffff)
            cerr << ", SYS: $" << setw(4) << setfill('0') << tuneInfo->initAddr();
        else
            cerr << ", INIT: $" << setw(4) << setfill('0') << tuneInfo->initAddr();
        cerr << endl;

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << "              : ";
        consoleColour(white, false);
        cerr << "LOAD  : $" << setw(4) << setfill('0') << tuneInfo->loadAddr()
             << "-$"        << setw(4) << setfill('0')
			 << tuneInfo->loadAddr() + (tuneInfo->c64dataLen() - 1);

        if (tuneInfo->playAddr() != 0xffff)
            cerr << ", PLAY: $" << setw(4) << setfill('0') << tuneInfo->playAddr();

        cerr << dec << endl;

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " SID details  : ";
        consoleColour(white, false);

		for (int i = 0; i < tuneInfo->sidChips(); ++i) {
			if (i >= 1) {
				consoleTable (tableMiddle);
				consoleColour(yellow, true);
            	cerr << "              : ";
				consoleColour(white, false);
			}
			
			cerr << "SID #" << i+1 << ": $" << hex << tuneInfo->sidChipBase(i)
			     << dec;
			cerr << ", model: " << getModel(tuneInfo->sidModel(i)) << endl;
		}
		cerr.unsetf(std::ios::uppercase);

        consoleTable (tableSeparator);

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " Timing       : ";
        consoleColour(white, false);
        cerr << info.speedString() << endl;

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " Channels     : ";
        consoleColour(white, false);
        cerr << (info.channels() == 1 ? "Mono" : "Stereo") << endl;

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " Filter       : ";
        consoleColour(white, false);
        cerr << (m_filter.enabled ? "Enabled" : "Disabled") << endl;

		// check if filter curve is provided by the command line
		// or by the config file
		double cfgFilter = ((getModel(tuneInfo->sidModel(0)) == SID6581) ?
		                   m_filter.filterCurve6581 : m_filter.filterCurve8580);
		consoleTable (tableMiddle);
		consoleColour(yellow, true);
		cerr << " Filter curve : ";
		consoleColour(white, false);
		cerr << ((m_fcurve == -1) ? cfgFilter : m_fcurve) << endl;

#ifdef FEAT_FILTER_RANGE
		if (getModel(tuneInfo->sidModel(0)) == SID6581) {
			consoleTable (tableMiddle);
			consoleColour(yellow, true);
			cerr << " Filter range : ";
			consoleColour(white, false);
			cerr << m_filter.filterRange6581 << endl;
		}
#endif

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " DigiBoost    : ";
        consoleColour(white, false);
        cerr << (m_engCfg.digiBoost ? "Enabled" : "Disabled") << endl;

        consoleTable (tableMiddle);
        consoleColour(yellow, true);
        cerr << " SID model    : ";
        consoleColour(white, false);
		cerr << getModel(m_engCfg.defaultSidModel) << " "
			 << (m_engCfg.forceSidModel ? "(forced)" : "(default)")
			 << endl;

        if (m_verboseLevel > 1) {
            consoleTable (tableMiddle);
            consoleColour(yellow, true);
            cerr << " Delay        : ";
            consoleColour(white, false);
            cerr << info.powerOnDelay() << " cycles at power-on" << endl;
        }
    }

    const char* romDesc = info.kernalDesc();

    consoleTable (tableSeparator);

    consoleTable (tableMiddle);
    consoleColour(magenta, true);
    cerr << " Kernal ROM   : ";
    if (strlen(romDesc) == 0) {
        consoleColour(red, false);
        cerr << "None - some tunes may not play!";
    } else {
        consoleColour(white, false);
        cerr << romDesc;
    }
    cerr << endl;

    romDesc = info.basicDesc();

    consoleTable (tableMiddle);
    consoleColour(magenta, true);
    cerr << " BASIC ROM    : ";
    if (strlen(romDesc) == 0) {
        consoleColour(red, false);
        cerr << "None - BASIC tunes won't play!";
    } else {
        consoleColour(white, false);
        cerr << romDesc;
    }
    cerr << endl;

    romDesc = info.chargenDesc();

    consoleTable (tableMiddle);
    consoleColour(magenta, true);
    cerr << " Chargen ROM  : ";
    if (strlen(romDesc) == 0) {
        consoleColour(red, false);
        cerr << "None";
    } else {
        consoleColour(white, false);
        cerr << romDesc;
    }
    cerr << endl;

#ifdef FEAT_REGS_DUMP_SID
    if (m_quietLevel >= 1) {
	    consoleTable(tableEnd);
        cerr << info_play;
		return;
    }

    if (m_verboseLevel > 1) {
        consoleTable(tableSeparator);
        consoleTable(tableMiddle);
		unsigned int movLines = (m_verboseLevel > 2) ? (tuneInfo->sidChips() * 6):
		                                               (tuneInfo->sidChips() * 3);
		cerr << "          Note  PW      Control reg.        Waveform(s)" << endl;

        for (int i = 0; i < movLines; ++i) {
            consoleTable(tableMiddle);
			cerr << endl; // reserve space for Voice 3 status
        }
    }
#endif

    consoleTable(tableEnd);

    if (m_driver.file)
        cerr << info_file << ": ";
    else
		cerr << info_play << " Time: ";

	/*
    // Get all the text to the screen so music playback
    // is not disturbed.
    if (!m_quietLevel)
        cerr << "00:00";
	// it still works without it somehow
	// still keeping it commented out if needed tho'
	*/

    cerr << flush;
}

void ConsolePlayer::refreshRegDump() {
#ifdef FEAT_REGS_DUMP_SID
    const SidTuneInfo* tuneInfo = m_tune.getInfo();
	const unsigned int movLines = (m_verboseLevel > 2) ?
								  (tuneInfo->sidChips() * 6 + 1):
							      (tuneInfo->sidChips() * 3 + 1);

    cerr << "\x1b[" << movLines << "A\r"; // Moves cursor X lines up

    for (int j=0; j < tuneInfo->sidChips(); ++j) {
    	uint8_t* registers = m_registers[j];
        uint8_t  oldCtl[3];
        oldCtl[0] = registers[0x04];
        oldCtl[1] = registers[0x0b];
        oldCtl[2] = registers[0x12];

        if (m_engine.getSidStatus(j, registers)) {
            oldCtl[0] ^= registers[0x04];
            oldCtl[1] ^= registers[0x0b];
            oldCtl[2] ^= registers[0x12];

            for (int i=0; i < 3; ++i) {
                consoleTable (tableMiddle);
                consoleColour(red, true);

                cerr << " Voice " << (j * 3 + i+1) << hex;

                consoleColour(yellow, true);

                cerr << " " << setw(4) << setfill(' ')
					 << getNote(registers[0x00 + i * 0x07] |
						       (registers[0x01 + i * 0x07] << 8));

				cerr << "  $" << setw(3) << setfill('0')
					 << (registers[0x02 + i * 0x07] |
						((registers[0x03 + i * 0x07] & 0x0f) << 8));

				cerr << "  ";

                // gate changed ?
                consoleColour((oldCtl[i] & 0x01) ? green : red, true);
                // gate on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x01) ? "GATE" : "gate");

				cerr << " ";

                // sync changed ?
                consoleColour((oldCtl[i] & 0x02) ? green : red, true);
                // sync on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x02) ? "SYNC" : "sync");

				cerr << " ";

                // ring changed ?
                consoleColour((oldCtl[i] & 0x04) ? green : red, true);
                // ring on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x04) ? "RING" : "ring");

				cerr << " ";

                // test changed ?
                consoleColour((oldCtl[i] & 0x08) ? green : red, true);
                // test on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x08) ? "TEST" : "test");

				cerr << "  ";

                // triangle changed ?
                consoleColour((oldCtl[i] & 0x10) ? green : red, true);
                // triangle on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x10) ? "TRI" : "___");

				cerr << " ";

                // sawtooth changed ?
                consoleColour((oldCtl[i] & 0x20) ? green : red, true);
                // sawtooth on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x20) ? "SAW" : "___");

				cerr << " ";

                // pulse changed ?
                consoleColour((oldCtl[i] & 0x40) ? green : red, true);
                // pulse on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x40) ? "PUL" : "___");

				cerr << " ";

                // noise changed ?
                consoleColour((oldCtl[i] & 0x80) ? green : red, true);
                // noise on ?
                cerr << ((registers[0x04 + i * 0x07] & 0x80) ? "NOI" : "___");

                cerr << dec << endl;
            }
        } else {
			for (int i=0; i < 3; ++i) {
                consoleTable(tableMiddle); cerr << "???\n";
		    }
        }
	}

	if (m_verboseLevel <= 2)
		consoleTable(tableEnd);

	else if (m_verboseLevel > 2) {
		// one big-ass bit table to access the registers here
	    uint16_t regBit[11];
		         regBit[0]  = 0x001;
		         regBit[1]  = 0x002;
	             regBit[2]  = 0x004;
		         regBit[3]  = 0x008;
		         regBit[4]  = 0x010;
		         regBit[5]  = 0x020;
	             regBit[6]  = 0x040;
	             regBit[7]  = 0x080;
    	         regBit[8]  = 0x100;
	             regBit[9]  = 0x200;
	             regBit[10] = 0x400;

        for (int j = 0; j < tuneInfo->sidChips(); ++j) {
            uint8_t* registers = m_registers[j];

	        consoleTable(tableSeparator);
	        consoleTable(tableMiddle);
            cerr << " SID #" << (j + 1) << ": "
			     << "M. vol.   Filters   F. chn. F. res.    Cutoff" << endl;
            consoleTable(tableMiddle);

            // binary volume meter, helps partially visualizing samples
			// yeah, i know it's a quite weird idea
	        consoleColour(red, true);
	        cerr << "          %";
            {
    	        for (int c = 3; c >= 0; --c) {
	                cerr << ((registers[0x18] & regBit[c]) ? "1" : "0");
	            }
            }

			// the filters!
            cerr << "  ";
            {
                const char *filOn[]  = {"LP", "BP", "HP", "3O"};
                const char *filOff[] = {"lp", "bp", "hp", "3o"};

	            for (int c = 4; c <= 7; ++c) {
                    cerr << ((registers[0x18] & regBit[c]) ?
					         filOn[c-4] : filOff[c-4])
					     << " ";
	            }
            }

			// see which voices are being filtered
            cerr << "  ";
            {
                const char *voice[] = {"1", "2", "3"};

	            for (int c = 0; c <= 2; ++c) {
	                cerr << ((registers[0x17] & regBit[c]) ? voice[c] : "-");
    	        }
            }

            // filter resonance display
            cerr << "    %";
            {
    	        for (int c = 4; c <= 7; ++c) {
                    cerr << ((registers[0x17] & regBit[c]) ? "1" : "0");
	            }
            }

			// filter cutoff frequency display
	        cerr << "  %";
            {
				/* the SID's datasheet says the first 2 bits of
				   register $15 plus all the bits on register $16
				   make up the frequency cutoff control, but here
				   for some reason this doesn't apply and then i
				   use register $16 only instead :shrug: */
				for (int a = 10; a >= 0; --a) {
					cerr << ((registers[0x16] & regBit[a]) ? "1" : "0");
				}
            }
	        cerr << dec << '\n';
	    }
        consoleTable(tableEnd);
	} else
#endif
        cerr << "\r";

    if (m_driver.file)
        cerr << info_file << ": ";
    else
		cerr << info_play << " Time: ";

    cerr << flush;
}

// Set colour of text on console
void ConsolePlayer::consoleColour(player_colour_t colour, bool bold) {
    if ((m_iniCfg.console()).ansi) {
        const char *mode = "";

        switch (colour) {
        case black:   mode = "0"; break;
        case red:     mode = "1"; break;
        case green:   mode = "2"; break;
        case yellow:  mode = "3"; break;
        case blue:    mode = "4"; break;
        case magenta: mode = "5"; break;
        case cyan:    mode = "6"; break;
        case white:   mode = "7"; break;
        }

		const char* bold_c = (bold) ? "1" : "0";
		cerr << "\x1b[" << bold_c << ";40;3" << mode << 'm';
    }
}

// Display menu outline
void ConsolePlayer::consoleTable(player_table_t table) {
    consoleColour (white, true);
    switch (table) {
    case tableStart:
        cerr << (m_iniCfg.console()).topLeft << setw(tableWidth)
             << setfill ((m_iniCfg.console()).horizontal) << ""
             << (m_iniCfg.console()).topRight;
        break;

    case tableMiddle:
        cerr << setw(tableWidth + 1) << setfill(' ') << ""
             << (m_iniCfg.console()).vertical << '\r'
             << (m_iniCfg.console()).vertical;
        return;

    case tableSeparator:
        cerr << (m_iniCfg.console()).junctionRight << setw(tableWidth)
             << setfill ((m_iniCfg.console()).horizontal) << ""
             << (m_iniCfg.console()).junctionLeft;
        break;

    case tableEnd:
        cerr << (m_iniCfg.console()).bottomLeft << setw(tableWidth)
             << setfill ((m_iniCfg.console()).horizontal) << ""
             << (m_iniCfg.console()).bottomRight;
        break;
    }

    // Move back to begining of row and skip first char
    cerr << "\n";
}


// Restore ANSI console to defaults
void ConsolePlayer::consoleRestore() {
    if ((m_iniCfg.console()).ansi) {
		cerr << "\x1b[?25h"; // cursor visibility
        cerr << "\x1b[0m";   // colors
	}
}
