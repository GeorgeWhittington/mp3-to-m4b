# mp3-to-m4b
Gui to convert mp3 files to m4b audiobooks

Ensure `adwaita-icon-theme` is installed to view properly.

## Installing dependencies:
Use the package manager of your choice (apt, brew, idk what windows folks have to suffer lol) and git+meson to install the following:
1. [avcpp](https://github.com/h4tr3d/avcpp) == commit fe8847aa740aa4c2868c020c82d6765c8520c1a1
2. gtkmm == v3.24.2

## Setting up Meson:
1. Install [Meson](https://mesonbuild.com/SimpleStart.html) and ensure it is in your PATH
2. Navigate to the root directory of this project
3. Run `meson setup build`

Once these steps are complete, you only need to use the command `meson compile` inside the build directory to build the application, even if the meson.build file changes. The executable will be called `mp3-to-m4b`.