# C64play

A music player for Commodore 64 tunes, based on [sidplayfp](https://github.com/libsidplayfp/sidplayfp).

### What I added

- Extra displays:
  - Master volume;
  - Active filters;
  - Filtered channels;
  - Filter resonance;
  - Filter cutoff;
  - With the exception of the 2nd and 3rd items, they're displayed in binary.
- Replay a tune by hitting the `r` key
- YouTube player-like keybinds
  - Hit `j` to go to the previous subtune;
  - `k` to pause a subtune;
  - and `l` to go to the next subtune.
- Use `g` to jump to a specific subtune instead of repeatedly hitting `j` or `k`.
- Other visual changes:
  - The console cursor is now hidden.
  - Prefix a `~` to the note if the frequency doesn't match one of
    the ones in the frequency table.

### What I removed

- Support for Windows
  - I'm thinking of making a new UI for that
  - Plus, as I'm a beginner, I needed to simplify things
- Display configuration file location
  - I didn't find any use for this, as there's no command-line option
    to load a custom configuration file and the file is always loaded
	from the same location...
- STILView
  - I never used that and don't wanna take care of something I never
    messed with...

### Requirements

- autotools
- libsidplayfp
- pod2man
- and a decent computer to better enjoy the emulation!

### Building

1. Create a directory named `build`. Not really a required step, it's just
to make things easier if doing a cleanup later.

2. Change to that directory.

3. Run `autoreconf -i ..`. This will generate the `configure` script,
which is necessary to our build.

4. Run `../configure`. If curious, you can check the options by running
`../configure --help` beforehand.

5. After configuring, run `make`. This is where the actual build begins.
You can add `-j` followed by the number of CPU cores you have to
accelerate that process.

6. After building, run `make install` as root if necessary.
7. You're set!! Enjoy your SID tunes by simply running `c64play <tune>`.
See the man page of C64play and its configuration file c64play.ini
for more details.
