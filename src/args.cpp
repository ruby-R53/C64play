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

#include <iostream>

#include <cstring>
#include <climits>
#include <cstdlib>

#include "ini/types.h"

#include "sidlib_features.h"

#include "sidcxx11.h"

using std::cout;
using std::cerr;
using std::endl;

// Wide-chars are not supported here yet 
#undef SEPARATOR
#define SEPARATOR "/"

/**
 * Try load SID tune from HVSC_BASE
 */
bool ConsolePlayer::tryOpenTune(const char *hvscBase) {
    std::string newFileName(hvscBase);

    newFileName.append(SEPARATOR).append(m_filename);
    m_tune.load(newFileName.c_str());
    if (!m_tune.getStatus()) {
        return false;
    }

    m_filename.assign(newFileName);
    return true;
}

/**
 * Try load songlength DB from HVSC_BASE
 */
bool ConsolePlayer::tryOpenDatabase(const char *hvscBase) {
    std::string newFileName(hvscBase);

    newFileName.append(SEPARATOR).append("DOCUMENTS").append(SEPARATOR)
	           .append("Songlengths.md5");

    return m_database.open(newFileName.c_str());
}

// Convert time from integer
bool parseTime(const char *str, uint_least32_t &time) {
    // Check for empty string
    if (*str == '\0')
        return false;

    uint_least32_t _time;
    uint_least32_t milliseconds = 0;

    char *sep = (char *) strstr (str, ":");
    if (!sep) { // User gave seconds
        _time = atoi(str);
    } else { // Read in MM:SS[.mmm] format
        int val;
        *sep = '\0';
        val  = atoi(str);
        if (val < 0 || val > 99)
            return false;
        _time = (uint_least32_t) val * 60;

        // parse milliseconds
        char *milli = (char *) strstr(sep+1, ".");
        if (milli) {
            char *start = milli + 1;
            char *end;
            milliseconds = strtol(start, &end, 10);
            switch (end - start) {
            	case 1:  milliseconds *= 100; break;
            	case 2:  milliseconds *= 10; break;
            	case 3:  break;
            	default: return false;
            }

            if (milliseconds > 999)
                return false;

            *milli = '\0';
        }
        val = atoi (sep + 1);
        if (val < 0 || val > 59)
            return false;
        _time += (uint_least32_t) val;
    }

    time = _time * 1000 + milliseconds;
    return true;
}

bool parseAddress(const char *str, uint_least16_t &address) {
    if (*str == '\0')
        return false;

    long x = strtol(str, nullptr, 0);

    address = x;
    return true;
}

void displayDebugArgs() {
    std::ostream &out = cout;

    out << "Debug options:" << endl
        << "--cpu-debug    display CPU registers and assembly dumps" << endl
        << "--delay=<num>  simulate C64 power-on delay (default: random)" << endl
        << "--noaudio      no audio output device" << endl
        << "--nosid        no SID emulation" << endl
        << "--none         no audio output device and no SID emulation" << endl;
}

