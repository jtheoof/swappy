mod config;
mod constant;
mod swappy_gtk;

use swappy_gtk::State;

fn main() {
    let mut state = State::new();

    state.config.load();
    swappy_gtk::init(state);
}
