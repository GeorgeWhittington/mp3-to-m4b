# mp3-to-m4b
Gui to convert mp3 files to m4b audiobooks

## requirements for pkg-config:
1. gtkmm-3.0
2. libavcodec
3. libavformat
4. libavfilter
5. libavutil

## gcc command if you're being lazy
Something along the lines of:
``g++ -o main main.cc converter_application.cc converter_treestore.cc converter_window.cc get_duration.c `pkg-config --cflags --libs gtkmm-3.0 libavcodec libavformat libavfilter libavutil` -std=c++11``
