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

#include "NDWICalculator.hpp"
#include "CloudDetection.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <LandsatImage.hpp>
#include <Utils.hpp>

using namespace WaterCoherer;

int main() {
  unsigned int cores = std::thread::hardware_concurrency();
  std::cout << "INFO Water Coherer: Application starting..." << std::endl;
  std::cout << "INFO Water Coherer: Using " << cores << " logical processors." << std::endl;

  LandsatImage oldest_image;
  oldest_image.load_image("../data/LE71880252009232ASN00");

  LandsatImage medium_image;
  medium_image.load_image("../data/LE71880252009104ASN00");

  LandsatImage recent_image;
  recent_image.load_image("../data/LE71880252009264ASN00");

  PixelPositionsLayers localized_clouds =
    {
      {"oldest", CloudDetection::localize_clouds(oldest_image.view_blue_layer(), cores)},
      {"medium", CloudDetection::localize_clouds(medium_image.view_blue_layer(), cores)},
      {"recent", CloudDetection::localize_clouds(recent_image.view_blue_layer(), cores)}
    };

  auto sumarized_cloud_positons = merge_pixel_positions_layers(localized_clouds);


  auto water_localization_oldest = NDWICalculator::localize_water(cores,
                                                                  oldest_image.view_green_layer(),
                                                                  oldest_image.view_nir_layer(),
                                                                  sumarized_cloud_positons);

  std::cout << "INFO Water Coherer: Localized " << water_localization_oldest.size()
            << " pixels of water."
            << std::endl;

  auto water_localization_medium = NDWICalculator::localize_water(cores,
                                                                  medium_image.view_green_layer(),
                                                                  medium_image.view_nir_layer(),
                                                                  sumarized_cloud_positons);

  std::cout << "INFO Water Coherer: Localized " << water_localization_medium.size()
            << " pixels of water."
            << std::endl;

  auto water_localization_recent = NDWICalculator::localize_water(cores,
                                                                  recent_image.view_green_layer(),
                                                                  recent_image.view_nir_layer(),
                                                                  sumarized_cloud_positons);

  WaterDifferencer differencer(water_localization_recent);
  differencer.generate_clasterized_water_layer(recent_image.view_nir_layer()).save_tiff
    ("different_water_types.tif");

  std::cout << "INFO Water Coherer: Localized " << water_localization_recent.size()
            << " pixels of water."
            << std::endl;

  auto oldest_image_water = generate_layer(water_localization_oldest,
                                           oldest_image.width(),
                                           oldest_image.height());

  auto medium_image_water = generate_layer(water_localization_medium,
                                           medium_image.width(),
                                           medium_image.height());

  auto recent_image_water = generate_layer(water_localization_recent,
                                           recent_image.width(),
                                           recent_image.height());

  auto common_clouds = generate_layer(sumarized_cloud_positons, oldest_image.width(),
                                      oldest_image.height());

  oldest_image_water.save_tiff("oldest_water.tif");
  medium_image_water.save_tiff("medium_water.tif");
  recent_image_water.save_tiff("recent_water.tif");
  common_clouds.save_tiff("common_clouds.tif");
  return 0;
}