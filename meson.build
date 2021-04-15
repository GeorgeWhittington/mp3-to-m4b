project('mp3-to-m4b', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

deps = [
  dependency('gtk+-3.0'), dependency('gtkmm-3.0'),
  dependency('avcpp'), dependency('boost'),
  dependency('gdk-3.0')
]

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
           'src/conversion_dialog.cc',
           'src/conversion_worker.cc',
           'src/conversion_data.cc',
           resources,
           install: false,
           dependencies: deps,
           cpp_args: ['-x', 'c++'],
           link_args: ['/usr/local/Cellar/boost/1.75.0_2/lib/libboost_filesystem.dylib'])
