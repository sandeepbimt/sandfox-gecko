#include "gtest/gtest.h"
#include "mozilla/gfx/SandfoxDarkModeColorFilter.h"
using namespace mozilla::gfx;
TEST(SandfoxDarkModeColorFilter, BackgroundDarkensWhite) {
  auto c = SandfoxDarkModeColorFilter::Transform(sRGBColor::OpaqueWhite(), SandfoxDarkModeRole::Background);
  EXPECT_LT(c.r, 0.30f); EXPECT_LT(c.g, 0.30f); EXPECT_LT(c.b, 0.30f);
}
TEST(SandfoxDarkModeColorFilter, DarkBackgroundPreserved) {
  sRGBColor c(0.08f, 0.08f, 0.08f, 1.0f);
  EXPECT_EQ(SandfoxDarkModeColorFilter::Transform(c, SandfoxDarkModeRole::Background), c);
}
TEST(SandfoxDarkModeColorFilter, BlackTextBecomesLight) {
  auto c = SandfoxDarkModeColorFilter::Transform(sRGBColor::OpaqueBlack(), SandfoxDarkModeRole::Foreground);
  EXPECT_GT(c.r, 0.75f); EXPECT_GT(c.g, 0.75f); EXPECT_GT(c.b, 0.75f);
}
TEST(SandfoxDarkModeColorFilter, TransparentPreserved) {
  sRGBColor c(1, 1, 1, 0); EXPECT_EQ(SandfoxDarkModeColorFilter::Transform(c, SandfoxDarkModeRole::Background), c);
}