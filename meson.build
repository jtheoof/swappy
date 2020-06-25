project(
	'swappy',
	'rust',
	version: '1.5.1',
	license: 'MIT',
	meson_version: '>=0.59.0',
  default_options: [ 'warning_level=2', ],
)

version = '"@0@"'.format(meson.project_version())
git = find_program('git', native: true, required: false)
if git.found()
	git_commit = run_command([git, 'rev-parse', '--short', 'HEAD'], check: true)
	git_branch = run_command([git, 'rev-parse', '--abbrev-ref', 'HEAD'], check: true)
	if git_commit.returncode() == 0 and git_branch.returncode() == 0
	  # TODO: Try to add __DATE__ back
		version = '"@0@-@1@ (branch \'@2@\')"'.format(
			meson.project_version(),
			git_commit.stdout().strip(),
			git_branch.stdout().strip(),
		)
	endif
endif

application_id = 'me.jtheoof.swappy'

cairo = dependency('cairo')
pango = dependency('pango')
gtk4 = dependency('gtk4', version: '>= 4.6.0')
gio = dependency('gio-2.0')

cargo = find_program('cargo', required: true)

cargo_sources = files(
  'Cargo.toml',
  'Cargo.lock',
)

subdir('res')
subdir('src')
subdir('src/po')

scdoc = find_program('scdoc', required: get_option('man-pages'))

if scdoc.found()
	sh = find_program('sh')

	man_pages = ['swappy.1.scd']

	mandir = get_option('mandir')

	foreach src : man_pages
		topic = src.split('.')[0]
		section = src.split('.')[1]
		output = '@0@.@1@'.format(topic, section)

		custom_target(
			output,
			input: src,
			output: output,
			command: [
				sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc.full_path(), output)
			],
			install: true,
			install_dir: '@0@/man@1@'.format(mandir, section)
		)
	endforeach
endif

gnome.post_install(
  glib_compile_schemas: true,
  gtk_update_icon_cache: true,
  update_desktop_database: true,
)

summary({
    'buildtype': get_option('buildtype'),
    'version': version,
  },
  section: 'Build',
)

summary({
    'prefix': get_option('prefix'),
    'libdir': get_option('libdir'),
    'datadir': get_option('datadir'),
    'bindir': get_option('bindir'),
  },
  section: 'Directories',
)
