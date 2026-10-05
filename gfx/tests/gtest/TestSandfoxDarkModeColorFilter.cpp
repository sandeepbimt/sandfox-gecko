/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/RelativeLuminanceUtils.h"
#include "mozilla/gfx/SandfoxDarkModeColorFilter.h"
#include "gtest/gtest.h"

using mozilla::RelativeLuminanceUtils;
using mozilla::gfx::SandfoxDarkModeColorFilter;
using mozilla::gfx::sRGBColor;

TEST(SandfoxDarkModeColorFilter, DarkBackgroundIsPreserved) {
  const sRGBColor input = sRGBColor::FromABGR(NS_RGB(32, 32, 32));
  const sRGBColor output = SandfoxDarkModeColorFilter::TransformBackground(input);

  EXPECT_EQ(output.ToABGR(), input.ToABGR());
}

TEST(SandfoxDarkModeColorFilter, BrightBackgroundBecomesDark) {
  const sRGBColor input = sRGBColor::FromABGR(NS_RGB(255, 255, 255));
  const sRGBColor output = SandfoxDarkModeColorFilter::TransformBackground(input);

  EXPECT_LT(RelativeLuminanceUtils::Compute(output.ToABGR()), 0.10f);
  EXPECT_EQ(NS_GET_A(output.ToABGR()), 255);
}

TEST(SandfoxDarkModeColorFilter, TransparentBackgroundIsPreserved) {
  const sRGBColor input = sRGBColor::FromABGR(NS_RGBA(255, 255, 255, 0));
  const sRGBColor output = SandfoxDarkModeColorFilter::TransformBackground(input);

  EXPECT_EQ(output.ToABGR(), input.ToABGR());
}
