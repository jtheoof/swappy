mod config;
mod constant;
mod paint;
mod swappy_gtk;

use swappy_gtk::State;

extern crate pretty_env_logger;

fn main() {
    pretty_env_logger::init();
    let mut state = State::new();

    println!("{:?}", state.config);

    swappy_gtk::init(state);
}
