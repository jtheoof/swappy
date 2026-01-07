#pragma once

#include "swappy.h"

void render_state(struct swappy_state *state);

// Draw the guides of the crop being dragged (darkened outside, border) in image
// coordinates. They are drawn on screen only, never into the rendering
// surface, so they don't end up in the image.
void render_crop_guides(cairo_t *cr, struct swappy_state *state);