// Parse command line arguments
int ConsolePlayer::args(int argc, const char *argv[]) {
    if (argc == 0) { // at least one argument required
        displayArgs();
        return -1;
    }

    // default arg options
    m_driver.output = OUT_SOUNDCARD;
    m_driver.file   = false;
    m_driver.info   = false;

	m_mute_channel.reset();

    int  infile = 0;
    int  i      = 0;
    bool err    = false;

    // parse command line arguments
    while ((i < argc) && (argv[i] != nullptr)) {
        if ((argv[i][0] == '-') && (argv[i][1] != '\0')) {
            // help options
            if ((argv[i][1] == 'h') || !std::strcmp(&argv[i][1], "-help")) {
                displayArgs();
                return 0;
            }

            else if (!std::strcmp(&argv[i][1], "-help-debug")) {
                displayDebugArgs();
                return 0;
            }

            else if (argv[i][1] == 'b') {
                if (!parseTime(&argv[i][2], m_timer.start))
                    err = true;
            }

			// Override sidTune and enable SID #2
            else if (argv[i][1] == 'D') {
                if (!parseAddress(&argv[i][2], m_engCfg.secondSidAddress))
                    err = true;
            }

			// Override sidTune and enable SID #3
            else if (argv[i][1] == 'T') {
                if (!parseAddress(&argv[i][2], m_engCfg.thirdSidAddress))
                    err = true;
            }

            else if (argv[i][1] == 'f') {
                if (argv[i][2] == '\0')
                    err = true;
                m_engCfg.frequency = (uint_least32_t) atoi(argv[i]+2);
            }

            // Disable filter emulation?
            else if (strncmp (&argv[i][1], "nf", 2) == 0) {
                if (argv[i][3] == '\0')
                    m_filter.enabled = false;
            }

            // Track options
			else if (argv[i][1] == 'o') {
                if (argv[i][2] == 'l') {
                    m_track.loop   = true;
                    m_track.single = ((argv[i][3] == 's') ? true : false);
                    m_track.first  = atoi(&argv[i][((argv[i][3] == 's') ? 4 : 3)]);
                }
                else if (argv[i][2] == 's') {
                    m_track.loop   = ((argv[i][3] == 'l') ? true : false);
                    m_track.single = true;
                    m_track.first  = atoi(&argv[i][((argv[i][3] == 'l') ? 4 : 3)]);
                } else { // User didn't provide track number?
                    m_track.first = atoi(&argv[i][2]);
                }
            }

            else if (argv[i][1] == 'p') {
                if (argv[i][2] == '\0') // user didn't provide anything?
                    err = true;
                {
                    uint_least8_t precision = atoi(&argv[i][2]);
                    if (precision <= 16)
                        m_bitDepth = 16;
                    else
                        m_bitDepth = 32;
                }
            }

            else if (argv[i][1] == 'q') {
                if (argv[i][2] == '\0')
                    m_quietLevel = 1;
                else
                    m_quietLevel = atoi(&argv[i][2]);
            }

            else if (argv[i][1] == 't') {
                if (!parseTime(&argv[i][2], m_timer.length))
                    err = true;

                m_timer.valid = true;
            }

            // Resampling options
			else if (argv[i][1] == 'r') {
                if (argv[i][2] == 'i') {
                    m_engCfg.samplingMethod = SidConfig::INTERPOLATE;
                }
                else if (argv[i][2] == 'r') {
                    m_engCfg.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
                } else {
                    err = true;
                }
                m_engCfg.fastSampling = ((argv[i][3] == 'f') ? true : false);
            }

            // SID model + audio layout options
			else if (argv[i][1] == 'm') { // Mono playback
                if (argv[i][2] == '\0') {
                    m_channels = 1;
                }
                else if (argv[i][2] == 'o') { // use the 6581 chip
                    m_engCfg.defaultSidModel = SidConfig::MOS6581;
                }
                else if (argv[i][2] == 'n') { // or the 8580 one
                    m_engCfg.defaultSidModel = SidConfig::MOS8580;
				}

#ifdef FEAT_SAMPLE_MUTE
				// Sample channel muting
				else if (argv[i][2] >= 'a' && argv[i][2] <= 'c') {
					m_mute_samples[(argv[i][2] - 'a')] = true;
                }
#endif

				else {
					// Channel muting
                    const int voice = atoi(&argv[i][2]);
                    if (voice > 0 && voice <= m_mute_channel.size())
                        m_mute_channel[voice-1] = true;
                }

                m_engCfg.forceSidModel = ((argv[i][3] == 'f') ? true : false);
            }

            else if (std::strcmp (&argv[i][1], "-digiboost") == 0) {
                m_engCfg.digiBoost = true;
            }

            // Video/Verbose Options
			else if (argv[i][1] == 'v') {
                if (argv[i][2] == '\0')
                    m_verboseLevel = 1;
                else if (argv[i][2] == 'f') {
                    m_engCfg.forceC64Model = true;
                }
                else if (argv[i][2] == 'n') {
                    m_engCfg.defaultC64Model = SidConfig::NTSC;
                }
                else if (argv[i][2] == 'p') {
                    m_engCfg.defaultC64Model = SidConfig::PAL;
                } else {
                    m_verboseLevel = atoi(&argv[i][2]);
                }
                m_engCfg.forceC64Model = ((argv[i][((argv[i][2] == 'f') ? 
										 2 : 3)] == 'f') ?  true : false);
            }

            else if (strncmp (&argv[i][1], "-delay=", 7) == 0) {
                m_engCfg.powerOnDelay = (uint_least16_t) atoi(&argv[i][8]);
            }

            else if (strncmp (&argv[i][1], "-fcurve=", 8) == 0) {
                if (strncmp (&argv[i][9], "auto", 4) == 0) {
                    m_autofilter = true;
                } else {
                    m_fcurve = atof(&argv[i][9]);
                }
            }

#ifdef FEAT_FILTER_RANGE
            else if (strncmp (&argv[i][1], "-frange=", 8) == 0) {
                if (strncmp (&argv[i][9], "auto", 4) == 0) {
                    m_autofilter = true;
                } else {
                    m_frange = atof(&argv[i][9]);
                }
            }
#endif

            // Render it to a file?
            else if (argv[i][1] == 'w') {
                m_driver.output = OUT_WAV;
                m_driver.file   = true;
                if (argv[i][2] != '\0')
                    m_outfile = &argv[i][2];
            }

            else if (strncmp (&argv[i][1], "-wav", 4) == 0) {
                m_driver.output = OUT_WAV;
                m_driver.file   = true;
                if (argv[i][5] != '\0')
                    m_outfile = &argv[i][5];
            }

            else if (strncmp (&argv[i][1], "-info", 5) == 0) {
                m_driver.info   = true;
            }

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
            else if (std::strcmp (&argv[i][1], "-residfp") == 0) {
                m_driver.sid    = EMU_RESIDFP;
            }
#endif // HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
            else if (std::strcmp (&argv[i][1], "-resid") == 0) {
                m_driver.sid    = EMU_RESID;
            }
#endif // HAVE_SIDPLAYFP_BUILDERS_RESID_H

            // These are for debug
            else if (std::strcmp (&argv[i][1], "-none") == 0) {
                m_driver.sid    = EMU_NONE;
                m_driver.output = OUT_NULL;
            }

            else if (std::strcmp (&argv[i][1], "-nosid") == 0) {
                m_driver.sid = EMU_NONE;
            }

            else if (std::strcmp (&argv[i][1], "-noaudio") == 0) {
                m_driver.output = OUT_NULL;
            }

            else if (std::strcmp (&argv[i][1], "-cpu-debug") == 0) {
                m_cpudebug = true;
            }

            else {
                err = true;
            }

        } else { // Reading file name
            if (infile == 0)
                infile = i;
            else
                err = true;
        }

        if (err) {
            displayArgs(argv[i]);
            return -1;
        }

        ++i;  // next index
    }

    const char* hvscBase = getenv("HVSC_BASE");

    // Load the tune
    m_filename = argv[infile];
    m_tune.load(m_filename.c_str());
    if (!m_tune.getStatus()) {
        std::string errorString(m_tune.statusString());

        // Try prepending HVSC_BASE
        if (!hvscBase || !tryOpenTune(hvscBase)) {
            displayError(errorString.c_str());
            return -1;
        }
    }

    // If filename specified we can only convert one song
    if (m_outfile != nullptr)
        m_track.single = true;

    // Can only loop if not creating audio files
    if (m_driver.output > OUT_SOUNDCARD)
        m_track.loop = false;

    if (m_driver.info && m_driver.file) {
        displayError("WARNING: metadata can only be added to WAV files!");
    }

    // Select the desired track
    m_track.first    = m_tune.selectSong(m_track.first);
    m_track.selected = m_track.first;

    if (m_track.single)
        m_track.songs = 1;

    // If user provided no time then load songlength database
    // and set default lengths in case it's not found in there.
    {   // Time of 0 provided for wav generation?
        if (m_driver.file && m_timer.valid && !m_timer.length) {
            displayError("ERROR: can't use -t0 if recording!");
            return -1;
        }
        if (!m_timer.valid) {
            m_timer.length = m_driver.file ? m_iniCfg.playercfg().recordLength
                                           : m_iniCfg.playercfg().playLength;

            songlengthDB  = false;
            bool dbOpened = false;
            if (hvscBase) {
                if (tryOpenDatabase(hvscBase)) {
                    dbOpened = true;
                    songlengthDB = true;
                }
            }

            if (!dbOpened) {
                // Try load user configured songlength DB
                if (m_iniCfg.playercfg().database.length() != 0) {
                    // Try loading the database specificed by the user
                    const char *database = m_iniCfg.playercfg().database.c_str();

                    if (!m_database.open(database)) {
                        displayError(m_database.error());
                        return -1;
                    }

                    if (m_iniCfg.playercfg().database.find(TEXT(".md5")) != SID_STRING::npos)
                        songlengthDB = true;
                }
            }
        }
    }

    // Configure engine with settings
    if (!m_engine.config(m_engCfg)) { // Config failed
        displayError(m_engine.error());
        return -1;
    }
    return 1;
}


