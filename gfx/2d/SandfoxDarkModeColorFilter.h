/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_gfx_SandfoxDarkModeColorFilter_h
#define mozilla_gfx_SandfoxDarkModeColorFilter_h

#include "mozilla/RelativeLuminanceUtils.h"
#include "mozilla/gfx/Types.h"

namespace mozilla::gfx {

// First SANDFOX Smart Dark renderer primitive.
//
// This deliberately handles only solid background colors in V1.  The layout
// layer decides which paint role is being transformed; images, video, canvas,
// SVG assets, and text are not touched by this primitive.
class SandfoxDarkModeColorFilter final {
 public:
  static sRGBColor TransformBackground(const sRGBColor& aColor) {
    if (aColor.a == 0.0f) {
      return aColor;
    }

    const nscolor color = aColor.ToABGR();
    const float luminance = RelativeLuminanceUtils::Compute(color);

    // Keep already-dark backgrounds unchanged.  This threshold is intentionally
    // conservative for the first renderer prototype and can be tuned after
    // measured compatibility testing.
    constexpr float kBackgroundLuminanceThreshold = 0.75f;
    if (luminance < kBackgroundLuminanceThreshold) {
      return aColor;
    }

    // Map bright backgrounds to a dark neutral luminance while preserving the
    // original hue/chroma relationship as much as the luminance adjustment
    // permits.  This follows the same role-aware/thresholded principle used by
    // Chromium's native Force Dark pipeline without applying a page-wide
    // inversion matrix.
    constexpr float kTargetBackgroundLuminance = 0.06f;
    return sRGBColor::FromABGR(
        RelativeLuminanceUtils::Adjust(color, kTargetBackgroundLuminance));
  }
};

}  // namespace mozilla::gfx

#endif  // mozilla_gfx_SandfoxDarkModeColorFilter_h
