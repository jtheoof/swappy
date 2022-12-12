use std::string::String;

pub struct Config {
    save_dir: String,
    show_panel: bool,
    line_size: u32,
    text_size: u32,
    text_font: String,
    early_exit: bool,
    save_filename_format: String,
}

impl Config {
    pub fn new(config_file: &str) -> Config {
        Config {
            save_dir: String::from("test"),
            show_panel: true,
            line_size: 5,
            text_size: 20,
            text_font: String::from("sans-serif"),
            early_exit: false,
            save_filename_format: String::from("swappy-%Y%m%d_%H%M%S.png"),
        }
    }
}
