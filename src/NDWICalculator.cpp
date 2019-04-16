#include <utility>

#include <utility>

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

#include "../include/NDWICalculator.hpp"

#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <list>

using namespace WaterCoherer;

auto NDWICalculator::generate_ndwi_layer(TiffImage img1, TiffImage img2,
                                         Method method) -> TiffImage {
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

auto NDWICalculator::generate_ndwi_layer_high_performance(TiffImage img1,
                                                          TiffImage img2,
                                                          Method method,
                                                          unsigned int cores)
-> TiffImage {
    switch (method) {
        case Method::GreenNir:
            return generate_ndwi_layer_green_nir_high_performance(img1, img2,
                                                                  cores);
        case Method::NirSwir:
            return generate_ndwi_layer_nir_swir_high_performance(img1, img2,
                                                                 cores);
        default:
            std::cerr << "NDWI Calculator - High performanace: Invalid method!"
                      << std::endl;
            std::exit(1);
    }
}

auto NDWICalculator::generate_ndwi_layer_nir_swir(TiffImage nir_layer,
                                                  TiffImage swir_layer)
-> TiffImage {
    TiffImage result(static_cast<const unsigned int>(nir_layer.width()),
                     static_cast<const unsigned int>(nir_layer.height()),
                     static_cast<const unsigned int>(nir_layer.depth()),
                     1);

    if (nir_layer.height() != swir_layer.height() ||
        nir_layer.width() != swir_layer.width()) {
        return result;
    }

    cimg_forXY(result, x, y) {
            float nir_value = nir_layer(static_cast<const unsigned int>(x),
                                        static_cast<const unsigned int>(y));
            float swir_value = swir_layer(static_cast<const unsigned int>(x),
                                          static_cast<const unsigned int>(y));
            if (nir_value < 1.f || swir_value < 1.f) {
                result(static_cast<const unsigned int>(x), static_cast<const unsigned int>(y)) = 0;
            } else {
                float ndwi_level =
                        (nir_value - swir_value) / (nir_value + swir_value);

                if (ndwi_level > 1.f) {
                    result(static_cast<const unsigned int>(x),
                           static_cast<const unsigned int>(y)) = static_cast<unsigned char>(255);
                } else {
                    result(static_cast<const unsigned int>(x),
                           static_cast<const unsigned int>(y)) = 0;
                }
            }
        }
    return result;
}

auto NDWICalculator::generate_ndwi_layer_green_nir(TiffImage green_layer,
                                                   TiffImage nir_layer)
-> TiffImage {
    TiffImage result(static_cast<const unsigned int>(green_layer.width()),
                     static_cast<const unsigned int>(green_layer.height()),
                     static_cast<const unsigned int>(green_layer.depth()), 1);

    if (green_layer.height() != green_layer.height() ||
        green_layer.width() != green_layer.width()) {
        return result;
    }

    cimg_forXY(result, x, y) {
            float green_value = green_layer(static_cast<const unsigned int>(x),
                                            static_cast<const unsigned int>(y));
            float nir_value = nir_layer(static_cast<const unsigned int>(x),
                                        static_cast<const unsigned int>(y));
            if (green_value < 1.f || nir_value < 1.f) {
                result(static_cast<const unsigned int>(x), static_cast<const unsigned int>(y)) = 0;
            } else {
                float ndwi_level =
                        (green_value - nir_value) / (green_value + nir_value);

                if (ndwi_level >= 0.3f) {
                    result(static_cast<const unsigned int>(x),
                           static_cast<const unsigned int>(y)) = static_cast<unsigned char>(255);
                } else {
                    result(static_cast<const unsigned int>(x),
                           static_cast<const unsigned int>(y)) = 0;
                }
            }
        }
    return result;
}

auto NDWICalculator::generate_ndwi_layer_nir_swir_high_performance(
        TiffImage nir_layer, TiffImage swir_layer, unsigned int cores)
-> TiffImage {
    // TODO: Implement high_performance method
    return generate_ndwi_layer_green_nir(std::move(nir_layer), std::move(swir_layer));
}

auto NDWICalculator::generate_ndwi_layer_green_nir_high_performance(
        TiffImage green_layer, TiffImage nir_layer, unsigned int cores)
-> TiffImage {
    std::vector<std::thread> thread_pool;
    TiffImage result(static_cast<const unsigned int>(green_layer.width()),
                     static_cast<const unsigned int>(green_layer.height()),
                     static_cast<const unsigned int>(green_layer.depth()), 1);

    for (unsigned int i = 0UL; i < cores; ++i) {
        thread_pool.emplace_back(
                std::thread([i, cores, &result, &green_layer, &nir_layer]() {
                    unsigned int height = green_layer.height() / cores;
                    unsigned int start = i > 0 ? height * i : height * i + 1;
                    unsigned int stop =
                            i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

                    for (unsigned int y = start; y < stop - 1; ++y) {
                        for (unsigned int x = 0; x < static_cast<unsigned int>(green_layer.width());
                             ++x) {
                            float green_value = green_layer(x, y);
                            float nir_value = nir_layer(x, y);

                            if (green_value > 1.f && nir_value > 1.f) {
                                float ndwi_level = (green_value - nir_value) /
                                                   (green_value + nir_value);
                                result(x, y) =
                                        static_cast<unsigned char>(ndwi_level >= 0.35f ? 255 : 0);
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

auto NDWICalculator::localize_water(TiffImage green_layer, TiffImage nir_layer,
                                    unsigned int cores) -> WaterLocalization {
    std::vector<std::thread> thread_pool;
    std::mutex result_mutex;
    WaterLocalization result;

    for (unsigned int i = 0UL; i < cores; ++i) {
        thread_pool.emplace_back(
                std::thread([i, cores, &result, &green_layer, &nir_layer, &result_mutex]() {
                    unsigned int height = green_layer.height() / cores;
                    unsigned int start = i > 0 ? height * i : height * i + 1;
                    unsigned int stop =
                            i < cores - 1 ? height * (i + 1) + 1 : (height * (i + 1));

                    for (unsigned int y = start; y < stop - 1; ++y) {
                        for (unsigned int x = 0; x < static_cast<unsigned int>(green_layer.width());
                             ++x) {
                            float green_value = green_layer(x, y);
                            float nir_value = nir_layer(x, y);
                            if (green_value > 1.f && nir_value > 1.f) {
                                float ndwi_level = (green_value - nir_value) /
                                                   (green_value + nir_value);
                                if (ndwi_level >= 0.35f) {
                                    std::lock_guard<std::mutex> guard(result_mutex);
                                    result.emplace_back(std::make_pair(x, y));
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