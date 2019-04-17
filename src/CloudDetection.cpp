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

#include "../include/CloudDetection.hpp"
#include <mutex>
#include <thread>

using namespace WaterCoherer;

CloundPositions CloudDetection::localize_clouds(const WaterCoherer::TiffImage &image_layer,
                                                unsigned int cores) {
  CloundPositions result;
  std::vector<std::thread> thread_pool;
  std::mutex result_mutex;
  for (unsigned int i = 0UL; i < cores; ++i) {
    thread_pool.emplace_back(
      std::thread([i, cores, &result, &image_layer, &result_mutex]() {
        unsigned int height = image_layer.height() / cores;
        unsigned int start = i > 0 ? height * i : height * i + 1;
        unsigned int stop = i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

        for (unsigned int y = start; y < stop - 1; ++y) {
          for (unsigned int x = 0; x < static_cast<unsigned int>(image_layer.width()); ++x) {
            float value = image_layer(x,y);
            if (value > 120) {
              std::lock_guard<std::mutex> guard(result_mutex);
              result.emplace_back(std::make_pair(x, y));
            }
          }
        }
      }));
  }

  for (auto &&thread : thread_pool) {
    thread.join();
  }
  return result;
}

TiffImage CloudDetection::generate_cloud_layer(const CloundPositions &cloud_positions, unsigned
int width, unsigned int height) {
  TiffImage result(width, height, 1, 1);
  for (const auto &cloud_pixel: cloud_positions) {
    result(cloud_pixel.first, cloud_pixel.second) = 255;
  }
  return result;
}
