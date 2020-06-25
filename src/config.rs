use std::string::String;

pub struct Config {
    save_dir: String,
    show_panel: bool,
    line_size: u32,
    text_size: u32,
    text_font: String,
}

impl Config {
    pub fn new(config_file: &str) -> Config {
        Config {
            save_dir: String::from("test"),
            show_panel: true,
            line_size: 5,
            text_size: 10,
            text_font: String::from("monospace"),
        }
    }
}
