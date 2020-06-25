project(
	'swappy',
	'c',
	version: '1.1.0',
	license: 'MIT',
	meson_version: '>=0.48.0',
	default_options: [
		'c_std=c11',
		'warning_level=2',
		'werror=true',
	],
)

version = '"@0@"'.format(meson.project_version())
git = find_program('git', native: true, required: false)
if git.found()
	git_commit = run_command([git, 'rev-parse', '--short', 'HEAD'])
	git_branch = run_command([git, 'rev-parse', '--abbrev-ref', 'HEAD'])
	if git_commit.returncode() == 0 and git_branch.returncode() == 0
		version = '"@0@-@1@ (" __DATE__ ", branch \'@2@\')"'.format(
			meson.project_version(),
			git_commit.stdout().strip(),
			git_branch.stdout().strip(),
		)
	endif
endif
add_project_arguments('-DSWAPPY_VERSION=@0@'.format(version), language: 'c')

add_project_arguments('-Wno-unused-parameter', language: 'c')

swappy_inc = include_directories('include')

cc = meson.get_compiler('c')

if cc.get_id() == 'clang'
    message('clang')
    add_global_arguments('-Wno-missing-field-initializers', language: 'c')
endif

cairo = dependency('cairo')
pango = dependency('pango')
math = cc.find_library('m')
realtime = cc.find_library('rt')
gtk = dependency('gtk+-3.0', version: '>=3.20.0')
gio = dependency('gio-2.0')

libnotify = dependency('libnotify', required: false)

if libnotify.found()
	add_project_arguments('-DHAVE_LIBNOTIFY', language: 'c')
endif

subdir('res')
subdir('src/po')

executable(
	'swappy',
	swappy_resources,
	files([
		'src/main.c',
		'src/algebra.c',
		'src/application.c',
		'src/buffer.c',
		'src/box.c',
		'src/config.c',
		'src/clipboard.c',
		'src/file.c',
		'src/paint.c',
		'src/pixbuf.c',
		'src/render.c',
		'src/notification.c',
		'src/util.c',
	]),
	dependencies: [
		cairo,
		pango,
		gio,
		gtk,
		libnotify,
		math,
		realtime,
	],
	link_args: '-rdynamic',
	include_directories: [swappy_inc],
	install: true,
)

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
				sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc.path(), output)
			],
			install: true,
			install_dir: '@0@/man@1@'.format(mandir, section)
		)
	endforeach
endif
