#!/bin/sh

# SPDX-FileCopyrightText: 2019 Bilal Elmoussaoui <bil.elmoussaoui@gmail.com>
# SPDX-FileCopyrightText: 2020 Christian Hergert <chergert@redhat.com>
# SPDX-FileCopyrightText: 2021 Felix Häcker <haeckerfelix@gnome.org>
# SPDX-License-Identifier: GPL-3.0-or-later

export MESON_BUILD_ROOT="$1"
export MESON_SOURCE_ROOT="$2"
export CARGO_TARGET_DIR="$MESON_BUILD_ROOT"/target
export CARGO_HOME="$MESON_BUILD_ROOT"/cargo-home
export OUTPUT="$3"
export BUILDTYPE="$4"
export APP_BIN="$5"


if [ $BUILDTYPE = "release" ]
then
    echo "RELEASE MODE"
    cargo build --manifest-path \
        "$MESON_SOURCE_ROOT"/Cargo.toml --release && \
        cp "$CARGO_TARGET_DIR"/release/"$APP_BIN" "$OUTPUT"
else
    echo "DEBUG MODE"
    cargo build --manifest-path \
        "$MESON_SOURCE_ROOT"/Cargo.toml && \
        cp "$CARGO_TARGET_DIR"/debug/"$APP_BIN" "$OUTPUT"
fi
