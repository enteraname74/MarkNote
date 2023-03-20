marknote_sources = [
  'main.c',
  'marknote-application.c',
  'marknote-window.c',
  'marknote-file-list.c',
]

marknote_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1'),
]

marknote_sources += gnome.compile_resources('marknote-resources',
  'marknote.gresource.xml',
  c_name: 'marknote'
)

executable('marknote', marknote_sources,
  dependencies: marknote_deps,
       install: true,
)