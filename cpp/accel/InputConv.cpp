#include "InputConv.h"
#include "AccelTest.h"
#include "Timer.h"

void run_input_conv_layer(
    const float* w_data,
    const float* k_data,
    const float* h_data,
    const float* data_i,
    Word* data_o,
    const unsigned M,
    const unsigned N
) {
  const unsigned S = 32;

  std::vector<bool> w;
  w.reserve(M*N*K*K);
  for (unsigned i = 0; i < M*N*K*K; ++i) {
    w[i] = (w_data[i] >= 0) ? false : true;
  }

  float conv_buffer[S*S];
  float in_buffer[(S+2)*(S+2)];

  // ------------------------------------------
  // assign 0 to the boundaries
  // ------------------------------------------
  for (unsigned c = 0; c < S+2; ++c)
    in_buffer[c] = 0;
  for (unsigned r = 1; r < S+1; ++r) {
    in_buffer[r*(S+2) + 0] = 0;
    in_buffer[r*(S+2) + S+1] = 0;
  }
  for (unsigned c = 0; c < S+2; ++c)
    in_buffer[(S+1)*(S+2) + c] = 0;

  static Timer t_conv1("conv1");
  t_conv1.start();

  // ------------------------------------------
  // Main conv loop
  // ------------------------------------------
  for (unsigned n = 0; n < N; ++n) {
    // clear conv_buffer
    for (unsigned i = 0; i < S*S; ++i) {
      conv_buffer[i] = 0;
    }

    // Loop over all input images
    for (unsigned m = 0; m < M; ++m) {
      const unsigned w_n = n*M + m;

      // copy input
      for (unsigned r = 1; r < S+1; ++r) {
        for (unsigned c = 1; c < S+1; ++c) {
          in_buffer[r*(S+2) + c] = data_i[m*S*S + (r-1)*S + (c-1)];
      } }

      // operate on 1 input image
      for (int r = 0; r < S; ++r) {
      for (int c = 0; c < S; ++c) {
        float res = 0;

        // perform convolution
        for (int kr = 0; kr < K; ++kr) {
        for (int kc = 0; kc < K; ++kc) {
          float pix = in_buffer[(r+kr)*(S+2) + (c+kc)];
          const bool b = w[w_n*K*K + (8-(kr*K+kc))];
          res += (b==0) ? pix : -pix;
        } } // kr,kc

        conv_buffer[r*S + c] += res;
      } } // r,c of input img
    } // m

    // perform batch-norm
    for (unsigned i = 0; i < S*S; i+=WORD_SIZE) {
      Word out_wrd = 0;
      for (unsigned b = 0; b < WORD_SIZE; ++b) {
        float x = static_cast<float>(conv_buffer[i+b]);
        x = x * k_data[n] + h_data[n];
        out_wrd[b] = (x >= 0) ? 0 : 1;
      }
      data_o[(n*S*S + i)/WORD_SIZE] = out_wrd;
    }
  } // n

  t_conv1.stop();
}
