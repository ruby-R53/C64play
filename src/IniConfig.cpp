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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "IniConfig.h"

#include <string>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>  /* mkdir */
#include <dirent.h>    /* opendir */

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "utils.h"
#include "ini/dataParser.h"

#include "sidcxx.h"

const TCHAR* colors[16] = {
    TEXT("black"),
    TEXT("red"),
    TEXT("green"),
    TEXT("yellow"),
    TEXT("blue"),
    TEXT("magenta"),
    TEXT("cyan"),
    TEXT("white"),
    TEXT("bright black"),
    TEXT("bright red"),
    TEXT("bright green"),
    TEXT("bright yellow"),
    TEXT("bright blue"),
    TEXT("bright magenta"),
    TEXT("bright cyan"),
    TEXT("bright white")
};

inline void debug([[ maybe_unused ]] const TCHAR *msg, [[ maybe_unused ]] const TCHAR *val) {
#ifndef NDEBUG
	SID_COUT << msg << val << std::endl;
#endif
}

inline void error(const TCHAR *msg) {
	SID_CERR << msg << std::endl;
}

inline void error(const TCHAR *msg, const TCHAR *val) {
	SID_CERR << msg << val << std::endl;
}

const TCHAR *DIR_NAME  = TEXT("C64play");
const TCHAR *FILE_NAME = TEXT("c64play.ini");

// Initialize everything else 
IniConfig::IniConfig() { clear(); }

IniConfig::~IniConfig() { clear(); }

void IniConfig::clear() {
	// [Player] section
	player_s.database.clear ();
	player_s.playLength   = 0;				 // infinite play time!
	player_s.recordLength = (4 * 60) * 1000; // 4 minutes recording time
#ifdef FEAT_NEW_PLAY_API
	player_s.fadeoutLen   = 0; // no fade out by default
#endif
	player_s.kernalRom.clear();
	player_s.basicRom.clear ();
	player_s.chargenRom.clear();
	player_s.verboseLevel = 0;
	player_s.quietLevel   = 0;

	// [Console] section, characters
	console_s.ansi			= false;
	console_s.topLeft		= '+';
	console_s.topRight		= '+';
	console_s.bottomLeft	= '+';
	console_s.bottomRight	= '+';
	console_s.vertical		= '|';
	console_s.horizontal	= '-';
	console_s.junctionLeft	= ':';
	console_s.junctionRight = ':';

	// and colors
   	console_s.decorations   = bright_white;
    console_s.title         = bright_white;
	console_s.versions      = white;
    console_s.info_label    = bright_cyan;
    console_s.info_text     = white;
    console_s.file_label    = bright_green;
    console_s.file_text     = white;
	console_s.addr_label    = bright_magenta;
	console_s.addr_text     = white;
	console_s.chip_label    = bright_yellow;
	console_s.chip_text     = white;
	console_s.rom_label     = bright_magenta;
	console_s.rom_text      = white;
    console_s.note          = cyan;
    console_s.control_on    = green;
    console_s.control_off   = yellow;
	console_s.wave_on       = green;
	console_s.wave_off      = blue;
	console_s.misc_regs     = yellow;

	// [Audio] section
	audio_s.sampleRate = SidConfig::DEFAULT_SAMPLING_FREQ;
	audio_s.channels   = 0;
	audio_s.bitDepth   = 16;

	// [Emulation] section
	emulation_s.modelDefault = SidConfig::PAL;
	emulation_s.modelForced  = false;
	emulation_s.sidModel	 = SidConfig::MOS6581;
	emulation_s.forceModel	 = false;
	emulation_s.ciaModel	 = SidConfig::MOS6526;
	emulation_s.filter		 = true;
	emulation_s.engine.clear();

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
	emulation_s.bias			= 0.5;
#endif

	emulation_s.filterCurve6581 = 0.5;
#ifdef FEAT_FILTER_RANGE
	emulation_s.filterRange6581 = 0.5;
#endif
	emulation_s.filterCurve8580 = 0.5;
#ifdef FEAT_CW_STRENGTH
	emulation_s.combinedWaveformsStrength = SidConfig::AVERAGE;
#endif
	emulation_s.powerOnDelay   = -1; // -1 means the delay will be random
	emulation_s.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
	emulation_s.fastSampling   = false;
}


// static helpers
const TCHAR* readKey(iniHandler &ini, const TCHAR *key) {
	const TCHAR* value = ini.getValue(key);
	if (value == nullptr) { // Doesn't exist? Add it!
		ini.addValue(key, TEXT(""));
		debug(TEXT("Key doesn't exist: "), key);
	}
	else if (!value[0]) {
		// Ignore empty values
		return nullptr;
	}

	return value;
}

