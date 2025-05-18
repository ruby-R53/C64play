May 17/18, 2025:
- Trying to implement fade out for tunes!
  - You can leave a default on the configuration file or also
    set that fade out time in the command line. It's set in
    seconds, and a value of 0 disables it, in case you wanna
    play a non-looping tune.
    - That value of 0 is actually the default, too.

- Fix broken `-b`
  - The thing would throw a segfault whenever you specified
    the start time. It was the mixer's fault.

- Changes to the configuration file
  - The words are now capitalized following English
    capitalization rules.
  - Changed "version" to "model".

- This is currently what we'll get in version 1.1.0!

May 1st, 2025:
- The filter curve value for the 6581 chip still ranges from
  -2.0 to 2.0. However, for the 8580 it now ranges from 0.0
  to 1.0.
  - This is because anything beyond the 0.0-1.0 range breaks
    that chip's sound. So it's kinda useless to leave it that
	way.
  - I've actually wanting to do this for a while, but only now
    I figured out how to do so x)
- C64PLAY IS FINALLY AVAILABLE TO THE PUBLIC!!! VERSION 1.0.0
  IS HERE!!!!

April 28, 2025:
- C++17 is now the minimum version required for C64play to
  compile.
  - I mean, if the minimum version for the library is that of
    the year 2021 then why the hell would this be able to
	compile on compilers that still use the C++11 standard??

April 26, 2025:
- New player API support!
  - Not sure if I'll benefit much from it, but if it's new I
    guess it should be better, right?
- And actually, fuck the "properly clear audio buffer" thing. I
  just reverted that and it's now working properly.
  - I guess the code used for that really was targeted towards
    the new audio buffer thing in sidplayfp. Fun.

April 25, 2025:
- Properly clear audio buffer
  - I actually have been wanting to do that for quite a while,
    but only now I was able to figure out what the heck was
	wrong that didn't make it work like in sidplayfp. Turns out
	I had to hard code the 16-bit audio depth because it'd
	conflict with PulseAudio's setting (or at least that's my
	understanding of it). Nice.

April 24, 2025:
- I KEEP CHANGING MY MIND. I WANT MULTI-THREADING. I DON'T WANT
  MULTI-THREADING. AAAAAAAAAAAAAAA
  - I did get some progress this time. I had to split the
    `updateDisplay()` function in `updateTimer()` and the
	already existing `refreshRegDump()`. And that fixed the
	`-l0` and missing SLDB entry problems. However···
  - The damn display throws unprocessed data onto the screen.
    Could it be that my refresh rate was set way too fast?
	Well, I did notice that if a tune has a very high speed
	(like the ones that use Digi samples) the bug gets worse.
	Meanwhile, on normal 50 Hz VBI ones that bug is nearly
	non-existent. What the heck is going on???
  - Result: I won't implement multi-threading. FOR NOW :)
    Because of that, I'm making a new branch containing the
	multi-thread implementation, so that I don't need to keep
	commenting stuff out and making a mess on the code.

April 22, 2025:
- Meh, not implementing multi-threading for now.
  - I changed my mind because of the bugs below. I just
    couldn't figure out what the hell was wrong, and no
	debugger was able to help. The program simply crashed, but
	nothing meaningful was displayed on the trace whatsoever.
	Weirdly enough, it works just fine on sidplayfp, even on
	its latest code···
  - So yeah, the bugs below really were caused by the multi-
    thread implementation. I wonder why. I really wonder why.

April 21, 2025:
- Threaded display update (sorta) implemented! (VERY
  EXPERIMENTAL!)
  - This had actually been planned along with being able to
    change the buffer size in the configuration file. However,
	it turned out to be a disaster and now I'm taking smaller
	steps.
	I decided to implement the threaded display update first as
	every buffer write blocks the execution of the program, so
	it would consequently feel really janky and the thread
	creation would be useless.
  - Considering the above, the performance gain is promising,
    but at the same time underwhelming.
	At least now the display has a more consistent refresh
	rate compared to before (around 62 Hz), but that can still
	be further improved. I'm still a beginner, it's the first
	project I work on that has audio involved, so I can't
	really figure out how to implement the better-suited
	PulseAudio Asynchronous API here at the moment.
	I'll just wait to see if another simpler path could be
	taken here instead.
