# swappy

A Wayland native snapshot and editor tool, inspired by [Snappy] on macOS. Works great with [slurp] and [sway].

Wayland code was largely taken from [grim].

## Screenshot

![Swappy Screenshot](docs/images/screenshot.png) 


## Example usage

Swappshot a region:

```sh
swappy -g "100,100 200x200"
```

Select a region and swappshot it:

```sh
swappy -g "$(slurp)"
```

Grab a swappshot from a specific window under Sway, using `swaymsg` and `jq`:

```sh
swappy -g "$(swaymsg -t get_tree | jq -r '.. | select(.pid? and .visible?) | .rect | "\(.x),\(.y) \(.width)x\(.height)"' | slurp)"
```

## Keyboard Shortcuts

* `Ctrl+b`: Toggle Paint Panel

<hr>

* `b`: Switch to Brush
* `t`: Switch to Text
* `r`: Switch to Rectangle
* `o`: Switch to Ellipse
* `a`: Switch to Arrow

<hr>

* `R`: Use Red Color
* `G`: Use Green Color
* `B`: Use Blue Color
* `C`: Use Custom Color
* `Minus`: Reduce Stroke Size
* `Plus`: Increase Stroke Size
* `Equal`: Reset Stroke Size
* `k`: Clear Paints (cannot be undone)

<hr>

* `Ctrl+z`: Undo
* `Ctrl+Shift+z` or `Ctrl+y`: Redo
* `Ctrl+s`: Save to file (see man page)
* `Ctrl+c`: Copy to clipboard
* `Escape` or `q`: Quit swappy

## Limitations

* **Mutli-Monitor**: I don't have a multi-monitor setup at home. Most likely it won't work properly. Pull requests are welcome.
* **Copy**: Copy to clipboard won't work if you close swappy (the content of the clipboard is lost). This because GTK 3.24 [has not implemented persistent storage](https://gitlab.gnome.org/GNOME/gtk/blob/3.24.13/gdk/wayland/gdkdisplay-wayland.c#L857). We need to do it on the [Wayland level](https://github.com/swaywm/wlr-protocols/blob/master/unstable/wlr-data-control-unstable-v1.xml), or wait for GTK 4.

## Installation

### Arch Linux User Repository

Assuming [yay](https://aur.archlinux.org/packages/yay/) as your AUR package manager:

* **stable**: `yay -S swappy`
* **latest**: `yay -S swappy-git`

## Building from source

Install dependencies:

* meson
* wayland
* wayland-protocols
* cairo
* gtk

Optional dependencies:

* libnotify

Then run:

```sh
meson build
ninja -C build
```

## Contributing

Pull requests are welcome.

## License

MIT

[Snappy]: http://snappy-app.com/
[slurp]: https://github.com/emersion/slurp
[grim]: https://github.com/emersion/grim
[sway]: https://github.com/swaywm/sway
