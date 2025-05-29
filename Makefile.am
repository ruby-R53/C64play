ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CPPFLAGS = \
-I $(top_builddir)/src \
-I $(top_srcdir)/src \
-DPKGDATADIR="\"$(pkgdatadir)/\"" \
$(C64PLAY_CFLAGS) \
$(ALSA_CFLAGS) \
$(PULSE_CFLAGS) \
@debug_flags@


bin_PROGRAMS = \
src/c64play

#=========================================================
# C64play source files to be included in our tarball

src_c64play_SOURCES = \
src/IniConfig.cpp \
src/IniConfig.h \
src/args.cpp \
src/keyboard.cpp \
src/keyboard.h \
src/main.cpp \
src/menu.cpp \
src/mixer.cpp \
src/mixer.h \
src/player.cpp \
src/player.h \
src/sidcxx.h \
src/uidefs.h \
src/sidlib_features.h \
src/utils.cpp \
src/utils.h \
src/codeConvert.cpp \
src/codeConvert.h \
$(ICONV_SOURCES) \
src/audio/AudioBase.h \
src/audio/AudioConfig.h \
src/audio/AudioDrv.cpp \
src/audio/AudioDrv.h \
src/audio/IAudio.h \
src/audio/alsa/audiodrv.cpp \
src/audio/alsa/audiodrv.h \
src/audio/null/null.cpp \
src/audio/null/null.h \
src/audio/oss/audiodrv.cpp \
src/audio/oss/audiodrv.h \
src/audio/pulse/audiodrv.cpp \
src/audio/pulse/audiodrv.h \
src/audio/wav/WavFile.cpp \
src/audio/wav/WavFile.h \
src/ini/iniHandler.h \
src/ini/iniHandler.cpp \
src/ini/dataParser.h \
src/ini/sidfstream.h \
src/ini/types.h

src_c64play_LDADD = \
$(LIBICONV) \
$(AUDIO_LDFLAGS) \
$(SIDPLAYFP_LIBS) \
$(BUILDERS_LDFLAGS) \
$(ALSA_LIBS) \
$(PULSE_LIBS)

#=========================================================
# Documentation, ROMs and OsciDump

EXTRA_DIST = \
doc/c64play.pod \
doc/c64play.ini.pod \
roms/basic.901226-01.bin \
roms/characters.901225-01.bin \
roms/kernal.901227-03.bin \
oscidump.sh \
oscidump_2SID.sh

dist_man_MANS = \
doc/c64play.1 \
doc/c64play.ini.5

DISTCLEANFILES = $(dist_man_MANS)

.pod.1:
	@mkdir -p $(@D)
	pod2man -c "User programs" -s 1 $< > $@

.pod.5:
	@mkdir -p $(@D)
	pod2man -c "File formats" -s 5 $< > $@