void readDouble(iniHandler &ini, const TCHAR *key, double &result) {
	const TCHAR* value = readKey(ini, key);
	if (value == nullptr)
		return;

	try {
		result = dataParser::parseDouble(value);
	}
	catch (dataParser::parseError const &e) {
		error(TEXT("Error parsing double at "), key);
	}
}

void readInt(iniHandler &ini, const TCHAR *key, int &result) {
	const TCHAR* value = readKey(ini, key);
	if (value == nullptr)
		return;

	try {
		result = dataParser::parseInt(value);
	}
	catch (dataParser::parseError const &e) {
		error(TEXT("Error parsing int at "), key);
	}
}

void readBool(iniHandler &ini, const TCHAR *key, bool &result) {
	const TCHAR* value = readKey(ini, key);
	if (value == nullptr)
		return;

	try {
		result = dataParser::parseBool(value);
	}
	catch (dataParser::parseError const &e) {
		error(TEXT("Error parsing bool at "), key);
	}
}

SID_STRING readString(iniHandler &ini, const TCHAR *key) {
	const TCHAR* value = ini.getValue(key);
	if (value == nullptr) {
		// Doesn't exist, add it
		ini.addValue(key, TEXT(""));
		debug(TEXT("Key doesn't exist: "), key);
		return SID_STRING();
	}

	return SID_STRING(value);
}

void readChar(iniHandler &ini, const TCHAR *key, char &ch) {
	SID_STRING str = readString(ini, key);
	if (str.empty())
		return;

	TCHAR c = 0;

	// Check if we have an actual character
	if (str[0] == '\'') {
		if (str[2] != '\'')
			return;
		else
			c = str[1];
	} else { // Nope, it's a number
		try {
			c = dataParser::parseInt(str.c_str());
		}
		catch (dataParser::parseError const &e) {
			error(TEXT("Error parsing int at "), key);
		}
	}

	// Clip off special characters
	if ((unsigned) c >= 32)
		ch = c;
}

bool readColor(iniHandler &ini, const TCHAR *key, color_t &ch) {
    SID_STRING str = readString(ini, key);
    if (str.empty())
        return false;

    for (int i = 0; i < 16; ++i) {
        if (str.compare(colors[i]) == 0) {
            ch = static_cast<color_t>(i);
            return true;
        }
    }

    return false;
}

bool readTime(iniHandler &ini, const TCHAR *key, int &value) {
	SID_STRING str = readString(ini, key);
	if (str.empty())
		return false;

	int time;
	int milliseconds = 0;
	const size_t sep = str.find_first_of(':');
	const size_t dot = str.find_first_of('.');
	try {
		if (sep == SID_STRING::npos) { // User gave seconds?
			time = dataParser::parseInt(str.c_str());
		} else { // Read in MM:SS.mmm format
			const int min = dataParser::parseInt(str.substr(0, sep).c_str());
			if (min < 0 || min > 99)
				goto IniCofig_readTime_error;

			time = min * 60;

			int sec;
			if (dot == SID_STRING::npos) {
				sec = dataParser::parseInt(str.substr(sep + 1).c_str());
			} else {
				sec = dataParser::parseInt(str.substr(sep + 1, dot - sep).c_str());
				SID_STRING msec = str.substr(dot + 1);
				milliseconds = dataParser::parseInt(msec.c_str());

				switch (msec.length()) {
					case 1: milliseconds *= 100; break;
					case 2: milliseconds *= 10; break;
					case 3: break;
					default: goto IniCofig_readTime_error;
				}
			}

			if (sec < 0 || sec > 59)
				goto IniCofig_readTime_error;

			time += sec;
		}
	}
	catch (dataParser::parseError const &e) {
		error(TEXT("Error parsing time at "), key);
		return false;
	}

	value = time * 1000 + milliseconds;
	return true;

IniCofig_readTime_error:
	error(TEXT("Invalid time at "), key);
	return false;
}


