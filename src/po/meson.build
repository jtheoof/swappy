i18n = import('i18n')

# define GETTEXT_PACKAGE
add_project_arguments('-DGETTEXT_PACKAGE="swappy"', language:'c')

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
