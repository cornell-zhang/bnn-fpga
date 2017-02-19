#include "AccelPrint.h"

void print_params3d(Word in[], unsigned M, unsigned num) {
  unsigned addr = M / CONV_W_PER_WORD;
  unsigned off = M % CONV_W_PER_WORD;
  for (unsigned n = M; n < M+num; ++n) {
    print_bits(in, addr*WORD_SIZE+off*WT_SIZE, 3, 3, 3);
    printf ("--%u--\n", n+1);
    if (++off == CONV_W_PER_WORD) {
      off = 0;
      addr++;
    }
  }
}

void print_wt_word(const Word& in) {
  assert(K*K == WT_SIZE);
  for (unsigned i = 0; i < CONV_W_PER_WORD; ++i) {
    for (unsigned r = 0; r < K; ++r) {
      for (unsigned c = 0; c < K; ++c) {
        printf ("%3d", in[i*WT_SIZE + r*K + c] == 0 ? 0 : 1);
      }
      printf ("\n");
    }
    printf ("--%u/%u--\n", i+1, CONV_W_PER_WORD);
  }
}

void print_wt(const WtType& in) {
  for (unsigned r = 0; r < K; ++r) {
    for (unsigned c = 0; c < K; ++c) {
      printf ("%3d", in[r*K + c] == 0 ? 0 : 1);
    }
    printf ("\n");
  }
}

