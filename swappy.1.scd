swappy(1)

# NAME

swappy - grab and edit on the fly snapshots of a Wayland compositor, inspired by Snappy on macOS

# DESCRIPTION

*swappy* [options...]

# SYNOPSIS

swappy is a command-line utility to take and edit screenshots of Wayland desktops. For now
it requires support for the screencopy protocol to work. Support for the
xdg-output protocol is optional, but improves fractional scaling support.

swappy will save the swappshot images to a timestamped file name in
*$XDG_DESKTOP_DIR*. If this variable is not set, it will revert to: *$XDG_CONFIG_HOME/Desktop*.
If *$XDG_CONFIG_HOME* is not set, it will revert to: *$HOME/Desktop*.

# OPTIONS

*-h*
	Show help message and quit.

*-g* "<x>,<y> <width>x<height>"
	Set the region to capture, in layout coordinates.

	This is slurp friendly.

# AUTHORS

Written and maintained by Jeremy Attali <jeremy.attali@gmail.com>. See
https://github.com/jtheoof/swappy.
