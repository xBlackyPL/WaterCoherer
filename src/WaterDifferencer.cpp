// The MIT License (MIT)

// Copyright (c) 2019 Rafal Aleksander

//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.

#include "WaterDifferencer.hpp"

#include <utility>

using namespace WaterCoherer;

WaterDifferencer::WaterDifferencer(PixelPositionContainer water_localization) :
  water_localization_(std::move(water_localization)) {
}

TiffImage WaterDifferencer::generate_clasterized_water_layer(const TiffImage &image_layer) {
  TiffImage result(image_layer.width(), image_layer.height(), 1, 3);

  for (const auto &water_pixel: water_localization_) {
    if (image_layer(water_pixel.first, water_pixel.second) >= 17.f) {
      result(water_pixel.first, water_pixel.second, 1) = 255;
    } else {
      result(water_pixel.first, water_pixel.second, 2) = 255;
    }
  }
  return result;
}
