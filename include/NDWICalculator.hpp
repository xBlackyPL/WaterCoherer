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

#include "CImage.hpp"
#include "WaterCohererTypes.hpp"
#include "WaterDifferencer.hpp"

/*
    NDWI = (Xnir - Xswir)/(Xnir + Xswir)
    NDWI = (Xgreen - Xnir)/(Xgreen + Xnir)

    Band 10 - Blue
    Band 20 - Green
    Band 30 - Red
    Band 40 - Near Infrared (NIR)
    Band 50 - Shortwave Infrared (SWIR)
    Band 60 - Thermal
    Band 70 - Shortwave Infrared (SWIR)
    Band 80 - Panchromatic
*/

namespace WaterCoherer {
    class NDWICalculator {
    private:
        static auto generate_ndwi_layer_nir_swir(TiffImage, TiffImage) -> TiffImage;

        static auto generate_ndwi_layer_green_nir(TiffImage, TiffImage)
        -> TiffImage;

        static auto generate_ndwi_layer_nir_swir_high_performance(TiffImage,
                                                                  TiffImage,
                                                                  unsigned int)
        -> TiffImage;

        static auto generate_ndwi_layer_green_nir_high_performance(TiffImage,
                                                                   TiffImage,
                                                                   unsigned int)
        -> TiffImage;

    public:
        enum class Method {
            GreenNir, NirSwir
        };

        static auto generate_ndwi_layer(TiffImage, TiffImage, Method) -> TiffImage;

        static auto generate_ndwi_layer_high_performance(TiffImage, TiffImage,
                                                         Method, unsigned int)
        -> TiffImage;

        static auto localize_water(TiffImage, TiffImage, unsigned int)
        -> WaterLocalization;
    };
}  // namespace WaterCoherer
