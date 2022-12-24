use dirs::{config_dir, desktop_dir, home_dir};
use ini::Ini;
use log::{debug, warn};
use std::path::{Path, PathBuf};
use std::string::String;

use crate::paint::PaintMode;

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
            debug!("found default save path at: {:?}", path.to_str());
            return pb.to_path_buf();
        }
    }

    warn!("unable to find a default save path, make sure \"save_dir\" is properly set in your swappy config");
    return PathBuf::default();
}

#[derive(Debug, Clone)]
pub struct Config {
    pub save_dir: PathBuf,
    pub save_filename_format: String,
    pub line_size: u32,
    pub text_font: String,
    pub text_size: u32,
    pub show_panel: bool,
    pub paint_mode: PaintMode,
    pub early_exit: bool,
    pub fill_shape: bool,
}

impl Default for Config {
    fn default() -> Self {
        Config {
            save_dir: get_default_save_dir(),
            save_filename_format: String::from("swappy-%Y%m%d_%H%M%S.png"),
            line_size: 5,
            text_font: String::from("sans-serif"),
            text_size: 20,
            show_panel: false,
            paint_mode: PaintMode::Brush,
            early_exit: false,
            fill_shape: false,
        }
    }
}

impl Config {
    pub fn new(_config_file: &str) -> Config {
        load_config_file()
    }
}

fn load_config_file() -> Config {
    let mut config = Config::default();
    let config_file = config_dir().unwrap_or_default().join("swappy/config");

    debug!("looking for config file at: {:?}", config_file);
    let ini = Ini::load_from_file(config_file);

    ini.map_or(config.clone(), |i| {
        debug!("found swappy config file, parsing now");
        for (sec, prop) in i.iter() {
            println!("Section: {:?}", sec);
            if sec.unwrap_or_default().eq("Default") {
                debug!("in General section");
                for (k, v) in prop.iter() {
                    println!("{}:{}", k, v);
                    match k {
                        "save_dir" => config.save_dir = Path::new(v).to_path_buf(),
                        "save_filename_format" => config.save_filename_format = String::from(v),
                        "line_size" => config.line_size = v.parse::<u32>().unwrap(),
                        "text_font" => config.text_font = String::from(v),
                        "text_size" => config.text_size = v.parse::<u32>().unwrap(),
                        "show_panel" => config.show_panel = v.parse::<bool>().unwrap(),
                        "paint_mode" => {
                            config.paint_mode = v.parse::<PaintMode>().unwrap_or_else(|_| {
                                warn!("paint_mode is not a valid value: {} - see man page for details", v);
                                config.paint_mode
                            })
                        },
                        "early_exit" => config.early_exit = v.parse::<bool>().unwrap_or_else(|_| {
                                warn!("early_exit is not a valid value: {} - see man page for details", v);
                                config.early_exit

                        }),
                        _ => (),
                    }
                }
            }
        }

        config
    })
}
