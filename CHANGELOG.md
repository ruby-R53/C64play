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
