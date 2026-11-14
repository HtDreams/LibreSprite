// LibreSprite | Copyright (C) 2024-2026  LibreSprite contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include <cstring>
#include <vector>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/app.h"
#include "app/document.h"
#include "app/file/file.h"
#include "app/file/file_format.h"
#include "app/file/format_options.h"
#include "app/ini_file.h"
#include "base/file_handle.h"
#include "doc/doc.h"

#include <stdio.h>
#include <stdlib.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>

namespace app {

using namespace base;

class QoiFormat : public FileFormat {
  const char* onGetName() const override { return "qoi"; }
  const char* onGetExtensions() const override { return "qoi"; }
  int onGetFlags() const override {
    return
      FILE_SUPPORT_LOAD |
      FILE_SUPPORT_SAVE |
      FILE_SUPPORT_RGB |
      FILE_SUPPORT_RGBA |
      FILE_SUPPORT_GRAY |     // onSave() converts grayscale to RGBA
      FILE_SUPPORT_GRAYA |
      FILE_SUPPORT_SEQUENCES |
      FILE_SUPPORT_PALETTE_WITH_ALPHA;
  }

  bool onLoad(FileOp* fop) override;
  bool onSave(FileOp* fop) override;
};

static FileFormat::Regular<QoiFormat> ff{"qoi"};

bool QoiFormat::onLoad(FileOp* fop)
{
  qoi_desc desc;
  auto decoded = std::shared_ptr<void>(qoi_read(fop->filename().c_str(), &desc, 4), free);
  if (!decoded) {
    fop->setError("Could not load qoi");
    return false;
  }
  fop->sequenceSetHasAlpha(true);
  // qoi's own decoder allows dimensions well past sequenceImage()'s shared
  // kMaxFileImageDimension cap, so this can legitimately return null here.
  auto image = fop->sequenceImage(IMAGE_RGB, desc.width, desc.height);
  if (!image)
    return false;
  // qoi_read() returns R,G,B,A bytes. doc RGB images store each pixel as a
  // packed uint32_t (r | g<<8 | b<<16 | a<<24), whose byte order in memory
  // depends on the CPU: copy pixel by pixel instead of memcpy() so the result
  // is correct on big-endian machines too (e.g. PowerPC/MorphOS).
  const uint8_t* src = static_cast<const uint8_t*>(decoded.get());
  for (int y = 0; y < (int)desc.height; ++y) {
    for (int x = 0; x < (int)desc.width; ++x, src += 4)
      doc::put_pixel(image, x, y, doc::rgba(src[0], src[1], src[2], src[3]));
  }
  return true;
}

bool QoiFormat::onSave(FileOp* fop)
{
  const Image* image = fop->sequenceImage();
  const int w = image->width();
  const int h = image->height();

  // qoi_write() expects R,G,B,A bytes. Build them pixel by pixel: passing the
  // image memory directly was wrong on big-endian CPUs (packed uint32_t pixels
  // are A,B,G,R in memory there) and read out of bounds for grayscale images
  // (2 bytes per pixel), crashing the save.
  std::vector<uint8_t> rgba(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
  uint8_t* dst = rgba.data();
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x, dst += 4) {
      const doc::color_t c = doc::get_pixel(image, x, y);
      switch (image->pixelFormat()) {
        case IMAGE_RGB:
          dst[0] = doc::rgba_getr(c);
          dst[1] = doc::rgba_getg(c);
          dst[2] = doc::rgba_getb(c);
          dst[3] = doc::rgba_geta(c);
          break;
        case IMAGE_GRAYSCALE:
          dst[0] = dst[1] = dst[2] = doc::graya_getv(c);
          dst[3] = doc::graya_geta(c);
          break;
        default:
          fop->setError("QOI: only RGB and grayscale images can be saved\n");
          return false;
      }
    }
  }

  qoi_desc desc {
    .width = (unsigned int) w,
    .height = (unsigned int) h,
    .channels = 4,
    .colorspace = QOI_SRGB
  };
  return qoi_write(fop->filename().c_str(), rgba.data(), &desc) != 0;
}

} // namespace app
