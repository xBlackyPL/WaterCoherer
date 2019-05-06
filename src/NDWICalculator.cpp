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

#include <utility>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <NDWICalculator.hpp>

using namespace WaterCoherer;

TiffImage NDWICalculator::generate_ndwi_layer_nir_swir(const TiffImage &nir_layer,
                                                       const TiffImage &swir_layer) {
  TiffImage result(nir_layer.width(), nir_layer.height(), 1, 1);

  if (nir_layer.height() != swir_layer.height() || nir_layer.width() != swir_layer.width()) {
    return result;
  }

  cimg_forXY(result, x, y) {
      float nir_value = nir_layer(static_cast<unsigned int>(x),
                                  static_cast<unsigned int>(y));

      float swir_value = swir_layer(static_cast<unsigned int>(x),
                                    static_cast<unsigned int>(y));

      if (nir_value > 1.f && swir_value > 1.f) {
        float ndwi_level = (nir_value - swir_value) / (nir_value + swir_value);
        if (ndwi_level > 1.f) {
          result(static_cast<unsigned int>(x), static_cast<unsigned int>(y)) = 255;
        }
      }
    }
  return result;
}

TiffImage NDWICalculator::generate_ndwi_layer_green_nir(const TiffImage &green_layer, const
TiffImage &nir_layer) {
  TiffImage result(green_layer.width(), green_layer.height(), 1, 1);

  if (green_layer.height() != green_layer.height() || green_layer.width() != green_layer.width()) {
    return result;
  }

  cimg_forXY(result, x, y) {
      float green_value = green_layer(static_cast<unsigned int>(x),
                                      static_cast<unsigned int>(y));

      float nir_value = nir_layer(static_cast<unsigned int>(x),
                                  static_cast<unsigned int>(y));

      if (green_value > 1.f && nir_value > 1.f) {
        float ndwi_level = (green_value - nir_value) / (green_value + nir_value);
        if (ndwi_level >= 0.33f) {
          result(static_cast<unsigned int>(x), static_cast<unsigned int>(y)) = 255;
        }
      }
    }
  return result;
}

TiffImage NDWICalculator::generate_ndwi_layer_green_nir_high_performance(
  const TiffImage &green_layer, const TiffImage &nir_layer, unsigned int cores) {
  std::vector<std::thread> thread_pool;

  auto start = std::chrono::system_clock::now();
  TiffImage result(green_layer.width(), green_layer.height(), 1, 1);
  auto stop = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration<double>(stop - start);
  std::cout << std::to_string(elapsed.count()) + ",";

  start = std::chrono::system_clock::now();
  for (unsigned int i = 0UL; i < cores; ++i) {
    thread_pool.emplace_back(
      std::thread([i, cores, &result, &green_layer, &nir_layer]() {
        unsigned int height = green_layer.height() / cores;
        unsigned int start = i > 0 ? height * i : height * i + 1;
        unsigned int stop = i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

        for (unsigned int y = start; y < stop - 1; ++y) {
          for (unsigned int x = 0; x < static_cast<unsigned int>(green_layer.width()); ++x) {
            float green_value = green_layer(x, y);
            float nir_value = nir_layer(x, y);

            if (green_value > 1.f && nir_value > 1.f) {
              float ndwi_level = (green_value - nir_value) / (green_value + nir_value);
              result(x, y) = static_cast<unsigned char>(ndwi_level >= 0.33f ? 255 : 0);
            }
          }
        }
      }));
  }

  for (auto &&thread : thread_pool) {
    thread.join();
  }
  stop = std::chrono::system_clock::now();
  elapsed = std::chrono::duration<double>(stop - start);
  std::cout << std::to_string(elapsed.count()) + ",";
  return result;
}

PixelPositions NDWICalculator::localize_water(unsigned int cores,
                                              const TiffImage &green_layer,
                                              const TiffImage &nir_layer) {
  std::vector<std::thread> thread_pool;
  std::mutex result_mutex;
  PixelPositions result;
  auto start = std::chrono::system_clock::now();
  for (unsigned int i = 0UL; i < cores; ++i) {
    thread_pool.emplace_back(
      std::thread([i, cores, &result, &green_layer, &nir_layer, &result_mutex]() {
        unsigned int height = green_layer.height() / cores;
        unsigned int start = i > 0 ? height * i : height * i + 1;
        unsigned int stop = i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

        for (unsigned int y = start; y < stop - 1; ++y) {
          for (unsigned int x = 0; x < static_cast<unsigned int>(green_layer.width()); ++x) {
            float green_value = green_layer(x, y);
            float nir_value = nir_layer(x, y);

            if (green_value > 1.f && nir_value > 1.f) {
              float ndwi_level = (green_value - nir_value) / (green_value + nir_value);
              if (ndwi_level >= 0.33f) {
                std::lock_guard<std::mutex> guard(result_mutex);
                result.insert({x, y});
              }
            }
          }
        }
      }));
  }

  for (auto &&thread : thread_pool) {
    thread.join();
  }
  auto stop = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration<double>(stop - start);
  std::cout << std::to_string(elapsed.count()) + ",";

  return result;
}

PixelPositions NDWICalculator::localize_water(unsigned int cores, const TiffImage &green_layer,
                                              const TiffImage &nir_layer,
                                              const PixelPositions &omitted_pixels) {
  std::vector<std::thread> thread_pool;
  std::mutex result_mutex;
  PixelPositions result;
  for (unsigned int i = 0UL; i < cores; ++i) {
    thread_pool.emplace_back(
      std::thread([i, cores, &result, &green_layer, &nir_layer, &result_mutex, &omitted_pixels]() {
        unsigned int height = green_layer.height() / cores;
        unsigned int start = i > 0 ? height * i : height * i + 1;
        unsigned int stop = i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

        for (unsigned int y = start; y < stop - 1; ++y) {
          for (unsigned int x = 0; x < static_cast<unsigned int>(green_layer.width()); ++x) {
            if (omitted_pixels.count({x, y})) {
              continue;
            }

            float green_value = green_layer(x, y);
            float nir_value = nir_layer(x, y);

            if (green_value > 1.f && nir_value > 1.f) {
              float ndwi_level = (green_value - nir_value) / (green_value + nir_value);
              if (ndwi_level >= 0.33f) {
                std::lock_guard<std::mutex> guard(result_mutex);
                result.insert({x, y});
              }
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

TiffImage NDWICalculator::generate_ndwi_layer(const TiffImage &img1, const TiffImage &img2,
                                              Method method) {
  switch (method) {
    case Method::GreenNir:
      return generate_ndwi_layer_green_nir(img1, img2);
    case Method::NirSwir:
      return generate_ndwi_layer_nir_swir(img1, img2);
    default:
      std::cerr << "NDWI Calculator: Invalid method!" << std::endl;
      std::exit(1);
  }
}

TiffImage NDWICalculator::generate_ndwi_layer_high_performance(const TiffImage &img1,
                                                               const TiffImage &img2, Method method,
                                                               unsigned int cores) {
  switch (method) {
    case Method::GreenNir:
      return generate_ndwi_layer_green_nir_high_performance(img1, img2, cores);
    case Method::NirSwir:
      // return generate_ndwi_layer_nir_swir_high_performance(img1, img2, cores);
    default:
      std::cerr << "NDWI Calculator - High performance: Invalid method!" << std::endl;
      std::exit(1);
  }
}