- NEW BUGS
  - Playing with `-l0` now crashes the program's audio
    handling.
  - Whenever a tune doesn't have an SLDB entry, its duration
    won't be infinite, and instead will be 82:47.295.
  - Those all appeared after I implemented the threaded display
    updates. I actually wonder if that's really what caused it
	to behave like that, or if it's some change I made a little
	after. Bummer x(

April 20, 2025:
- More command line option changes
  - `-f<num>` -> `-r<num>` ("sample *r*ate")
  - `-r<i|r>[f]` -> `-R<i|r>[f]` (as a consequence of the
    change above)
  - `-nf` -> `-f`. Instead of only disabling the filter
    emulation, it now flips it instead, so if filter emulation
	is already disabled in the settings file, that means this
	will enable it.
- Some data type changes. Not sure if it impacts performance,
  but it's still better to reserve the exact amount of space
  each variable should take.
- Happy 4/20!

April 19, 2025:
- Fixes and changes to the command line options
  - I've renamed some options to names that make a little
    more sense. Here's what I changed:
	- `--noaudio` -> `--no-audio` & `--nosid` -> `--no-sid`.
	  Those are rather aesthetical than to make more sense,
	  I mean, I literally just added a dash to them.
	- `--none` -> `--null` (both audio driver and SID chip
	  are nonexistent).
	- `-t` -> `-l` ("*l*ength").
	- `--fcurve` -> `--curve` & `--frange` -> `--range`. Those
	  are rather aesthetical than to make more sense, again. I
	  at least think it will still mean the same thing for
	  everyone ¯\\\_(ツ)_/¯
	- `-p` -> `-d` ("bit *d*epth").
  - The `--wav` option was removed in favor of the `-w` one.
    - I mean, the latter is much better to type. Easier to
	  handle as well. I've always used it. Pretty sure most
	  people who used sidplayfp have used it as well instead
	  of that long form.

April 18, 2025:
- Fix increase on CPU usage during pause
  - I have no idea what could have possibly caused that, but
    it got fixed by simply adding a .1-second delay to the
	condition check ¯\\\_(ツ)_/¯

April 16, 2025:
- Fixed Filter Curve display for CSG 8580 tunes
  - I put the same variable for the 6581 filter curve value xd
- C64play is now bundled with the C64's latest ROMs!
  - I think it's safe to include them here now. I mean, I got
    my (and this program's) ROMs from the internet, and I'm
	pretty sure plenty of other websites have the same files.
	Commodore doesn't even exist anymore, so how would they
	even wanna do something about it?
	I mean, even VICE which is a lot more popular than a lot
	of other tools bundles those ROMs, so why would anything
	happen to my rather insignificant project···?

April 10, 2025:
- Fixed filter cutoff display
  - The lower 8 bits are actually meant to be the higher ones.
    Apparently I misunderstood the SID's datasheet.
	So thank you for pointing that out, Jammer!!

April 6, 2025:
- C64play now ignores the `g` key if it's either invoked with
  `-os` or if the file only has one tune.
  - No idea why I decided to add that now. Ever since I
    implemented that key shortcut, that idea never, EVER,
	appeared in my head. Better late than never, I guess.

April 1, 2025:
- Not a joke, I had to fix OsciDump to work with relative
  paths···
  - I've always ignored that, only now I'm fixing it ¯\\\_(ツ)_/¯

March 22, 2025:
- Fixed WAV dumping when the SID model is forced
  - The program would ignore that setting and dump the
    individual channels on the tune-specified SID model.
	The code looks a little ugly, but hey, it works! I should
	be able to do it better later. For now, it's just a quick
	fix.
- Small cleanup
  - Removed redundant `cerr << dec;` on `menu.cpp`. Why the
    heck did I only notice that now?

February 12, 2025:
- Fixed (?) `oscidump_2SID.sh`
  - I literally just copy & pasted the code for the 1SID
    version of this script and missed some simple details.
	I didn't even bother actually including the 2nd sample
	channel dump :sob:

February 11, 2025:
- First update of the year!
- I was finally able to implement not one but 2 scripts for
  channel dumping, useful for oscilloscope views.
  - `oscidump.sh` is for single-SID tunes, and
    `oscidump_2SID.sh` is for 2SID ones. Maybe I should make
	one for 3SID as well, but this is enough for now.
	- Both of them are pretty straightforward to use - just put
	  the file name and then the same options you'd put on
	  C64play to play the tune!
	  - NOTE: sample dumping feature for `oscidump_2SID.sh` is
		untested! Let me know if there're any bugs :)
- Minor grammar fixes
  - In English, you have to capitalize every word in a title.
    So that's what I did for the displays, because they are
	titles in a way.
- Filter curve now ranges from -2.5 to 2.5
  - I got that based on how for reSID you have -5000 mV to
    +5000 mV, but that number is way too high, so I had to
	adapt it.
	Values over 1.0 and below 0.0 still break 8580 tunes, but
	it works fine for 6581 ones. I was annoyed I wasn't getting
	the sound I wanted with that previous range.
- That's it! I hope I can finally make the repository public
  soon this year. I still have to thoroughly test this program.

October 28, 2024:
- Working on implementing a script for dumping each channel of
  a tune. Handy for oscilloscope views, that's why I named it
  `oscidump.sh` :)

October 26/27, 2024:
- Fixed one extra detail on the Filter range/DigiBoost display
  - Damn, coding that was an awful quest. Hopefully that's the
    last time I have to fix something there. Well, I still need
    to optimize that code a little, it has some redundancy...
- Code cleanup
  - Replaced spaces with tabs!

September 30, 2024:
- Oh my friggin' GOD, I didn't notice a small error and pushed
  the code anyways. OOPS!
  - Welp, I rewrote that part of the code. It's still messy but
    now it *actually* works as intended.
- Changes in the layout:
  - I changed how the lines on the 4th block are displayed. It
    makes a bit more sense, you get the default SID model first
	and then its related parameters, not the other way around.
  - The `CIA model` field appears if verbosity level is 2 or
    higher.

September 29, 2024:
- Fixed a broken and messy code for checking the SID model to
  place the `Filter range` and `DigiBoost` fields.
  - It did work if the model was either forced or specified by
    the tune, not if it was `Any` or `Unknown`. I tried to add
	stuff to the existing code for those cases but only worsened
	the already messy code.

	I still have to clean some stuff up tho', but at least it
	works much better compared to that previous code block.
  - UPDATE (10:33 PM -03): **bold** of me to assume what I had
    done was actually working. It instead broke the tunes that
	specified the SID model, and fixed for the ones that didn't.

	Oops.

	At least now I think I managed to fix it. Had to add some
	mess to the code, but it's still better than what it had
	previously. I'll clean that up later, I promise!!

September 27, 2024:
- sidplayfp now supports setting the filter range setting on the
  command line, and so does C64play.
  - Which means, now the `Filter range` display also changes if
    that parameter was provided on the command line!

September 24, 2024:
- `CIA model` display implemented.

September 15, 2024:
- `CHANGELOG.md` exists
- The UI changes dynamically according to the number of SIDs from
  the tune.
  - The SID number on the `SID details` and filter-related fields
    now doesn't appear for those tunes, only if they use more chips.
- Sample muting implemented!!
  - Same implementation done in `sidplayfp`, except that I changed
    the command-line argument and also the keys.

September 14, 2024:
- There was an epically failed attempt at adding a buffer size option
  on the configuration file instead of keeping it hard-coded.
  - Gotta learn how PulseAudio works better now!

September 12 (?), 2024:
- The `Subtune` field now becomes `On loop?` if the file is a single
  tune.
