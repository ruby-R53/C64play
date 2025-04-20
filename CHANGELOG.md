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
