#include "Dense.h"
#include "Timer.h"

const static Word m1("0x5555555555555555", 16);
const static Word m2("0x3333333333333333", 16);
const static Word m4("0x0f0f0f0f0f0f0f0f", 16);
const static Word h01("0x0101010101010101", 16);
static Timer t_dense("dense");
static Timer t_last ("last");

// -----------------------------------------------------------------------
// Performs dense dot product on M input bits, n*M is the weight offset
// -----------------------------------------------------------------------
int dotproduct_m(
    const Word* in,
    const Word* w,
    const unsigned M,
    const unsigned n
) {
  assert (M % WORD_SIZE == 0);
  int sum = 0;

  // Loop across in the inputs in batches of WORD_SIZE
  for (unsigned m = 0; m < M; m+=WORD_SIZE) {
    const Word in_wrd = in[m/WORD_SIZE];
    const Word wt_wrd = w[(n*M+m)/WORD_SIZE];

    Word x = wt_wrd ^ in_wrd;

    // count_set bit for 64 bits, returns 2*cnt
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >> 8;
    x += x >> 16;
    x += x >> 32;
    x = x & 0x7f;

    sum += WORD_SIZE - (x<<1).to_int();
  }
  return sum;
}

// -----------------------------------------------------------------------
// Internal dense layer
// -----------------------------------------------------------------------
void dense_layer_cpu(
    const Word*  wt,
    const float* k_data,
    const float* h_data,
    const Word* in,
    Word* out,
    const unsigned M,
    const unsigned N
) {
  t_dense.start();

  for (unsigned n = 0; n < N; n+=WORD_SIZE) {
    Word out_wrd = 0;
    for (unsigned nb = 0; nb < WORD_SIZE; ++nb) {
      int sum = dotproduct_m(in, wt, M, n+nb);
      float res = static_cast<float>(sum) * k_data[n+nb] + h_data[n+nb];
      if (res < 0)
        out_wrd[nb] = 1;
    }
    out[n/WORD_SIZE] = out_wrd;
  }

  t_dense.stop();
}

// -----------------------------------------------------------------------
// Final dense layer
// -----------------------------------------------------------------------
int last_layer_cpu(
    const Word*  wt,
    const float* k_data,
    const float* h_data,
    const Word* in,
    const unsigned M,
    const unsigned N
) {
  t_last.start();

  int pred = -1;
  float maxval = 0;

  for (unsigned n = 0; n < N; ++n) {
    int sum = dotproduct_m(in, wt, M, n);
    float val = static_cast<float>(sum) * k_data[n] + h_data[n];
    if (pred == -1 || val > maxval) {
      pred = n;
      maxval = val;
    }
  }

  t_last.stop();
  return pred;
}
