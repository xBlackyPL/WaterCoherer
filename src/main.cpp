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

#include <iostream>
#include <chrono>
#include "NDWICalculator.hpp"

using namespace WaterCoherer;

int main(int argc, char const *argv[]) {
    TiffImage green_layer;
    green_layer.load_tiff(
        "../data/LE71880252009264ASN00/L71188025_02520090921_B20.TIF");

    TiffImage nir_layer;
    nir_layer.load_tiff(
        "../data/LE71880252009264ASN00/L71188025_02520090921_B40.TIF");

    auto start = std::chrono::system_clock::now();
    TiffImage result = NDWICalculator::generate_ndiw_layer_high_performance(
        green_layer, nir_layer, Method::GreenNir,8);
    auto stop = std::chrono::system_clock::now();

    result.save("result.tiff");

    std::chrono::duration<double> elapsed_time = stop-start;
    std::cout << elapsed_time.count() << std::endl;

    return 0;
}