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

using namespace WaterCoherer;

int main(int argc, char const *argv[]) {
  unsigned int cores = std::thread::hardware_concurrency();
  std::cout << "Water Coherer: Application starting..." << std::endl;
  std::cout << "Water Coherer: Using " << cores << " logical processors." << std::endl;

  LandsatImage image;
  image.load_image("../data/LE71880252009264ASN00");

  TiffImage result = NDWICalculator::generate_ndwi_layer_high_performance(
    image.get_image_layer("green"), image.get_image_layer("near infrared"), NDWICalculator::Method::GreenNir, cores);
  result.save("result.tiff");

  auto water_localization = NDWICalculator::localize_water(image.get_image_layer("green"), image.get_image_layer("near infrared"), cores);
  std::cout << "Water Coherer: Localized " << water_localization.size() << " pixels of water."
            << std::endl;

  WaterDifferencer water_differencer(water_localization);
  auto water_clasterized_water = water_differencer.generate_clasterized_water_layer(image.get_image_layer("near infrared"));
  water_clasterized_water.save("water_clasterized.tiff");

//  TiffImage red_layer;
//  red_layer.load_tiff(
//    "../data/LE71880252009264ASN00/L71188025_02520090921_B10.TIF");
//
//  auto cloud_localization = CloudDetection::localize_clouds(red_layer, cores);
//  auto cloud_layer = CloudDetection::generate_cloud_layer(cloud_localization,
//                                                          red_layer.width(), red_layer.height());

//  cloud_layer.save("clouds.tiff");
  /*
   *  auto cloud_layer_0101 = CloudDetection::localize_clouds(red_layer_0101, cores);
   *  auto cloud_layer_0202 = CloudDetection::localize_clouds(red_layer_0202, cores);
   *  auto cloud_layer_0303 = CloudDetection::localize_clouds(red_layer_0303, cores);
   *
   *  auto total_cloud_layer = sum(cloud_layer0101...0303);
   *
   *  auto water_localization_... = NDWICalculator::localize_water(
   *    green_layer...,
   *    nir_layer...,
   *    cloud_layer_0101,
   *    cores);
   *
   *   water_localization_0101.size() != water_localization_0202.size() !=
   *   water_localization_0303.size()
   */

  return 0;
}