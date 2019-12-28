# swappy

A Wayland native snapshot tool, inspired by Snappy on macOS. Works great with [slurp] and [sway].

Code was largely inspired by [grim].

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

## Package manager installation

### Arch Linux User Repository

Assuming [yay](https://aur.archlinux.org/packages/yay/) as your AUR package manager.

* stable version: `yay -S swappy`
* git version: `yay -S swappy-git`

## Building from source

Install dependencies:

* meson
* wayland
* cairo
* gtk

Optional dependencies:

* libnotify

Then run:

```sh
meson build
ninja -C build
```

To run directly, use `build/swappy`, or if you would like to do a system
installation (in `/usr/local` by default), run `ninja -C build install`.

## Contributing

Pull requests are welcome.

## License

MIT

[slurp]: https://github.com/emersion/slurp
[grim]: https://github.com/emersion/grim
[sway]: https://github.com/swaywm/sway
