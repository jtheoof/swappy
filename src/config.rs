use die::die;
use dirs::{config_dir, desktop_dir, home_dir};
use std::path::PathBuf;
use std::string::String;

fn get_default_save_dir() -> PathBuf {
    let storage_paths = vec![
        desktop_dir().unwrap_or_default(),
        config_dir().unwrap_or_default().join("Desktop"),
        home_dir().unwrap_or_default().join("Desktop"),
        home_dir().unwrap_or_default(),
    ];

    for pb in storage_paths.iter() {
        let path = pb.as_path();
        if path.is_dir() {
            return pb.to_path_buf();
        }
    }

    die!("unable to find a default save directory");
}

#[derive(Debug)]
pub struct Config {
    pub save_dir: PathBuf,
    pub save_filename_format: String,
    pub show_panel: bool,
    pub line_size: u32,
    pub text_size: u32,
    pub text_font: String,
    pub fill_shape: bool,
    pub early_exit: bool,
}

impl Config {
    pub fn new(config_file: &str) -> Config {
        Config {
            save_dir: get_default_save_dir(),
            save_filename_format: String::from("swappy-%Y%m%d_%H%M%S.png"),
            show_panel: false,
            line_size: 5,
            text_size: 20,
            text_font: String::from("sans-serif"),
            fill_shape: false,
            early_exit: false,
        }
    }

    pub fn load(&self) -> &Config {
        println!("config loaded: {:?}", self);
        return self;
    }
}
