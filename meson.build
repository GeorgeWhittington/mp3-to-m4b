project('mp3-to-m4b', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

deps = [
  dependency('gtkmm-3.0'), dependency('avcpp')]

gnome = import('gnome')

resources = gnome.compile_resources(
  'resources', 'resources/resources.xml',
  source_dir: 'resources')

executable('mp3-to-m4b',
           'src/main.cc',
           'src/get_duration.cc',
           'src/converter_window.cc',
           'src/converter_treestore.cc',
           'src/converter_application.cc',
           resources,
           install: false,
           dependencies: deps,
           cpp_args: ['-x', 'c++'])
