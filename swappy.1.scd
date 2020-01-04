swappy(1)

# NAME

swappy - grab and edit on the fly snapshots of a Wayland compositor

# DESCRIPTION

*swappy* [options...]

# SYNOPSIS

swappy is a command-line utility to take and edit screenshots of Wayland
desktops. It can also work on regular X11 desktops if using the *-f* option.

swappy will save the swappshot images to a timestamped file name in
*$XDG_DESKTOP_DIR*. If this variable is not set, it will revert to:
*$XDG_CONFIG_HOME/Desktop*. If *$XDG_CONFIG_HOME* is not set, it will revert
to: *$HOME/Desktop*.

Can be used in two ways, either as the output of grim (recommended) or by
grabbing the geometry (if the compositor supports the screencopy protocol).

# OPTIONS

*-h*
	Show help message and quit.

*-g* "<x>,<y> <width>x<height>"
	Set the region to capture, in layout coordinates. This is slurp friendly.

	Requires a Wayland compositor that supports screencopy protocol.

*-f* <file>
	A PNG file to load for editing.

	If set to *-*, read the file from standard input instead. This is grim
	friendly.

# KEY BINDINGS

## LAYOUT

- *Ctrl+b*: Toggle Paint Panel

## PAINT MODE

- *b*: Switch to Brush
- *t*: Switch to Text
- *r*: Switch to Rectangle
- *o*: Switch to Ellipse
- *a*: Switch to Arrow

- *R*: Use Red Color
- *G*: Use Green Color
- *B*: Use Blue Color
- *C*: Use Custom Color
- *Minus*: Reduce Stroke Size
- *Plus*: Increase Stroke Size
- *Equal*: Reset Stroke Size
- *k*: Clear Paints (cannot be undone)

## HEADER BAR

- *Ctrl+z*: Undo
- *Ctrl+Shift+z* or *Ctrl+y*: Redo
- *Ctrl+s*: Save to file (see man page)
- *Ctrl+c*: Copy to clipboard
- *Escape* or *q*: Quit swappy

# AUTHORS

Written and maintained by jtheoof <contact@jtheoof.me>. See
https://github.com/jtheoof/swappy.
