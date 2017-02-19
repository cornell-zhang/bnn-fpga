#include "AccelTest.h"
#include "AccelSchedule.h"
#include <math.h>
#include <cstdlib>

//------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------

bool layer_is_conv(unsigned layer_idx) {
  return layer_is_binconv(layer_idx) || layer_is_fpconv(layer_idx);
}
bool layer_is_binconv(unsigned layer_idx) {
  assert(layer_idx != 0 && layer_idx <= N_LAYERS);
  return T_tab[layer_idx-1] == LAYER_CONV;
}
bool layer_is_fpconv(unsigned layer_idx) {
  assert(layer_idx != 0 && layer_idx <= N_LAYERS);
  return T_tab[layer_idx-1] == LAYER_CONV1;
}
bool layer_is_last(unsigned layer_idx) {
  assert(layer_idx != 0 && layer_idx <= N_LAYERS);
  return T_tab[layer_idx-1] == LAYER_LAST;
}
bool layer_wt_size(unsigned layer_idx);
bool layer_kh_size(unsigned layer_idx);

// Simple log function, only works for powers of 2
unsigned log2(unsigned x) {
  unsigned res = 0;
  while (x != 1) {
    x = x >> 1;
    res += 1;
  }
  return res;
}

// number of Words allocated to store n conv weights
unsigned WTS_TO_WORDS(const unsigned n) {
  // divide n weights by W_PER_WORD
  const unsigned words = (n + CONV_W_PER_WORD-1) / CONV_W_PER_WORD;
  // round up to nearest convolvers
  return ((words+CONVOLVERS-1) / CONVOLVERS) * CONVOLVERS;
}

//------------------------------------------------------------------------
// Binarize weights and pack them into Words
//------------------------------------------------------------------------
void set_weight_array(Word* w, const float* wts, unsigned layer_idx) {
  const unsigned M = M_tab[layer_idx-1];
  const unsigned N = N_tab[layer_idx-1];

  if (layer_is_conv(layer_idx)) {
    set_conv_weight_array(w, wts, M*N);
  } else {
    set_dense_weight_array(w, wts, M, N);
  }
}

void set_conv_weight_array(Word* w, const float* wts, unsigned size) {
  unsigned wrd = 0, off = 0;
  for (unsigned m = 0; m < size; ++m) {
    for (unsigned i = 0; i < WT_SIZE; ++i) {
      set_bit(w, wrd*WORD_SIZE+off*WT_SIZE+i, wts[m*WT_SIZE+i]>=0 ? Bit(0) : Bit(-1));
    }
    if (++off == CONV_W_PER_WORD) {
      off = 0;
      wrd++;
    }
  }
}

void set_dense_weight_array(Word* w, const float* wts, unsigned M, unsigned N) {
  unsigned w_idx = 0;
  for (unsigned n = 0; n < N; ++n) {
    for (unsigned m = 0; m < M; m+=WORD_SIZE) {
      Word wrd = 0;
      for (unsigned b = 0; b < WORD_SIZE; ++b) {
        wrd[b] = ((wts[(m+b)*N+n] < 0) ? 1 : 0);
      }
      w[w_idx] = wrd;
      ++w_idx;
    }
  }
}

//------------------------------------------------------------------------
// Binarize and pack the batch norm parameters
//------------------------------------------------------------------------
const int M_INT = 32767;

int round_away_from_zero(float f) {
  return f < 0 ? floor(f) : ceil(f);
}

// compute -(h/k) without floating-point exception
float compute_thresh(const float k, const float h) {
  if (k == 0)
    return (h >= 0) ? -M_INT : M_INT;
  else
    return -(h/k);
}

void set_bnorm_array(Word* kh, const float* k, const float* h, unsigned layer_idx) {
  const unsigned N = N_tab[layer_idx-1];
  if (!layer_is_last(layer_idx)) {
    set_bnorm_array1(kh, k, h, layer_idx, N);
  } else {
    set_bnorm_array2(kh, k, h, N);
  }
}

void set_bnorm_array1(Word* kh, const float* k, const float* h, unsigned layer_idx, unsigned N) {
  for (unsigned n = 0; n < N; ++n) {
    NormComp comp;
    if (layer_is_fpconv(layer_idx)) {
      // fixed point number for first conv layer
      C1Comp fi = compute_thresh(k[n], h[n]);
      comp(15,0) = fi(15,0);
    } else {
      // integer number for all other layer, round away from 0
      comp = round_away_from_zero( compute_thresh(k[n], h[n]) );
    }

    int off = n % KH_PER_WORD;
    Word w = kh[n/KH_PER_WORD];
    w((off+1)*16-1, off*16) = comp(15,0);
    kh[n/KH_PER_WORD] = w;
  }
}

void set_bnorm_array2(Word* kh, const float* k, const float* h, unsigned N) {
  for (unsigned n = 0; n < N; ++n) {
    KType ki = k[n];
    HType hi = h[n];
    //printf ("** ki=%f, hi=%f\n", ki.to_float(), hi.to_float());
    Word w = kh[n/2];
    if (n % 2 == 0) {
      w(15, 0) = ki(15,0);
      w(31,16) = hi(15,0);
    } else {
      w(47,32) = ki(15,0);
      w(63,48) = hi(15,0);
    }
    kh[n/2] = w;
  }
}

