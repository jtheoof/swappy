project(
	'swappy',
	'c',
	version: '1.0.0',
	license: 'MIT',
	meson_version: '>=0.48.0',
	default_options: [
		'c_std=c11',
		'warning_level=2',
		'werror=true',
	],
)

add_project_arguments('-Wno-unused-parameter', language: 'c')
#add_project_arguments('-Wno-format-truncation', language: 'c')

swappy_inc = include_directories('include')

cc = meson.get_compiler('c')

cairo = dependency('cairo')
math = cc.find_library('m')
realtime = cc.find_library('rt')
gtk = dependency('gtk+-wayland-3.0', version: '>=3.22.0')
gtk_layer_shell = dependency('gtk-layer-shell-0', version: '>= 0.1.0')
libnotify = dependency('libnotify')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_protos = dependency('wayland-protocols', version: '>=1.14')

subdir('res')
subdir('protocol')

executable(
	'swappy',
	files([
		'src/main.c',
		'src/notification.c',
		'src/box.c',
		'src/config.c',
		'src/clipboard.c',
		'src/render.c',
		'src/screencopy.c',
		'src/wayland.c',
		'src/application.c',
	]),
	dependencies: [
		cairo,
		client_protos,
		gtk,
		gtk_layer_shell,
		libnotify,
		math,
		realtime,
		wayland_client,
		wayland_cursor,
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
