/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_gfx_SandfoxDarkModeColorFilter_h
#define mozilla_gfx_SandfoxDarkModeColorFilter_h

#include "mozilla/LookAndFeel.h"
#include "mozilla/RelativeLuminanceUtils.h"
#include "mozilla/StaticPrefs_layout.h"
#include "mozilla/gfx/Types.h"
#include "nsIFrame.h"
#include "nsPresContext.h"

namespace mozilla::gfx {

class SandfoxDarkModeColorFilter final {
 public:
  enum class Role : uint8_t { Background, Foreground, Border, Shadow };

  static bool IsActive(const nsIFrame& aFrame) {
    if (!StaticPrefs::layout_css_sandfox_dark_pages_enabled() ||
        aFrame.PresContext()->IsChrome()) {
      return false;
    }

    // Explicit native dark support wins. This is the primary double-darkening
    // guard. Light-only and unspecified documents use Smart Dark.
    if (auto scheme = LookAndFeel::ExplicitColorSchemeForFrame(&aFrame)) {
      if (*scheme == ColorScheme::Dark) {
        return false;
      }
    }
    return true;
  }

  static sRGBColor Transform(const nsIFrame& aFrame, const sRGBColor& aColor,
                             Role aRole,
                             const sRGBColor* aBackground = nullptr) {
    if (!IsActive(aFrame) || aColor.a == 0.0f) {
      return aColor;
    }

    const float luminance =
        RelativeLuminanceUtils::Compute(aColor.ToABGR());
    const float backgroundLuminance =
        aBackground ? RelativeLuminanceUtils::Compute(aBackground->ToABGR())
                    : luminance;

    switch (aRole) {
      case Role::Background:
        return TransformBackground(aColor, luminance);
      case Role::Foreground:
        if (backgroundLuminance > 0.30f && luminance < 0.35f) {
          return Adjust(aColor, 0.90f);
        }
        return aColor;
      case Role::Border:
        if (backgroundLuminance > 0.30f && luminance > 0.55f) {
          return Adjust(aColor, 0.25f);
        }
        if (backgroundLuminance < 0.30f && luminance < 0.12f) {
          return Adjust(aColor, 0.30f);
        }
        return aColor;
      case Role::Shadow:
        if (backgroundLuminance < 0.30f && luminance > 0.65f) {
          return Adjust(aColor, 0.12f);
        }
        return aColor;
    }
    return aColor;
  }

  static sRGBColor TransformBackground(const sRGBColor& aColor) {
    return TransformBackground(
        aColor, RelativeLuminanceUtils::Compute(aColor.ToABGR()));
  }

 private:
  static sRGBColor Adjust(const sRGBColor& aColor, float aTargetLuminance) {
    return sRGBColor::FromABGR(RelativeLuminanceUtils::Adjust(
        aColor.ToABGR(), aTargetLuminance));
  }

  static sRGBColor TransformBackground(const sRGBColor& aColor,
                                       float aLuminance) {
    if (aLuminance < 0.80f) {
      return aColor;
    }

    const uint32_t theme = StaticPrefs::layout_css_sandfox_dark_pages_theme();
    // Chromium Force Dark uses a very dark default surface (roughly #121212).\n    // Keep SANDFOX similarly dark while preserving the source hue.\n    float target = 0.006f;
    switch (theme) {
      case 1:  // Deep
        target = 0.002f;
        break;
      case 2:  // AMOLED/OLED
        target = 0.0f;
        break;
      case 3:  // Grey
        target = 0.030f;
        break;
      case 4:  // Blue
        target = 0.012f;
        break;
      case 5:  // Warm
        target = 0.015f;
        break;
      default:
        break;
    }
    return Adjust(aColor, target);
  }
};

}  // namespace mozilla::gfx

#endif  // mozilla_gfx_SandfoxDarkModeColorFilter_h
