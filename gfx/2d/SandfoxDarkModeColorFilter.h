/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef mozilla_gfx_SandfoxDarkModeColorFilter_h
#define mozilla_gfx_SandfoxDarkModeColorFilter_h

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

#include "mozilla/StaticPrefs_layout.h"
#include "mozilla/gfx/Types.h"

namespace mozilla::gfx {

enum class SandfoxDarkModeRole : uint8_t { Background, Foreground, Border, Shadow };

class SandfoxDarkModeColorFilter final {
 public:
  static bool Enabled() { return StaticPrefs::layout_css_sandfox_dark_pages_mode() == 2; }
  static uint32_t Theme() { return StaticPrefs::layout_css_sandfox_dark_pages_theme(); }

  static sRGBColor Transform(const sRGBColor& aColor, SandfoxDarkModeRole aRole) {
    if (!Enabled() || aColor.a <= 0.0f) return aColor;
    switch (aRole) {
      case SandfoxDarkModeRole::Background: return TransformBackground(aColor);
      case SandfoxDarkModeRole::Foreground: return TransformForeground(aColor);
      case SandfoxDarkModeRole::Border: return TransformBorder(aColor);
      case SandfoxDarkModeRole::Shadow: return TransformShadow(aColor);
    }
    return aColor;
  }

 private:
  struct Hsl { float h; float s; float l; };

  static float Luminance(const sRGBColor& c) {
    auto linear = [](float v) {
      return v <= 0.04045f ? v / 12.92f : std::pow((v + 0.055f) / 1.055f, 2.4f);
    };
    return 0.2126f * linear(c.r) + 0.7152f * linear(c.g) + 0.0722f * linear(c.b);
  }

  static Hsl ToHsl(const sRGBColor& c) {
    const float mx = std::max({c.r, c.g, c.b});
    const float mn = std::min({c.r, c.g, c.b});
    const float d = mx - mn;
    Hsl out{0.0f, 0.0f, (mx + mn) * 0.5f};
    if (d <= 0.00001f) return out;
    out.s = d / (1.0f - std::fabs(2.0f * out.l - 1.0f));
    if (mx == c.r) out.h = std::fmod((c.g - c.b) / d, 6.0f);
    else if (mx == c.g) out.h = (c.b - c.r) / d + 2.0f;
    else out.h = (c.r - c.g) / d + 4.0f;
    out.h /= 6.0f;
    if (out.h < 0.0f) out.h += 1.0f;
    return out;
  }

  static sRGBColor FromHsl(const Hsl& h, float alpha) {
    if (h.s <= 0.00001f) return sRGBColor(h.l, h.l, h.l, alpha);
    const float q = h.l < 0.5f ? h.l * (1.0f + h.s) : h.l + h.s - h.l * h.s;
    const float p = 2.0f * h.l - q;
    auto hue = [p, q](float t) {
      if (t < 0.0f) t += 1.0f; if (t > 1.0f) t -= 1.0f;
      if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
      if (t < 1.0f / 2.0f) return q;
      if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
      return p;
    };
    return sRGBColor(hue(h.h + 1.0f / 3.0f), hue(h.h), hue(h.h - 1.0f / 3.0f), alpha);
  }

  static std::pair<float, float> ThemeTones() {
    switch (Theme()) {
      case 1: return {0.14f, 0.20f};
      case 2: return {0.06f, 0.12f};
      case 3: return {0.00f, 0.04f};
      case 4: return {0.00f, 0.02f};
      case 5: return {0.08f, 0.14f};
      case 6: return {0.12f, 0.18f};
      default: return {0.095f, 0.15f};
    }
  }

  static sRGBColor TransformBackground(const sRGBColor& c) {
    if (Luminance(c) < 0.48f) return c;
    const auto tones = ThemeTones();
    Hsl h = ToHsl(c);
    if (h.s < 0.16f) return sRGBColor(Luminance(c) > 0.82f ? tones.first : tones.second,
                                      Luminance(c) > 0.82f ? tones.first : tones.second,
                                      Luminance(c) > 0.82f ? tones.first : tones.second, c.a);
    h.l = Luminance(c) > 0.82f ? 0.20f : 0.28f;
    h.s = std::min(h.s, 0.70f);
    return FromHsl(h, c.a);
  }

  static sRGBColor TransformForeground(const sRGBColor& c) {
    if (Luminance(c) > 0.42f) return c;
    Hsl h = ToHsl(c); h.l = h.s < 0.16f ? 0.90f : 0.82f; h.s = std::min(h.s, 0.65f);
    return FromHsl(h, c.a);
  }

  static sRGBColor TransformBorder(const sRGBColor& c) {
    if (Luminance(c) < 0.22f) return c;
    Hsl h = ToHsl(c); h.l = h.s < 0.16f ? 0.28f : 0.38f; h.s = std::min(h.s, 0.55f);
    return FromHsl(h, c.a);
  }

  static sRGBColor TransformShadow(const sRGBColor& c) {
    if (Luminance(c) < 0.18f) return c;
    Hsl h = ToHsl(c); h.l = 0.06f; h.s = std::min(h.s, 0.25f);
    return FromHsl(h, c.a);
  }
};

}  // namespace mozilla::gfx

#endif