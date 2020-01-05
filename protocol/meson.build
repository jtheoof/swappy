wayland_scanner = find_program('wayland-scanner')

# should check wayland_scanner's version, but it is hard to get
if wayland_client.version().version_compare('>=1.14.91')
	code_type = 'private-code'
else
	code_type = 'code'
endif

wayland_scanner_code = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.c',
	arguments: [code_type, '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
	wayland_scanner,
	output: '@BASENAME@-client-protocol.h',
	arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

if wayland_protos.found()
	wl_protocol_dir = wayland_protos.get_pkgconfig_variable('pkgdatadir')
	client_protocols = [
		[wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
		[wl_protocol_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'],
		['wlr-data-control-unstable-v1.xml'],
		['wlr-screencopy-unstable-v1.xml'],
	]
else
	client_protocols = [
		['wlr-data-control-unstable-v1.xml'],
		['wlr-screencopy-unstable-v1.xml'],
	]
endif


client_protos_src = []
client_protos_headers = []

foreach p : client_protocols
	xml = join_paths(p)
	client_protos_src += wayland_scanner_code.process(xml)
	client_protos_headers += wayland_scanner_client.process(xml)
endforeach

lib_client_protos = static_library(
	'client_protos',
	client_protos_src + client_protos_headers,
	dependencies: [wayland_client]
) # for the include directory

client_protos = declare_dependency(
	link_with: lib_client_protos,
	sources: client_protos_headers,
)