void IniConfig::readPlayer(iniHandler &ini) {
	if (!ini.setSection(TEXT("Player")))
		ini.addSection(TEXT("Player"));

	player_s.database = readString(ini, TEXT("Songlength DB"));

	if (player_s.database.empty()) {
		SID_STRING buffer(utils::getDataPath());
		buffer.append(SEPARATOR).append(DIR_NAME).append(SEPARATOR)
			  .append("Songlengths.md5");
		if (::access(buffer.c_str(), R_OK) == 0)
			player_s.database.assign(buffer);
	}

	int time;
	if (readTime(ini, TEXT("Default Play Time"), time))
		player_s.playLength = time;

	if (readTime(ini, TEXT("Default Record Time"), time))
		player_s.recordLength = time;

#ifdef FEAT_NEW_PLAY_API
	if (readTime(ini, TEXT("Fade Out Length"), time))
		player_s.fadeoutLen = time;
#endif

	player_s.kernalRom	= readString(ini, TEXT("Kernal ROM"));
	player_s.basicRom	= readString(ini, TEXT("BASIC ROM"));
	player_s.chargenRom = readString(ini, TEXT("Chargen ROM"));

	readInt(ini, TEXT("Verboseness"), player_s.verboseLevel);
	readInt(ini, TEXT("Quietness"), player_s.quietLevel);
}


void IniConfig::readConsole(iniHandler &ini) {
	if (!ini.setSection(TEXT("Console")))
		ini.addSection(TEXT("Console"));

	readBool(ini, TEXT("ANSI"),                console_s.ansi);
	readChar(ini, TEXT("Top Left Char"),       console_s.topLeft);
	readChar(ini, TEXT("Top Right Char"),      console_s.topRight);
	readChar(ini, TEXT("Bottom Left Char"),    console_s.bottomLeft);
	readChar(ini, TEXT("Bottom Right Char"),   console_s.bottomRight);
	readChar(ini, TEXT("Vertical Char"),       console_s.vertical);
	readChar(ini, TEXT("Horizontal Char"),     console_s.horizontal);
	readChar(ini, TEXT("Junction Left Char"),  console_s.junctionLeft);
	readChar(ini, TEXT("Junction Right Char"), console_s.junctionRight);

	readColor(ini, TEXT("Decoration Color"),      console_s.decorations);
    readColor(ini, TEXT("Title Color"),           console_s.title);
	readColor(ini, TEXT("Version Color"),         console_s.versions);
    readColor(ini, TEXT("Info Label Color"),      console_s.info_label);
    readColor(ini, TEXT("Info Text Color"),       console_s.info_text);
    readColor(ini, TEXT("File Label Color"),      console_s.file_label);
    readColor(ini, TEXT("File Text Color"),       console_s.file_text);
    readColor(ini, TEXT("Addresses Label Color"), console_s.addr_label);
    readColor(ini, TEXT("Addresses Text Color"),  console_s.addr_text);
    readColor(ini, TEXT("Chip Label Color"),      console_s.chip_label);
    readColor(ini, TEXT("Chip Text Color"),       console_s.chip_text);
    readColor(ini, TEXT("ROM Label Color"),       console_s.rom_label);
    readColor(ini, TEXT("ROM Text Color"),        console_s.rom_text);
    readColor(ini, TEXT("Control On Color"),      console_s.control_on);
    readColor(ini, TEXT("Control Off Color"),     console_s.control_off);
    readColor(ini, TEXT("Wave On Color"),         console_s.wave_on);
    readColor(ini, TEXT("Wave Off Color"),        console_s.wave_off);
    readColor(ini, TEXT("Misc Registers Color"),  console_s.misc_regs);
}

void IniConfig::readAudio(iniHandler &ini) {
	if (!ini.setSection(TEXT("Audio")))
		ini.addSection(TEXT("Audio"));

	readInt(ini, TEXT("Sample Rate"), audio_s.sampleRate);
    readInt(ini, TEXT("Channels"),    audio_s.channels);
	readInt(ini, TEXT("Bit Depth"),   audio_s.bitDepth);
}


