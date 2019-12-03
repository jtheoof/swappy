#pragma once

#include <stdio.h>
#include <wayland-client.h>

#include "swappy.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"

void screencopy_frame_handle_buffer(void *data,
                                    struct zwlr_screencopy_frame_v1 *frame,
                                    uint32_t format, uint32_t width,
                                    uint32_t height, uint32_t stride);
void screencopy_frame_handle_flags(void *data,
                                   struct zwlr_screencopy_frame_v1 *frame,
                                   uint32_t flags);
void screencopy_frame_handle_ready(void *data,
                                   struct zwlr_screencopy_frame_v1 *frame,
                                   uint32_t tv_sec_hi, uint32_t tv_sec_lo,
                                   uint32_t tv_nsec);
void screencopy_frame_handle_failed(void *data,
                                    struct zwlr_screencopy_frame_v1 *frame);
bool screencopy_init(struct swappy_state *state);
void screencopy_destroy_buffer(struct swappy_buffer *buffer);
bool screencopy_parse_geometry(struct swappy_state *state);
