i18n = import('i18n')

i18n.gettext(meson.project_name(),
    args: '--directory=' + meson.project_source_root()
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
