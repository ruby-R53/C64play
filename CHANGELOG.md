September 29, 2024:
- Fixed a broken and messy code for checking the SID model to
  place the `Filter range` and `DigiBoost` fields.
  - It did work if the model was either forced or specified by
    the tune, not if it was `Any` or `Unknown`. I tried to add
	stuff to the existing code for those cases but only worsened
	the already messy code.
	I still have to clean some stuff up tho', but at least it
	works much better compared to that previous code block.

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
