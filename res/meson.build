pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())

# Import the gnome module and use a GNOME function to ensure that application
# resources will be compiled.
gnome = import('gnome')

gnome.compile_resources('swappy',
  'swappy.gresource.xml',
  gresource_bundle: true,
  install: true,
  install_dir: pkgdatadir,
)