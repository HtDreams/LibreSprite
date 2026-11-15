// LibreSprite
// Copyright (C) 2026  LibreSprite contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/pixel_format.h"

#include <cstddef>
#include <cstdint>

namespace script_api {

// Scripts see image data as bytes R,G,B,A (RGB) or V,A (grayscale), like an HTML
// canvas. doc stores those pixels as packed integers (r | g<<8 | b<<16 | a<<24 and
// v | a<<8), so the bytes only come out in that order on little-endian machines.
// On big-endian (PowerPC: MorphOS, AmigaOS 4) each pixel is byte-swapped in place.
// The swap is its own inverse, so it works both ways (getImageData/putImageData).
// Indexed and bitmap images use one byte per pixel: nothing to do.
inline void swap_pixels_to_script_order(uint8_t* data, std::size_t size, doc::PixelFormat format)
{
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  if (format == doc::IMAGE_RGB) {
    for (std::size_t i = 0; i + 3 < size; i += 4) {
      uint8_t t0 = data[i], t1 = data[i+1];
      data[i]   = data[i+3];
      data[i+1] = data[i+2];
      data[i+2] = t1;
      data[i+3] = t0;
    }
  }
  else if (format == doc::IMAGE_GRAYSCALE) {
    for (std::size_t i = 0; i + 1 < size; i += 2) {
      uint8_t t = data[i];
      data[i]   = data[i+1];
      data[i+1] = t;
    }
  }
#else
  (void)data; (void)size; (void)format;
#endif
}

} // namespace script_api
