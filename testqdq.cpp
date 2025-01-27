#include "DataPort.hpp"
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

void calculate_scale_and_zero_point_4D(NumPyArray* array, float& scale, int8_t& zero_point) {
    if (array == nullptr || array->data == nullptr || array->size == 0) {
        std::cout << "Invalid NumPyArray: Cannot calculate scale and zero point.\n" << std::endl;
        return;
    }

    // Find the minimum and maximum values in the data
    int32_t min_vall = array->data[0];
    int32_t max_vall = array->data[0];

    for (int i = 0; i < array->size; ++i) {
        min_vall = std::min(min_vall, (array->data[i]));
        max_vall = std::max(max_vall, (array->data[i]));
    }

    float min_val = static_cast<float> (min_vall);
    float max_val = static_cast<float> (max_vall);

    scale = (max_val-min_val)/255.0f;
    zero_point = static_cast<int8_t>(std::round(-128.0f-(min_val/scale)));
}

void Quantize(const float& scale, const int8_t& zero_point, const NumPyArray* src, QNumPyArray* dst) {
  if (!src || !src->data || src->size <= 0 || !dst || !dst->data) {
    throw std::invalid_argument("Invalid source or destination NumPyArray.");
  }
  
  dst->size = src->size;
  std::copy(std::begin(src->shape), std::end(src->shape), std::begin(dst->shape));

  for (int i = 0; i < src->size; i++) {
    const float real_val = static_cast<float>(src->data[i]);
    const float transformed_val = zero_point + (real_val / scale);
    const float clamped_val = std::max(-128.0f, std::min(127.0f, transformed_val));
    dst->data[i] = static_cast<int8_t>(std::round(clamped_val));
  }
}

void Dequantize(const float& scale, const int8_t& zero_point, const QNumPyArray* src, NumPyArray* dst) {
  if (!dst || !dst->data || !src || !src->data || src->size <= 0) {
    throw std::invalid_argument("Invalid destination NumPyArray or source QNumPyArray.");
  }

  if (dst->size < src->size) {
    throw std::out_of_range("Destination NumPyArray size is smaller than the source QNumPyArray.");
  }

  std::copy(std::begin(src->shape), std::end(src->shape), std::begin(dst->shape));
  dst->size = src->size;

  for (int i = 0; i < src->size; i++) {
    const int8_t quantized_val = src->data[i];
    dst->data[i] = static_cast<int>(scale * (quantized_val - zero_point));
  }
}

void QuantizeMultiplierSmallerThanOne(float real_multiplier,
                                      std::int32_t* quantized_multiplier,
                                      int* right_shift) {
  int s = 0;
  // We want to bring the real multiplier into the interval [1/2, 1).
  // We can do so by multiplying it by two, and recording how many times
  // we multiplied by two so that we can compensate that by a right
  // shift by the same amount.
  while (real_multiplier < 0.5f) {
    real_multiplier *= 2.0f;
    s++;
  }
  // Now that the real multiplier is in [1/2, 1), we convert it
  // into a fixed-point number.
  std::int64_t q =
      static_cast<std::int64_t>(std::round(real_multiplier * (1ll << 31)));
  // Handle the special case when the real multiplier was so close to 1
  // that its fixed-point approximation was undistinguishable from 1.
  // We handle this by dividing it by two, and remembering to decrement
  // the right shift amount.
  if (q == (1ll << 31)) {
    q /= 2;
    s--;
  }
  *quantized_multiplier = static_cast<std::int32_t>(q);
  *right_shift = s;
}

int32_t MultiplyByQuantizedMultiplier(int32_t x, int32_t quantized_multiplier, int shift) {
  const int64_t total_shift = 31 - shift;
  std::cout << "Total Shift : " << total_shift << std::endl;
  const int64_t round = static_cast<int64_t>(1) << (total_shift - 1);
  std::cout << "Round : " << round << std::endl;
  int64_t result = x * static_cast<int64_t>(quantized_multiplier) + round;
  std::cout << "Init Result  : " << result << std::endl;
  result = result >> total_shift;
  std::cout << "Shifted Result  : " << result << std::endl;

  result = std::clamp(
      result, static_cast<int64_t>(std::numeric_limits<int32_t>::min()),
      static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
  return static_cast<int32_t>(result);
}

int main(){
    int data[5] = {0, 64, 128, 192, 255};
    NumPyArray* input = createNumPyArray(1, 1, 1, 5, data);

    QNumPyArray* quantized = createQNumPyArray(1, 1, 1, 5);

    NumPyArray* dequantized = createNumPyArray(1, 1, 1, 5);

    float scale;
    int8_t zero_point;
    calculate_scale_and_zero_point_4D(input,scale,zero_point);

    std::cout << "Scale : " << scale << std::endl;
    std::cout << "Zero Point : " << static_cast<int>(zero_point) << std::endl;

    Quantize(scale,zero_point,input,quantized);
    printQNumPyArray(quantized);

    Dequantize(scale,zero_point,quantized,dequantized);
    std::cout << "Input : \n\n" << std::endl;
    printNumPyArray(input);
    std::cout << "Dequantized : \n\n" << std::endl;
    printNumPyArray(dequantized);
}