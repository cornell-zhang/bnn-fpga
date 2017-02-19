#include <cstddef>
#include <hls_video.h>

#include "Accel.h"
#include "AccelSchedule.h"
#include "AccelTest.h"
#include "Dense.h"
#include "ZipIO.h"
#include "ParamIO.h"
#include "DataIO.h"

int main(int argc, char** argv) {
  #ifdef HLS_COMPILE
  const unsigned l = 2;
  #else
  if (argc < 2) {
    printf ("Requires layer number as the first argument\n");
    exit(-1);
  }
  const unsigned l = atoi(argv[1]);
  #endif

  assert (l < N_LAYERS);

  const unsigned lconv  = 6;  // last conv

  const unsigned Si = S_tab[l-1];
  const unsigned So = S_tab[l];
  const unsigned M = M_tab[l-1];
  const unsigned N = N_tab[l-1];
  const unsigned wt_size = (layer_is_conv(l)) ? WTS_TO_WORDS(M*N) : M*N/WORD_SIZE;
  const unsigned kh_size = N/KH_PER_WORD;

  Word* wt      = new Word[wt_size];
  Word* kh      = new Word[kh_size];
  Word* data_i  = (Word*) MEM_ALLOC( DMEM_WORDS * sizeof(Word) );
  Word* data_o  = (Word*) MEM_ALLOC( N*So*So/WORD_SIZE * sizeof(Word) );
  if (!wt || !kh || !data_i || !data_o) {
    fprintf (stderr, "**** ERROR: Alloc failed in %s\n", __FILE__);
    return (-2);
  }
  for (unsigned i = 0; i < wt_size; ++i)
    wt[i] = 0;
  for (unsigned i = 0; i < kh_size; ++i)
    kh[i] = 0;

  printf ("## Testing Layer %u with %u outputs ##\n", l, N);

  // Load reference output from zip and set data_i
  printf ("## Loading test data ##\n");
  if (l == 1) {
    Cifar10TestInputs X(1);
    binarize_input_images(data_i, X.data, Si);
  } else {
    const float* input_maps = new float[M*Si*Si];
    std::string l_type = layer_is_conv(l) ? "/data/cpp_conv" : "/data/cpp_dense";
    unsigned l_num = layer_is_conv(l) ? l-1 : l-L_CONV-1;
    std::string input_file = get_root_dir() + l_type + std::to_string(l_num) + "_maps.zip";
    unzip_to_array(input_file, input_maps);
    set_bit_array(data_i, input_maps, M*Si*Si);
    delete[] input_maps;
  }

  // Binarize weights
  printf ("## Loading parameters ##\n");
  Params params(get_root_dir() + "/params/cifar10_parameters_nb.zip");
  const float* weights = params.float_data(widx_tab[l-1]);
  set_weight_array(wt, weights, l);

  // Binarize batch-norm parameters
  const float* k = params.float_data(kidx_tab[l-1]);
  const float* h = params.float_data(hidx_tab[l-1]);
  set_bnorm_array(kh, k, h, l);

  // Load binary ref
  Word* bin_ref = new Word[N*So*So/WORD_SIZE];
  if (layer_is_last(l)) {
    bin_ref[0] = 3;
  } else {
    const float* output_maps = new float[N*So*So];
    std::string l_type = layer_is_conv(l) ? "/data/cpp_conv" : "/data/cpp_dense";
    unsigned l_num = layer_is_conv(l) ? l : l-L_CONV;
    std::string output_file = get_root_dir() + l_type + std::to_string(l_num) + "_maps.zip";
    unzip_to_array(output_file, output_maps);
    set_bit_array(bin_ref, output_maps, N*So*So);
    delete[] output_maps;
  }

  // Perform test
  if (layer_is_conv(l)) {
    test_conv_layer(
        wt, kh, data_i, data_o,
        NULL, bin_ref,
        M, N, Si,
        (l==1) ? 0 : 1,   // conv_mode
        pool_tab[l-1]     // max_pool
      );
  } else {
    test_dense_layer(
        wt, kh, data_i, data_o,
        bin_ref,
        M, N
      );
  }

  printf ("Tests passed!\n");

  delete[] bin_ref;
  MEM_FREE( data_o );
  MEM_FREE( data_i );
  delete[] kh;
  delete[] wt;
  return 0;
}
