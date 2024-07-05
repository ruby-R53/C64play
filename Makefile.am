AUTOMAKE_OPTIONS = foreign subdir-objects

if USE_LIBOUT123
  OUT123_SOURCES = \
src/audio/out123/audiodrv.cpp \
src/audio/out123/audiodrv.h
endif

if USE_ICONV
  ICONV_SOURCES = \
src/codepages.h
endif

AM_CPPFLAGS = \
-I $(top_builddir)/src \
-I $(top_srcdir)/src \
-DPKGDATADIR="\"$(pkgdatadir)/\"" \
$(C64PLAY_CFLAGS) \
$(ALSA_CFLAGS) \
$(PULSE_CFLAGS) \
$(OUT123_CFLAGS) \
@debug_flags@


bin_PROGRAMS = \
src/c64play

#=========================================================
# C64play

src_c64play_SOURCES = \
src/IniConfig.cpp \
src/IniConfig.h \
src/args.cpp \
src/keyboard.cpp \
src/keyboard.h \
src/main.cpp \
src/menu.cpp \
src/player.cpp \
src/player.h \
src/sidcxx11.h \
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
src/audio/au/auFile.cpp \
src/audio/au/auFile.h \
src/audio/null/null.cpp \
src/audio/null/null.h \
src/audio/oss/audiodrv.cpp \
src/audio/oss/audiodrv.h \
$(OUT123_SOURCES) \
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
$(PULSE_LIBS) \
$(OUT123_LIBS)

#=========================================================
# docs

EXTRA_DIST = \
doc/c64play.pod \
doc/c64play.ini.pod

dist_man_MANS = \
doc/c64play.1 \
doc/c64play.ini.5

DISTCLEANFILES = $(dist_man_MANS)

.pod.1:
	@mkdir -p $(@D)
	pod2man -c "User Programs" -s 1 $< > $@

.pod.5:
	@mkdir -p $(@D)
	pod2man -c "File formats" -s 5 $< > $@
