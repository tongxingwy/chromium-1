// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_PUBLIC_GRAPHICS_TYPES_H_
#define CHROMECAST_PUBLIC_GRAPHICS_TYPES_H_

namespace chromecast {

struct Rect {
  Rect(int w, int h) : x(0), y(0), width(w), height(h) {}
  Rect(int arg_x, int arg_y, int w, int h)
      : x(arg_x), y(arg_y), width(w), height(h) {}

  int x;
  int y;
  int width;
  int height;
};

struct Size {
  Size(int w, int h) : width(w), height(h) {}

  int width;
  int height;
};

}  // namespace chromecast

#endif  // CHROMECAST_PUBLIC_GRAPHICS_TYPES_H_
