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

#ifndef PLAYER_H
#define PLAYER_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string>
#include <bitset>

#include <sidplayfp/SidTune.h>
#include <sidplayfp/sidplayfp.h>
#include <sidplayfp/SidConfig.h>
#include <sidplayfp/SidTuneInfo.h>
#include <sidplayfp/SidDatabase.h>

#include "audio/IAudio.h"
#include "audio/AudioConfig.h"
#include "audio/null/null.h"
#include "IniConfig.h"
#include "settings.h"

#ifdef FEAT_NEW_PLAY_API
# include <mixer.h>
#endif

#include "sidlib_features.h"

typedef enum {
	black,
	red,
	green,
	yellow,
	blue,
	magenta,
	cyan,
	white
} color_t;

typedef enum {
	tableStart,
	tableMiddle,
	tableSeparator,
	tableEnd
} table_t;

typedef enum {
    playerError = 0,
	playerRunning,
	playerPaused,
	playerStopped,
    playerRestart,
	playerExit,
	playerFast = 128,
    playerFastRestart = playerRestart | playerFast,
    playerFastExit = playerExit | playerFast
} player_state_t;

typedef enum {
	// Same as EMU_DEFAULT except that there's
	// no soundcard. But still allows WAV generation
    EMU_NONE = 0,
    // The following require a soundcard
    EMU_DEFAULT, EMU_RESIDFP, EMU_RESID,
	EMU_END
} SIDEMUS;

typedef enum {
	OUT_NULL,
	OUT_SOUNDCARD,
	OUT_WAV,
	OUT_END
} OUTPUTS;

class Chip {
	public:
	enum type {
		UNKNOWN_ANY,
		MOS6581,
		CSG8580
	};
};

// Grouped global variables
class ConsolePlayer {
private:
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    static const char RESIDFP_ID[];
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    static const char RESID_ID[];
#endif

    const char* const m_name;
    sidplayfp         m_engine;
    SidConfig         m_engCfg;
    SidTune           m_tune;
    std::string       m_filename;
    const char*       m_outfile;

	player_state_t    m_state;

    IniConfig       m_iniCfg;
    SidDatabase     m_database;

    Setting<double> m_fcurve;

#ifdef FEAT_FILTER_RANGE
    Setting<double> m_frange;
#endif

#ifdef FEAT_CW_STRENGTH
    SidConfig::
	sid_cw_t        m_combinedWaveformsStrength;
#endif

    uint8_t         m_registers[3][32];
    uint16_t*       m_freqTable;

    // Display parameters
    uint_least8_t   m_quietLevel;
    uint_least8_t   m_verboseLevel;

	bool            songlengthDB;

    bool            m_cpudebug;
	std::bitset<9>  m_mute_channel;

#ifdef FEAT_SAMPLE_MUTE
    std::bitset<3>  m_mute_samples;
#endif

    uint_least8_t   m_channels;
    uint_least8_t   m_bitDepth;

#ifdef FEAT_NEW_PLAY_API
	Mixer m_mixer;
#endif

    struct m_filter_t {
        bool        enabled;

        // Filter parameter for reSID
        double      bias;

        // Filter parameters for reSIDfp
        double      filterCurve6581;
#ifdef FEAT_FILTER_RANGE
        double      filterRange6581;
#endif
        double      filterCurve8580;
    } m_filter;

    struct m_driver_t {
        OUTPUTS     output;   // Selected output type
        SIDEMUS     sid;      // SID emulation
        bool        file;     // File based driver
        bool        info;     // File metadata
        AudioConfig cfg;
        IAudio*     selected; // Selected Output Driver
        IAudio*     device;   // Sound card/File Driver
        Audio_Null  null;     // Used for everything
    } m_driver;

    struct m_timer_t { // secs
        uint_least32_t start;
		uint_least32_t current;
        uint_least32_t stop;
        uint_least32_t length;
        bool           valid;
        bool           starting;
    } m_timer;

#ifdef FEAT_NEW_PLAY_API
	uint_least32_t m_fadeoutLen;
#endif

    struct m_track_t {
        uint16_t first;
        uint16_t selected;
        uint16_t songs;
        bool     loop;
        bool     single;
    } m_track;

    struct m_speed_t {
        uint8_t current;
        uint8_t max;
    } m_speed;

private:
    // Console
    void consoleColor  (color_t color, bool bold);
    void consoleTable  (table_t table);
    void consoleRestore(void);

    // Command line args
    void displayArgs   (const char *arg = nullptr);

    bool createOutput  (OUTPUTS driver, const SidTuneInfo *tuneInfo);
    bool createSidEmu  (SIDEMUS emu, const SidTuneInfo *tuneInfo);
    void decodeKeys    (void);
    void updateDisplay (void);
    void menu          (void);
    void refreshRegDump(void);

    uint_least32_t getBufSize();

	std::string getNote(uint16_t freq);

    std::string getFileName(const SidTuneInfo *tuneInfo, const char* ext);

    inline bool tryOpenTune(const char *hvscBase);
    inline bool tryOpenDatabase(const char *hvscBase);

public:
    ConsolePlayer(const char * const name);
    virtual ~ConsolePlayer() = default;

    void displayError(const char* error);

    int  args (int argc, const char *argv[]);
    bool open (void);
    void close(void);
    bool play (void);
    void stop (void);

    player_state_t state(void) const { return m_state; }
};

#endif // PLAYER_H
