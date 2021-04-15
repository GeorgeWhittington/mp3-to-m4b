# mp3-to-m4b
Gtk gui to convert mp3 files to m4b audiobooks
(This is a project for university, so I'm distributing no binaries, and won't be updating any of the code.)

## Installing dependencies:
Use the package manager of your choice (apt, brew, msys2's thing) and git+meson to install the following:
1. [avcpp](https://github.com/h4tr3d/avcpp) == commit fe8847aa740aa4c2868c020c82d6765c8520c1a1
2. gtkmm == v3.24.2
3. adwaita-icon-theme == v3.38.0
4. Boost == v1.75.0

The program also needs to access two command line tools which you will need to install. They are:
1. ffmpeg
2. AtomicParsley

By default the application will search the `/usr/local/bin` directory for these executables, if you have these installed elsewhere, you can override where the program searches for them by launching with this argument: `--bin_path <absolute path to binaries>`.

eg:
```
./mp3-to-m4b --bin_path "/home/username/my binaries"
```

## Setting up Meson:
1. Install [Meson](https://mesonbuild.com/SimpleStart.html) and ensure it is in your PATH
2. Navigate to the root directory of this project
3. Create a build directory (`mkdir build`) and navigate inside it (`cd build`)
4. Run `meson ..` to initialise meson
5. Open the file meson.build and adjust this keyword argument near the bottom: `link_args: ['/usr/local/Cellar/boost/1.75.0_2/lib/libboost_filesystem.dylib']`, you will need to locate where the boost filesystem dylib is installed on your system and put that path here. Unlike the other dependencies in this project, meson has trouble locating boost's dylibs on it's own.
6. Run `meson compile` to compile the project

Once these steps are complete, you only need to use the command `meson compile` inside the build directory to build the application, even if the meson.build file changes. The executable will be called `mp3-to-m4b`.
