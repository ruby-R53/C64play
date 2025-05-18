### Build requirements

- autotools - configure script creation
- libsidplayfp version >=2.2 - the emulation library itself
- pod2man - documentation
- patience
- and a decent computer to better enjoy the emulation!

### The build itself

1. Create a directory named `build`. Not really a required step, it's just
for organization.

2. Go to that directory.

3. Run `autoreconf -i ..`. This will generate the `configure` script,
which is necessary to our build.

4. Run `../configure`. If curious, you can check the options by running
`../configure --help` beforehand.

5. After configuring, run `make`. This is where the actual build begins.
You can also add `-j` followed by the number of CPU threads you have to
speed that process up!

6. After building, run `make install` as root if necessary.

7. You're set!! Enjoy your SID tunes by simply running
`c64play [options] <tune>`.

See the man page of C64play and its configuration file `c64play.ini` for
more details.
