#pragma once

#include "swappy.h"

bool clipboard_copy_drawing_area_to_selection(struct swappy_state *state);

// Before swappy exits: if it still owns the clipboard, hand the image to
// `wl-copy` so it outlives swappy.
void clipboard_hand_off_selection(void);