//------------------------------------------------------------------------
// Binarize the input image
//------------------------------------------------------------------------
void binarize_input_images(Word* dmem_i, const float* inputs, unsigned S) {
  // N is the number of images, S is the image width in pixels
  // Assume [in[ut] is a 3 channel non-interleaved image
  // We pack 3 interleaved pixels into each word
  const unsigned C = 3;
  const unsigned W = C1InputType(0).length();
  assert(W <= WORD_SIZE/C);
  for (unsigned s = 0; s < S*S; ++s) {
    Word wrd = 0;
    for (unsigned c = 0; c < C; ++c) {
      C1InputType t1 = inputs[c*S*S+s];
      unsigned offset = W*c;
      wrd(W-1+offset, offset) = t1(W-1, 0);
    }
    dmem_i[s] = wrd;
  }
}

//------------------------------------------------------------------------
// Padded convolution
//------------------------------------------------------------------------
void padded_conv(Word in[], Word w[], Word out[],
                 unsigned M, unsigned S)
{
  for (int r = 0; r < S; ++r) {
  for (int c = 0; c < S; ++c) {
    out[r*S + c] = 0;
  }}

  for (int m = 0; m < M; ++m) {
    for (int r = 0; r < S; ++r) {
    for (int c = 0; c < S; ++c) {
      Word res = 0;
      for (int kr = 0; kr < K; ++kr) {
      for (int kc = 0; kc < K; ++kc) {
        TwoBit pix = 0;
        int _r = r+kr-K/2;
        int _c = c+kc-K/2;
        if (_r >= 0 && _c >= 0 && _r < S && _c < S)
          pix = get_bit(in, m*S*S+_r*S+_c) == 0 ? 1 : -1;

        Address kaddr = m/CONV_W_PER_WORD;
        IdxType koff = m%CONV_W_PER_WORD;
        Bit k = w[kaddr][koff*K*K + (2-kr)*K + (2-kc)];

        res += (k!=0) ? (TwoBit)(-pix) : pix;
      } }
      out[r*S + c] += res;
    } }
  }
}

//------------------------------------------------------------------------
// Helper test function for the accelerator conv layers
//------------------------------------------------------------------------
void test_conv_layer(
    Word* weights,
    Word* kh,
    Word* data_i,
    Word* data_o,
    Word* conv_ref,
    Word* bin_ref,
    const unsigned M,
    const unsigned N,
    const unsigned Si,
    const ap_uint<1> conv_mode, // 0=conv1, 1=conv
    const ap_uint<1> max_pool
) {
  printf ("#### Testing convolution with %u inputs, width %u ####\n", M, Si);
  unsigned So = max_pool ? Si/2 : Si;
  unsigned input_words = conv_mode==0 ? Si*Si : M*Si*Si/WORD_SIZE;
  unsigned output_words = N*So*So/WORD_SIZE;
  if (output_words < 1) output_words = 1;
  assert (input_words <= DMEM_WORDS);
  //assert (output_words <= DMEM_O_WORDS);

  DB(3,
    printf ("*data*:\n");
    print_bits3d(data_i, 0, 1, Si, 6,Si);
    printf ("*params*:\n");
    print_params3d(weights, 0, 15);
  )

  AccelSchedule sched;
  compute_accel_schedule(
      weights, kh,
      M, N, Si,
      conv_mode.to_int(),
      max_pool,
      sched
  );

  run_accel_schedule(
      data_i, data_o,
      0,      // layer_idx
      input_words,
      output_words,
      0,      // dmem_mode
      sched
  );

  // print results
  printf ("*bin out*:\n");
  print_bits3d(data_o, 0, 1, So, 8,So);
  printf ("*bin ref*:\n");
  print_bits3d(bin_ref, 0, 1, So, 8,So);

  // Compare bin results
  printf ("## Checking results ##\n");
  unsigned n_err = 0;
  for (unsigned n = 0; n < N; ++n) {
    for (unsigned r = 0; r < So; ++r) {
      for (unsigned c = 0; c < So; ++c) {
        if (get_bit(data_o, n*So*So+r*So+c) != get_bit(bin_ref, n*So*So+r*So+c)) {
          n_err++;
          //printf ("bin out != ref at n=%d, (%d,%d)\n", n, r,c);
          //if (n_err > 64) exit(-1);
        }
      }
    }
  }
  float err_rate = float(n_err) / (N*So*So)*100;
  printf ("Error rate: %7.4f%%\n", err_rate);
  assert(err_rate < 1.0);
}

//------------------------------------------------------------------------
// Helper test function for the accelerator dense layers
//------------------------------------------------------------------------
void test_dense_layer(
    Word* weights,
    Word* kh,
    Word* data_i,
    Word* data_o,
    Word* bin_ref,
    const unsigned M,   // pixels
    const unsigned N    // pixels
) {
  printf ("#### Testing dense layer with %u inputs, %u outputs ####\n", M, N);
  DB(3,
    printf ("*data*:\n");
    print_bits(data_i, 0, 16, 8, 16);
    printf ("*params*:\n");
    print_bits(weights, 0, 16, 8, 16);
  )

  AccelSchedule sched;
  compute_accel_schedule(
      weights, kh,
      M, N, 1,
      2,      // layer_mode
      0,      // norm_mode
      sched
  );

  run_accel_schedule(
      data_i, data_o,
      0,      // layer_idx
      M/WORD_SIZE,
      N/WORD_SIZE,
      0,      // dmem_mode
      sched
  );

  // print results
  printf ("*bin out*:\n");
  print_bits(data_o, 0, 16, 8, 16);
  printf ("*bin ref*:\n");
  print_bits(bin_ref, 0, 16, 8, 16);

  // Compare bin results
  printf ("## Checking results ##\n");
  unsigned n_err = 0;
  for (unsigned n = 0; n < N; ++n) {
    if (get_bit(data_o, n) != get_bit(bin_ref, n)) {
      n_err++;
    }
  }
  float err_rate = float(n_err)/N * 100;
  printf ("Error rate: %7.4f%%\n", err_rate);
  assert(err_rate < 1.0);
}
