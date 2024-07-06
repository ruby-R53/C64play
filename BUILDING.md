### Building

This program currently uses autotools for its building process. To build and
install it, do the following:

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
