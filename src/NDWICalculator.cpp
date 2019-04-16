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

#define cimg_use_tiff
#include "NDWICalculator.hpp"
#include <memory>
#include <thread>
#include <vector>

using namespace WaterCoherer;

auto NDWICalculator::generate_ndwi_layer(TiffImage img1, TiffImage img2,
                                         Method method) -> TiffImage {
    switch (method) {
        case Method::GreenNir:
            return generate_ndwi_layer_green_nir(img1, img2);
            break;
        case Method::NirSwir:
            return generate_ndwi_layer_nir_swir(img1, img2);
            break;
        default:
            std::cerr << "NDWI Calculator: Invalid method!" << std::endl;
            std::exit(1);
    }
}

auto NDWICalculator::generate_ndiw_layer_high_performance(TiffImage img1,
                                                          TiffImage img2,
                                                          Method method,
                                                          unsigned int cores)
    -> TiffImage {
    switch (method) {
        case Method::GreenNir:
            return generate_ndwi_layer_green_nir_high_performance(img1, img2,
                                                                  cores);
            break;
        case Method::NirSwir:
            return generate_ndwi_layer_nir_swir_high_performance(img1, img2,
                                                                 cores);
            break;
        default:
            std::cerr << "NDWI Calculator - High performanace: Invalid method!"
                      << std::endl;
            std::exit(1);
    }
}

auto NDWICalculator::generate_ndwi_layer_nir_swir(TiffImage nir_layer,
                                                  TiffImage swir_layer)
    -> TiffImage {
    TiffImage result(nir_layer.width(), nir_layer.height(), nir_layer.depth(),
                     1);

    if (nir_layer.height() != swir_layer.height() ||
        nir_layer.width() != swir_layer.width()) {
        return result;
    }

    cimg_forXY(result, x, y) {
        float nir_value = nir_layer(x, y);
        float swir_value = swir_layer(x, y);
        if (nir_value < 1.f || swir_value < 1.f) {
            result(x, y) = 0;
        } else {
            float ndwi_level =
                (nir_value - swir_value) / (nir_value + swir_value);

            if (ndwi_level > 1.f) {
                result(x, y) = static_cast<unsigned char>(255);
            } else {
                result(x, y) = 0;
            }
        }
    }
    return result;
}

auto NDWICalculator::generate_ndwi_layer_green_nir(TiffImage green_layer,
                                                   TiffImage nir_layer)
    -> TiffImage {
    TiffImage result(green_layer.width(), green_layer.height(),
                     green_layer.depth(), 1);

    if (green_layer.height() != green_layer.height() ||
        green_layer.width() != green_layer.width()) {
        return result;
    }

    cimg_forXY(result, x, y) {
        float green_value = green_layer(x, y);
        float nir_value = nir_layer(x, y);
        if (green_value < 1.f || nir_value < 1.f) {
            result(x, y) = 0;
        } else {
            float ndwi_level =
                (green_value - nir_value) / (green_value + nir_value);

            if (ndwi_level >= 0.4f) {
                result(x, y) = static_cast<unsigned char>(255);
            } else {
                result(x, y) = 0;
            }
        }
    }
    return result;
}

auto NDWICalculator::generate_ndwi_layer_nir_swir_high_performance(
    TiffImage nir_layer, TiffImage swir_layer, unsigned int cores)
    -> TiffImage {
    // TODO: Implement high_performance method
    return generate_ndwi_layer_green_nir(nir_layer, swir_layer);
}

auto method(TiffImage& result) -> TiffImage { return result; }

auto NDWICalculator::generate_ndwi_layer_green_nir_high_performance(
    TiffImage green_layer, TiffImage nir_layer, unsigned int cores)
    -> TiffImage {
    std::vector<std::thread> threadPool;
    TiffImage result(green_layer.width(), green_layer.height(),
                     green_layer.depth(), 1);

    auto start = std::chrono::system_clock::now();
    for (unsigned int i = 0UL; i < cores; ++i) {
        threadPool.emplace_back(
            std::thread([i, cores, &result, &green_layer, &nir_layer]() {
                int height = green_layer.height() / cores;
                int start = i > 0 ? height * i : height * i + 1;
                int stop =
                    i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

                for (int y = start; y < stop - 1; ++y) {
                    for (int x = 0; x < green_layer.width(); ++x) {
                        float green_value = green_layer(x, y);
                        float nir_value = nir_layer(x, y);
                        if (green_value > 1.f && nir_value > 1.f) {
                            float ndwi_level = (green_value - nir_value) /
                                               (green_value + nir_value);
                            result(x, y) = ndwi_level >= 0.4f ? 255 : 0;
                        }
                    }
                }
            }));
    }

    for (auto&& thread : threadPool) {
        thread.join();
    }
    auto stop = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_time = stop - start;
    std::cout << elapsed_time.count() << std::endl;
    return result;
}
