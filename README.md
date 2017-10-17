# TIC-80 CL fork notes

This fork makes it possible to use Common Lisp with TIC-80.

It's another late-night hack, and therefore bound to be riddled with
bugs and whatnot.

Platforms other than Linux 64 are left for dead.

There's not much error checking, and I opted to treat most TIC-80 API
arguments as optional.

It doesn't try to limit functionality of the embedded Lisp, and
therefore not suitable for running untrusted code.

It is my first attempt at embedding
[ECL](https://common-lisp.net/project/ecl/), and therefore likely
contain a bunch of WTFs.

Example WTF: I've no idea how to reset the environment, apparently
`cl_shutdown` is not enough.

Example WTF: I've no idea how to statically link ECL, so have to do
with the shared library.

ECL has been compiled with the following configuration:

```
./configure --enable-threads=no --with-tcp=no --with-serve-event=no --with-asdf=no \
            --with-defsystem=no --with-rt=no --with-profile=no --with-dffi=no
```

After compiling TIC-80, you can run it from the `bin` directory using:

```
LD_PRELOAD=../lib/linux/libecl.so ./tic
```

Then use `new cl` to create a CL-based cart.

# TIC-80 issues tracker and wiki

[Discord chat](https://discord.gg/DkD73dP)

This is the official issues tracker of <https://tic.computer>. If you are experiencing a bug or would like to see a new feature [browse existing issues](https://github.com/nesbox/tic.computer/issues) or [create a new one](https://github.com/nesbox/tic.computer/issues/new).

Documentation is available in the [wiki](https://github.com/nesbox/tic.computer/wiki).

Thanks!

# Build instructions

## Windows
### with Visual Studio 2015
- install Visual Studio 2015
- install GIT
- run following commands in `cmd`
```
mkdir tic
cd tic
git clone https://github.com/nesbox/3rd-party
git clone https://github.com/nesbox/TIC-80
```
- open `TIC-80\build\windows\tic\tic.sln` and build
- enjoy :)

### with MinGW32
follow the instructions in the tutorial https://matheuslessarodrigues.github.io/tic80-build-tutorial/
made by [@matheuslessarodrigues](https://github.com/matheuslessarodrigues)

## Linux
run the following commands in the Terminal
```
sudo apt-get install git build-essential libgtk-3-dev
git clone https://github.com/nesbox/TIC-80
cd TIC-80
make linux32 (or linux64/arm depending on your system)
```

## iOS / tvOS
You can find iOS/tvOS version here https://github.com/CliffsDover/TIC-80
