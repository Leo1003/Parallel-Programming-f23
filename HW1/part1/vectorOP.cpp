#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  __pp_vec_float clamp = _pp_vset_float(9.999999f);
  __pp_vec_int int_1 = _pp_vset_int(1);
  __pp_vec_int int_0 = _pp_vset_int(0);

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    __pp_mask mask_all = _pp_init_ones(min(VECTOR_WIDTH, N - i));
    __pp_mask mask_active = _pp_init_ones(0);
    __pp_vec_float r, x;
    __pp_vec_int e;

    _pp_vset_float(r, 1.0, mask_all);
    _pp_vload_float(x, values + i, mask_all);
    _pp_vload_int(e, exponents + i, mask_all);

    _pp_vgt_int(mask_active, e, int_0, mask_all);
    while (_pp_cntbits(mask_active)) {
      _pp_vsub_int(e, e, int_1, mask_active);
      _pp_vmult_float(r, r, x, mask_active);
      _pp_vgt_int(mask_active, e, int_0, mask_all);
    }

    _pp_vgt_float(mask_active, r, clamp, mask_all);
    _pp_vmove_float(r, clamp, mask_active);

    _pp_vstore_float(output + i, r, mask_all);
  }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{
  __pp_vec_float r = _pp_vset_float(0.0);
  __pp_mask mask_all = _pp_init_ones();
  __pp_mask mask_first = _pp_init_ones(1);

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    __pp_vec_float x;
    _pp_vload_float(x, values + i, mask_all);
    _pp_vadd_float(r, r, x, mask_all);
  }

  for (int n = VECTOR_WIDTH; n > 1; n /= 2) {
    _pp_hadd_float(r, r);
    _pp_interleave_float(r, r);
  }

  float result = 0.0;
  _pp_vstore_float(&result, r, mask_first);

  return result;
}
