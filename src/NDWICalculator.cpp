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
#include <thread>
#include <vector>
#include <memory>

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
        return generate_ndwi_layer_nir_swir_high_performance(img1, img2, cores);
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
    return generate_ndwi_layer_green_nir(nir_layer, swir_layer);
}

void threadMethod(const TiffImage &green_layer, const TiffImage &nir_layer,
                  unsigned char * data, unsigned int index, unsigned int cores) {

    int height = green_layer.height() / cores;

    int start = index > 0 ? height * index : height * index + 1;

    int stop =
        index < cores - 1 ? height * (index + 1) + 1 : (height * (index + 1));

    for (int y = start; y < stop - 1; ++y) {
        for (int x = 0; x < green_layer.width(); ++x) {
            float green_value = green_layer(x, y);
            float nir_value = nir_layer(x, y);
            if (green_value < 1.f || nir_value < 1.f) {
                data[y * green_layer.width() + x] = 0;
            } else {
                float ndwi_level =
                    (green_value - nir_value) / (green_value + nir_value);

                if (ndwi_level >= 0.4f) {
                    data[y * green_layer.width() + x] = static_cast<unsigned char>(255);
                } else {
                    data[y * green_layer.width() + x]= 0;
                }
            }
        }
    }
}

auto NDWICalculator::generate_ndwi_layer_green_nir_high_performance(
    TiffImage green_layer, TiffImage nir_layer, unsigned int cores)
    -> TiffImage {

    auto data = new unsigned char[green_layer.width() * green_layer.height()];
    std::vector<std::thread> threadPool;

    for (unsigned int i = 0UL; i < cores; ++i) {
        threadPool.emplace_back(std::thread(
            [=]() { threadMethod(green_layer, nir_layer, data, i, cores); }));
    }

    for (auto &&thread : threadPool) {
        thread.join();
    }
    TiffImage result(data,green_layer.width(), green_layer.height());
    return result;
}
