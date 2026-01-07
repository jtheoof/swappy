swappy(1)

# NAME

swappy - grab and edit on the fly snapshots of a Wayland compositor

# SYNOPSIS

*swappy* [options...]

# DESCRIPTION

swappy is a command-line utility to take and edit screenshots of Wayland
desktops. Works great with grim, slurp and sway. But can easily work with
other screen copy tools that can output a final image to *stdout*.

swappy will save the annotated images to the config *save_dir*, see below.

If absent, then if it will try to default to a *Desktop* folder following this
pattern: *$XDG\_DESKTOP\_DIR*. If this variable is not set, it will revert to:
*$XDG\_CONFIG\_HOME/Desktop*. If *$XDG\_CONFIG\_HOME* is not set, it will revert
to: *$HOME/Desktop*.

# OPTIONS

*-h, --help*
	Show help message and quit.

*-v, --version*
	Show version and quit.

*-f, --file* <file>
	An image file to load for editing.

	If set to *-*, read the file from standard input instead. This is grim
	friendly.

*-o, --output-file <file>*
	Print the final surface to *<file>* when exiting the application.

	If set to *-*, prints the final surface to *stdout*.

	Note that the *Save* button will save the image to the config *save_dir*
	parameter, as described in the DESCRIPTION section.

# CONFIG FILE

The config file is located at *$XDG\_CONFIG\_HOME/swappy/config* or at
*$HOME/.config/swappy/config*. The file follows the GLib *conf* format.

```
	[Section]
	key=value
```

The following lines can be used as swappy's default:

```
	[Default]
	save_dir=$HOME/Desktop
	save_filename_format=swappy-%Y%m%d-%H%M%S.png
	show_panel=false
	line_size=5
	text_size=20
	text_font=sans-serif
	paint_mode=brush
	early_exit=false
	fill_shape=false
	auto_save=false
	custom_color=rgba(192,125,17,1)
	transparent=false
	transparency=50
```

- *save_dir* is where swappshots will be saved, can contain env variables, when it does not exist, swappy attempts to create it first, but does not abort if directory creation fails
- *save_filename_format* is the filename template, if it contains a date format, this will be parsed into a timestamp. Format is detailed in strftime(3). If this date format is missing, filename will have no timestamp
- *show_panel* is used to toggle the paint panel on or off upon startup
- *line_size* is the default line size (must be between 1 and 50)
- *text_size* is the default text size (must be between 10 and 50)
- *text_font* is the font used to render text, its format is pango friendly
- *paint_mode* is the mode activated at application start (must be one of: brush|text|rectangle|ellipse|arrow|blur|crop, matching is case-insensitive)
- *early_exit* is used to make the application exit after saving the picture or copying it to the clipboard
- *fill_shape* is used to toggle shape filling (for the rectangle and ellipsis tools) on or off upon startup
- *auto_save* is used to toggle auto saving of final buffer to *save_dir* upon exit
- *custom_color* is used to set a default value for the custom color. Accepted
formats are: standard name (one of: https://github.com/rgb-x/system/blob/master/root/etc/X11/rgb.txt),  #rgb, #rrggbb, #rrrgggbbb, #rrrrggggbbbb, rgb(r,b,g), rgba(r,g,b,a)
- *transparency* is used to set transparency of everything that is drawn during startup
- *transparent* is used to toggle transparency during startup


# KEY BINDINGS

## LAYOUT

- *Ctrl+b*: Toggle Paint Panel

## PAINT MODE

- *b*: Switch to Brush
- *e* *t*: Switch to Text (Editor)
- *r* *s*: Switch to Rectangle (Square)
- *c* *o*: Switch to Ellipse (Circle)
- *a*: Switch to Arrow
- *d*: Switch to Blur (d stands for droplet)
- *z*: Switch to Crop

- *R*: Use Red Color
- *G*: Use Green Color
- *B*: Use Blue Color
- *C*: Use Custom Color
- *Minus*: Reduce Stroke Size
- *Plus*: Increase Stroke Size
- *Equal*: Reset Stroke Size
- *f*: Toggle Shape Filling
- *T*: Toggle Transparency
- *x* *k*: Clear Paints (cannot be undone)

## MODIFIERS

- *Ctrl*: Center Shape (Rectangle & Ellipse) based on draw start

## HEADER BAR

- *Ctrl+z*: Undo
- *Ctrl+Shift+z* or *Ctrl+y*: Redo
- *Ctrl+s*: Save to file (see man page)
- *Ctrl+c*: Copy to clipboard
- *Escape* or *q* or *Ctrl+w*: Quit swappy

# AUTHORS

Written and maintained by jtheoof <contact@jtheoof.me>. See
https://github.com/jtheoof/swappy.
