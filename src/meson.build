pkgdatadir = get_option('prefix') / get_option('datadir') / meson.project_name()

conf = configuration_data()
conf.set_quoted('VERSION', '@0@'.format(version))
conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))
conf.set_quoted('PKGDATADIR', pkgdatadir)
conf.set_quoted('APPLICATION_ID', meson.project_name())

configure_file(
  input: 'constant.rs.in',
  output: 'constant.rs',
  configuration: conf
)

# Copy the config.rs output to the source directory.
run_command(
  'cp',
  join_paths(meson.project_build_root(), 'src', 'constant.rs'),
  join_paths(meson.project_source_root(), 'src', 'constant.rs'),
  check: true,
)

rust_sources = files(
  'main.rs',
  'constant.rs',
  'config.rs',
  'swappy.rs',
)

sources = [cargo_sources, rust_sources]

cargo_script = find_program(meson.project_source_root() / 'build-aux/cargo.sh')
cargo_release = custom_target(
  'cargo-build',
  build_by_default: true,
  input: sources,
  output: meson.project_name(),
  console: true,
  install: true,
  install_dir: get_option('bindir'),
  command: [
    cargo_script,
    meson.project_build_root(),
    meson.project_source_root(),
    '@OUTPUT@',
    get_option('buildtype'),
    meson.project_name(),
  ],
)
