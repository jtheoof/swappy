use die::die;
use gio::{resources_register, ApplicationCommandLine, ApplicationFlags, Resource};
use glib::{Bytes, Char, OptionArg, OptionFlags};
use gtk::prelude::*;
use gtk::{Application, ApplicationWindow, Box, Builder, Button, DrawingArea};

use gtk::gio;
use gtk::glib;

use crate::config::Config;
use crate::constant::VERSION;

pub struct UiState {
    panel_displayed: bool,
}

pub struct State {
    config: Config,
    ui: UiState,
}

impl UiState {
    pub fn new() -> UiState {
        UiState {
            panel_displayed: false,
        }
    }
}

impl State {
    pub fn new() -> State {
        State {
            config: Config::new("test"),
            ui: UiState::new(),
        }
    }
}

fn build_ui(app: &Application) {
    println!("building ui");
    let res_bytes = include_bytes!("ui/swappy.gresource");
    let data = Bytes::from(&res_bytes[..]);
    let resource = Resource::from_data(&data).unwrap();
    resources_register(&resource);

    let builder = Builder::from_resource("/me/jtheoof/swappy/swappy.ui");

    let window: ApplicationWindow = builder
        .object("paint-window")
        .expect("could not find paint-window in ui file");

    let side_panel: Box = builder
        .object("side-panel")
        .expect("could not find side-panel in ui file");

    let paint_area: DrawingArea = builder
        .object("painting-area")
        .expect("could not find painting-area in ui file");

    let toggle_button: Button = builder
        .object("btn-toggle-panel")
        .expect("could not find btn-toggle-panel in ui file");

    toggle_button.connect_clicked(move |_| {
        println!("button clicked");
        side_panel.set_property("visible", true);
    });

    paint_area.connect_resize(move |_, w, h| {
        println!("paint-area resized {}x{}", w, h);
    });

    window.set_application(Some(app));
    window.present();

    println!("ok window is shown");
}

fn on_handle_local_options(_app: &Application, options: &glib::VariantDict) -> i32 {
    if options.contains("version") {
        println!("swappy version {}", VERSION);
        return 0;
    }
    let maybe_file = options.lookup_value("file", None);
    match maybe_file {
        None => die!("no geometry found, did you use -f option?"),
        Some(file) => {
            println!("file is {}", file);
        }
    }
    -1
}

fn on_command_line_connected(app: &Application, _: &ApplicationCommandLine) -> i32 {
    println!("'command-line' called");
    build_ui(app);
    0
}

pub fn init(state: State) {
    let app = Application::new(
        Some("me.jtheoof.swappy"),
        ApplicationFlags::HANDLES_OPEN | ApplicationFlags::HANDLES_COMMAND_LINE,
    );

    app.add_main_option(
        "version",
        Char::from(b'v'),
        OptionFlags::NONE,
        OptionArg::None,
        "Print version and quit",
        None,
    );

    app.add_main_option(
        "file",
        Char::from(b'f'),
        OptionFlags::NONE,
        OptionArg::String,
        "Load a file at a specific path",
        None,
    );

    app.add_main_option(
        "output-file",
        Char::from(b'o'),
        OptionFlags::NONE,
        OptionArg::String,
        "Print the final surface to the given file when exiting, use - to print to stdout",
        None,
    );

    app.connect_handle_local_options(on_handle_local_options);
    app.connect_command_line(on_command_line_connected);

    app.run();
}
