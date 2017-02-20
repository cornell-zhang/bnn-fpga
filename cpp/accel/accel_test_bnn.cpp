#include <cstddef>
#include <cstdlib>
#include <hls_video.h>

#include "Accel.h"
#include "AccelSchedule.h"
#include "AccelTest.h"
#include "Dense.h"
#include "ZipIO.h"
#include "ParamIO.h"
#include "DataIO.h"
#include "Timer.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf ("Give number of images to test as 1st arg\n");
    return 0;
  }
  const unsigned n_imgs = std::stoi(argv[1]);

  const unsigned lconv  = 6;  // last conv
  const unsigned ldense = 8;  // last dense

  // print some config numbers
  printf ("* WT_WORDS   = %u\n", WT_WORDS);
  printf ("* KH_WORDS   = %u\n", KH_WORDS);

  // Load input data
  printf ("## Loading input data ##\n");
  Cifar10TestInputs X(n_imgs);
  Cifar10TestLabels y(n_imgs);

  // Load parameters
  printf ("## Loading parameters ##\n");
  Params params(get_root_dir() + "/params/cifar10_parameters_nb.zip");

  // ---------------------------------------------------------------------
  // allocate and binarize all weights
  // ---------------------------------------------------------------------
  Word* wt[N_LAYERS];
  Word* kh[N_LAYERS];
  for (unsigned l = 0; l < N_LAYERS; ++l) {
    const unsigned M = M_tab[l];
    const unsigned N = N_tab[l];
    if (layer_is_conv(l+1))
      wt[l] = new Word[WTS_TO_WORDS(M*N)];
    else
      wt[l] = new Word[M*N / WORD_SIZE];
    const float* weights = params.float_data(widx_tab[l]);
    set_weight_array(wt[l], weights, l+1);

    kh[l] = new Word[N/KH_PER_WORD * sizeof(Word)];
    const float* k = params.float_data(kidx_tab[l]);
    const float* h = params.float_data(hidx_tab[l]);
    set_bnorm_array(kh[l], k, h, l+1);
  }

  // ---------------------------------------------------------------------
  // // compute accelerator schedule (divides up weights)
  // ---------------------------------------------------------------------
  AccelSchedule layer_sched[N_LAYERS];
  for (unsigned l = 0; l < N_LAYERS; ++l) {
    compute_accel_schedule(
        wt[l], kh[l],
        M_tab[l], N_tab[l], S_tab[l], T_tab[l], pool_tab[l],
        layer_sched[l]
    );
  }

  // allocate memories for data i/o for the accelerator
  Word* data_i  = (Word*) MEM_ALLOC( DMEM_I_WORDS * sizeof(Word) );
  Word* data_o  = (Word*) MEM_ALLOC( DMEM_O_WORDS * sizeof(Word) );
  if (!data_i || !data_o) {
    fprintf (stderr, "**** ERROR: Alloc failed in %s\n", __FILE__);
    return (-2);
  }

  unsigned n_errors = 0;
  Timer t_accel("accel");
  Timer t_total("total");

  printf ("## Running BNN for %d images\n", n_imgs);

  //--------------------------------------------------------------
  // Run BNN
  //--------------------------------------------------------------
  for (unsigned n = 0; n < n_imgs; ++n) {
    float* data = X.data + n*3*32*32;
    binarize_input_images(data_i, data, 32);

    t_total.start();

    //------------------------------------------------------------
    // Execute conv layers
    //------------------------------------------------------------
    for (unsigned l = 1; l <= lconv; ++l) {
      const unsigned M = M_tab[l-1];
      const unsigned N = N_tab[l-1];
      const unsigned S = S_tab[l-1];
      unsigned input_words = (l==1) ? S*S : M*S*S/WORD_SIZE;

      t_accel.start();

      run_accel_schedule(
          data_i, data_o,
          l-1,        // layer_idx
          (l==1) ? input_words : 0,
          l % 2,      // mem_mode
          layer_sched[l-1]
      );

      t_accel.stop();
    }

    //------------------------------------------------------------
    // Execute dense layers
    //------------------------------------------------------------
    for (unsigned l = lconv+1; l <= ldense; ++l) {
      const unsigned M = M_tab[l-1];
      const unsigned N = N_tab[l-1];

      t_accel.start();

      run_accel_schedule(
          data_i, data_o,
          l-1,
          0,      // input_words
          l % 2,  // mem_mode
          layer_sched[l-1]
      );

      t_accel.stop();
    }

    //------------------------------------------------------------
    // Execute last layer
    //------------------------------------------------------------
    int prediction = -1;
    t_accel.start();

    run_accel_schedule(
        data_i, data_o,
        ldense,
        0,      // input_words
        1,      // mem_mode
        layer_sched[ldense]
    );

    t_accel.stop();
    t_total.stop();

    ap_int<8> p = 0;
    p(7,0) = data_o[0](7,0);
    prediction = p.to_int();

    //assert(prediction >= 0 && prediction <= 9);
    int label = y.data[n];

    printf ("  Pred/Label:\t%2u/%2d\t[%s]\n", prediction, label,
        ((prediction==label)?" OK ":"FAIL"));

    n_errors += (prediction!=label);
  }

  printf ("\n");
  printf ("Errors: %u (%4.2f%%)\n", n_errors, float(n_errors)*100/n_imgs);
  printf ("\n");

  MEM_FREE( data_o );
  MEM_FREE( data_i );
  for (unsigned n = 0; n < N_LAYERS; ++n) {
    delete[] wt[n];
    delete[] kh[n];
  }
  return 0;
}
