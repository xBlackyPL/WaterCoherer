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

#include "Utils.hpp"
#include <algorithm>
#include <string>
#include <set>

namespace WaterCoherer {
  PixelPositions merge_pixel_positions_layers(PixelPositionsLayers &found_water) {
    PixelPositions result;
    for (const auto &entry : found_water) {
      PixelPositions pixel_set = entry.second;
      std::merge(pixel_set.begin(), pixel_set.end(), result.begin(), result.end(),
                 std::inserter(result, result.begin()));
    }
    return result;
  }

  std::vector<std::string> split(const std::string &string, char separator) {
    std::vector<std::string> result;
    std::string::size_type position = 0;
    std::string::size_type previous_position = 0;
    while ((position = string.find(separator, position)) != std::string::npos) {
      std::string substring(string.substr(previous_position, position - previous_position));
      result.push_back(substring);
      previous_position = ++position;
    }
    result.push_back(string.substr(previous_position, position - previous_position));
    return result;
  }

  TiffImage generate_layer(const PixelPositions &pixel_positions, int width,
    int height) {
    TiffImage result(width, height, 1, 1);
    for (const auto &pixel: pixel_positions) {
      result(pixel.first, pixel.second) = 255;
    }
    return result;
  }
}