void IniConfig::readEmulation(iniHandler &ini) {
	if (!ini.setSection(TEXT("Emulation")))
		ini.addSection(TEXT("Emulation"));

	emulation_s.engine = readString(ini, TEXT("Engine"));

	{
		SID_STRING str = readString(ini, TEXT("Video Mode"));
		if (!str.empty()) {
			if (str.compare(TEXT("PAL")) == 0)
				emulation_s.modelDefault = SidConfig::PAL;
			else if (str.compare(TEXT("NTSC")) == 0)
				emulation_s.modelDefault = SidConfig::NTSC;
			else if (str.compare(TEXT("OLD_NTSC")) == 0)
				emulation_s.modelDefault = SidConfig::OLD_NTSC;
			else if (str.compare(TEXT("DREAN")) == 0)
				emulation_s.modelDefault = SidConfig::DREAN;
		}
	}

	readBool(ini, TEXT("Force Video Mode"), emulation_s.modelForced);
	readBool(ini, TEXT("DigiBoost"), emulation_s.digiboost);

	{
		SID_STRING str = readString(ini, TEXT("CIA Model"));
		if (!str.empty()) {
			if (str.compare(TEXT("MOS6526")) == 0)
				emulation_s.ciaModel = SidConfig::MOS6526;
			else if (str.compare(TEXT("MOS8521")) == 0)
				emulation_s.ciaModel = SidConfig::MOS8521;
		}
	}

	{
		SID_STRING str = readString(ini, TEXT("SID Model"));
		if (!str.empty()) {
			if (str.compare(TEXT("MOS6581")) == 0)
				emulation_s.sidModel = SidConfig::MOS6581;
			else if (str.compare(TEXT("MOS8580")) == 0)
				emulation_s.sidModel = SidConfig::MOS8580;
		}
	}

	readBool(ini, TEXT("Force SID Model"), emulation_s.forceModel);

	readBool(ini, TEXT("Filter Emulation"), emulation_s.filter);

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
	readDouble(ini, TEXT("Filter Bias"), emulation_s.bias);
#endif

	readDouble(ini, TEXT("6581 Filter Curve"), emulation_s.filterCurve6581);
#ifdef FEAT_FILTER_RANGE
	readDouble(ini, TEXT("6581 Filter Range"), emulation_s.filterRange6581);
#endif
	readDouble(ini, TEXT("8580 Filter Curve"), emulation_s.filterCurve8580);

#ifdef FEAT_CW_STRENGTH
	{
		SID_STRING str = readString(ini, TEXT("Combined Wave Strength"));
		if (!str.empty()) {
			if (str.compare(TEXT("AVERAGE")) == 0)
				emulation_s.combinedWaveformsStrength = SidConfig::AVERAGE;
			else if (str.compare(TEXT("WEAK")) == 0)
				emulation_s.combinedWaveformsStrength = SidConfig::WEAK;
			else if (str.compare(TEXT("STRONG")) == 0)
				emulation_s.combinedWaveformsStrength = SidConfig::STRONG;
		}
	}
#endif

	readInt(ini, TEXT("Power-On Delay"), emulation_s.powerOnDelay);

	{
		SID_STRING str = readString(ini, TEXT("Resampling"));
		if (!str.empty()) {
			if (str.compare(TEXT("INTERPOLATE")) == 0)
				emulation_s.samplingMethod = SidConfig::INTERPOLATE;
			else if (str.compare(TEXT("RESAMPLE")) == 0)
				emulation_s.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
		}
	}

#ifdef HAVE_RESIDFP_BUILDERS_RESID_H
	readBool(ini, TEXT("reSID's Fast Sampling"), emulation_s.fastSampling);
#endif
}

class iniError {
private:
	const SID_STRING msg;

public:
	iniError(const TCHAR* msg) : msg(msg) {}
	const SID_STRING message() const { return msg; }
};

void createDir(const SID_STRING& path) {
	DIR *dir = opendir(path.c_str());
	if (dir) {
		closedir(dir);
	}
	else if (errno == ENOENT) {
		if (mkdir(path.c_str(), 0755) < 0) {
			throw iniError(strerror(errno));
		}
	} else {
		throw iniError(strerror(errno));
	}
}

SID_STRING getConfigPath() {
	SID_STRING configPath;

	try {
		configPath = utils::getConfigPath();
	}
	catch (utils::error const &e) {
		throw iniError(TEXT("Cannot get config path!"));
	}

	debug(TEXT("Config path: "), configPath.c_str());

	// Make sure the config path exists
	createDir(configPath);

	configPath.append(SEPARATOR).append(DIR_NAME);

	// Make sure the app config path exists
	createDir(configPath);

	configPath.append(SEPARATOR).append(FILE_NAME);

	debug(TEXT("Config file: "), configPath.c_str());

	return configPath;
}

bool tryOpen([[ maybe_unused ]] iniHandler &ini) {
	return false;
}

void IniConfig::read() {
	clear();

	iniHandler ini;

	if (!tryOpen(ini)) {
		try {
			SID_STRING configPath = getConfigPath();

			// Opens an existing file or creates a new one
			if (!ini.open(configPath.c_str())) {
				error(TEXT("Error reading config file!"));
				return;
			}
		} catch (iniError const &e) {
			error(e.message().c_str());
			return;
		}
	}

	readPlayer	 (ini);
	readConsole  (ini);
	readAudio	 (ini);
	readEmulation(ini);

	m_fileName = ini.getFilename();

	ini.close();
}