void ConsolePlayer::displayArgs (const char *arg) {
    std::ostream &out = arg ? cerr : cout;

    if (arg)
        out << "Invalid option: " << arg << endl;
    else
        out << "Syntax: " << m_name << " [options] <file>" << endl;

    out << "Options:" << endl
        << "--help | -h        display this screen" << endl
        << "--help-debug       debug help menu" << endl
        << "-b<num>            set start time in [min:]sec[.mil] format" << endl
		<< "                   format, defaults to 0" << endl
        << "-f<num>            set sample rate in Hz, defaults to "
        << SidConfig::DEFAULT_SAMPLING_FREQ << endl
        << "-D<addr>           set address of SID #2 (e.g. -ds0xd420)" << endl
        << "-T<addr>           set address of SID #3 (e.g. -ts0xd440)" << endl
        << "-m<num|a-c>        mute voice <num> (e.g. -m1 -m2), use" << endl
		<< "                   'a','b' or 'c' for muting sample playback" << endl
        << "-nf                disable filter emulation" << endl
        << "-o<l|s>            loop and/or make it single track" << endl
        << "-o<num>            start track (default: preset)" << endl
        << "-p<num>            set format for file output: 16 for signed" << endl
		<< "                   16-bit, and 32 for 32-bit float. Defaults" << endl
		<< "                   to unsigned 16-bit" << endl
        << "-s                 use stereo output" << endl
        << "-m                 use mono output" << endl
        << "-t<num>            set play length in [mins:]secs[.milli]" << endl
		<< "                   format, use 0 for infinite play time" << endl
        << "-<v|q>[x]          [v]erbose or [q]uiet output, where [x] is" << endl
		<< "                   an optional level that defaults to 1" << endl
        << "-v[p|n][f]         set VIC's clock to [P]AL or [N]TSC, default" << endl
		<< "                   defined by song. You may also [f]orce" << endl
		<< "                   the setting to prevent speed fixing" << endl
        << "-m<o|n>[f]         set SID model to [o]ld (MOS6581) or [n]ew" << endl
		<< "                   (CSG8580), defaults to the old version. You" << endl
        << "                   may also [f]orce that setting" << endl
        << "--digiboost        enable the DigiBoost hack for the 8580 chip" << endl
        << "-r[i|r][f]         set resampling method, either [i]nterpolate" << endl
		<< "                   or [r]esample. If you're using reSID, you" << endl
		<< "                   may also enable [f]ast resampling" << endl
        << "--fcurve=<d|auto>  controls the filter curve for the reSIDfp" << endl
		<< "                   emulation (default: 0.5, ranges from 0.0 to" << endl
		<< "                   1.0)" << endl

#ifdef FEAT_FILTER_RANGE
        << "--frange=<d|auto>  controls the filter range in the ReSIDfp" << endl
		<< "                   emulation (same default and value range)" << endl
#endif

		<< "                   emulation. Ranges from 0.0 to 1.0 and" << endl
		<< "                   defaults to 0.5. You may also let C64play" << endl
		<< "                   [auto]matically change it for you" << endl
        << "-w[name]           render a WAV file, with default name" << endl
		<< "                   being <file>[n].wav" << endl
        << "--info             add metadata to WAV file" << endl;

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    out << "--residfp          use reSIDfp emulation (default)" << endl;
#endif

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    out << "--resid            use reSID emulation" << endl;
#endif

    out << endl
        << "Home page: " PACKAGE_URL;
}
