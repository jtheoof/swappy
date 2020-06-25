use std::env;
use std::process::Command;

fn compile_resource() {
    println!("hello build");
    for (key, val) in env::vars() {
        println!("{}: {}", key, val);
    }

    Command::new("sh")
        .args(&[
            "-c",
            "cd src/ui && glib-compile-resources swappy.gresource.xml",
        ])
        .output()
        .expect("failed to compile src/ui/swappy.gresource.xml");
}

fn main() {
    compile_resource();
}
