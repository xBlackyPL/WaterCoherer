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

#include "CImage.hpp"
#include "LandsatImage.hpp"
#include "Utils.hpp"
#include <iostream>
#include <memory>

using namespace WaterCoherer;

void LandsatImage::load_image(const char *input_directory_path) {
  auto directory = opendir(input_directory_path);
  if (nullptr == directory) {
    std::cerr << "Could not open " << input_directory_path << " directory" << std::endl;
    return;
  }
  auto directory_path = std::string(input_directory_path);
//  auto path = split(directory_path, '/');
//  image_descripton_ = path.back();

  auto file = readdir(directory);
  do {
    auto file_name = std::string(file->d_name);
    if (file_name.size() < 3) {
      continue;
    }
    auto file_extension = file_name.substr(file_name.size() - 3, 3);
    if (file_extension == "TIF") {
      unsigned long layer_index = std::strtol(file_name.substr(file_name.size() - 6, 1).c_str(),nullptr, 10);
      auto input_image_file_path = directory_path + "/" + file_name;
      TiffImage image_layer{};
      image_layer.load_tiff(input_image_file_path.c_str());
      push_back_image_layer(image_layer, layer_index);
    }
  } while (nullptr != (file = readdir(directory)));
  closedir(directory);
}

void LandsatImage::push_back_image_layer(const TiffImage &image_layer, unsigned char layer_index) {
  if (nullptr != image_layer.data()) {
    switch (layer_index) {
      case 1:
        image_layers_.insert({std::string("red"), image_layer});
        break;
      case 2:
        image_layers_.insert({std::string("green"), image_layer});
        break;
      case 3:
        image_layers_.insert({std::string("blue"), image_layer});
        break;
      case 4:
        image_layers_.insert({std::string("near infrared"), image_layer});
        break;
      case 5:
        image_layers_.insert({std::string("shortwave infrared"), image_layer});
        break;
      case 6:
        image_layers_.insert({std::string("thermal"), image_layer});
        break;
      default:
        std::cerr << "WaterCoherer: Omitted input image layer" << std::endl;
    }
  }
}

const TiffImage &LandsatImage::get_image_layer(const std::string &layer) {
  return image_layers_.at(layer);
}
