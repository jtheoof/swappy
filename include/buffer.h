#pragma once

#include <stdio.h>
#include <wayland-client.h>

#include "swappy.h"

void buffer_wayland_destroy(struct swappy_buffer *buffer);

bool buffer_init_from_screencopy(struct swappy_state *state);
bool buffer_init_from_file(struct swappy_state *state);

bool buffer_parse_geometry(struct swappy_state *state);

void buffer_resize_patterns(struct swappy_state *state);

void buffer_free_all(struct swappy_state *state);
