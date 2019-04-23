#pragma once

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

#include "WaterCohererTypes.hpp"

namespace WaterCoherer {
  class LandsatImage {
  private:
    int widht_ = 0;
    int height_ = 0;
    std::string image_descripton_;
    ImageLayers image_layers_;
    void push_back_image_layer(const TiffImage&, const std::string&, int);
    const TiffImage& get_image_layer(const std::string&);

  public:
    LandsatImage() = default;
    ~LandsatImage() = default;
    void load_image(const char *input_directory_path);
    int width() const;
    int height() const;
    const TiffImage& view_red_layer();
    const TiffImage& view_green_layer();
    const TiffImage& view_blue_layer();
    const TiffImage& view_nir_layer();
    const TiffImage& view_swir_layer();
    const TiffImage& view_termal_layer();
  };
}
