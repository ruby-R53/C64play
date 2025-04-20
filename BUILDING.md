### Build requirements

- autotools
- libsidplayfp (version 2.2 or higher)
- pod2man
- patience
- and a decent computer to better enjoy the emulation!

### The build itself

1. Create a directory named `build`. Not really a required step, it's just
for organization.

2. Change to that directory.

3. Run `autoreconf -i ..`. This will generate the `configure` script,
which is necessary to our build.

4. Run `../configure`. If curious, you can check the options by running
`../configure --help` beforehand.

5. After configuring, run `make`. This is where the actual build begins.
You can add `-j` followed by the number of CPU threads you have to make that
process faster.

6. After building, run `make install` as root if necessary.

7. You're set!! Enjoy your SID tunes by simply running `c64play [options] <tune>`.

See the man page of C64play and its configuration file `c64play.ini` for
more details.
