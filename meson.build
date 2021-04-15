project('mp3-to-m4b', 'cpp', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

deps = [
  dependency('gtkmm-3.0'), dependency('libavcodec'),
  dependency('libavformat'), dependency('libavfilter'),
  dependency('libavutil')]

gnome = import('gnome')

resources = gnome.compile_resources(
  'resources', 'resources/resources.xml',
  source_dir: 'resources')

executable('mp3-to-m4b',
           'main.cc',
           'get_duration.c',
           'converter_window.cc',
           'converter_treestore.cc',
           'converter_application.cc',
           resources,
           install: false,
           dependencies: deps)
