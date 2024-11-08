i18n = import('i18n')

conf = configuration_data()
# define GETTEXT_PACKAGE
conf.set_quoted('GETTEXT_PACKAGE',  meson.project_name())
conf.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))

config_h = configure_file(
        output : 'generated_config.h',
        configuration : conf)
add_project_arguments('-include', 'src/po/generated_config.h', language : 'c')

i18n.gettext(meson.project_name(),
    args: '--directory=' + meson.source_root()
)

# Translate and install our .desktop file
i18n.merge_file(
    input: meson.project_name() + '.desktop.in',
    output: meson.project_name() + '.desktop',
    po_dir: meson.current_source_dir(),
    type: 'desktop',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications')
)
