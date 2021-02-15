# Import the gnome module and use a GNOME function to ensure that application
# resources will be compiled.
gnome = import('gnome')

# Icons
install_subdir('icons',
  install_dir: join_paths(get_option('datadir')),
)

swappy_resources = gnome.compile_resources('swappy',
  'swappy.gresource.xml'
)
