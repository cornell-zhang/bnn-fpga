#=========================================================================
# opt.tcl
#=========================================================================
set_directive_inline conv3x3b
set_directive_inline encode_bit
#set_directive_inline conv_word
set_directive_inline process_word
#set_directive_inline bin_conv
#set_directive_inline fp_conv
#set_directive_inline bin_dense

set_directive_pipeline conv_word
set_directive_pipeline top/LOOP_DMEM_I
set_directive_pipeline top/LOOP_DMEM_O
set_directive_pipeline top/LOOP_WT_I
set_directive_pipeline top/LOOP_KH_I

set_directive_loop_tripcount -min 1 -max 512 top/LOOP_DMEM_I
set_directive_loop_tripcount -min 1 -max 1 top/LOOP_DMEM_O
set_directive_loop_tripcount -min 1 -max 1 top/LOOP_IMG_BATCH
set_directive_loop_tripcount -min 1 -max 512 bin_conv/LOOP_PHASES

# bin_conv/LOOP_WORDS_IN_PHASE
set_directive_pipeline bin_conv/LOOP_WORDS_IN_PHASE
set_directive_loop_tripcount -min 17 -max 32 bin_conv/LOOP_WORDS_IN_PHASE
set_directive_dependence -variable fixed_buffer -type inter -dependent false bin_conv/LOOP_WORDS_IN_PHASE
# bin_conv/LOOP_ACC_PHASES
set_directive_loop_tripcount -min 1 -max 1  bin_conv/LOOP_ACC_PHASES
set_directive_pipeline bin_conv/LOOP_ACC_PHASES_I
set_directive_loop_tripcount -min 1 -max 16 bin_conv/LOOP_ACC_PHASES_I
# bin_conv/LOOP_BATCH_NORM
set_directive_pipeline bin_conv/LOOP_BATCH_NORM
set_directive_loop_tripcount -min 1 -max 16 bin_conv/LOOP_BATCH_NORM
# bin_conv/LOOP_POOL_NORM
set_directive_pipeline bin_conv/LOOP_POOL_NORM
set_directive_loop_tripcount -min 1 -max 16 bin_conv/LOOP_POOL_NORM

# fp_conv/LOOP_FP_CONV_O
set_directive_loop_tripcount -min 1 -max 32 fp_conv/LOOP_FP_CONV_O
# fp_conv/LOOP_RESET_LINEBUFFERS
set_directive_unroll fp_conv/LOOP_RESET_LINEBUFFERS
set_directive_unroll -region fp_conv/LOOP_RESET_LINEBUFFERS
# fp_conv/LOOP_LOAD_WTS
set_directive_unroll fp_conv/LOOP_LOAD_WTS
# fp_conv/LOOP_CONV_COLS
set_directive_pipeline fp_conv/LOOP_CONV_COLS
# fp_conv/LOOP_OUTPUT
set_directive_pipeline fp_conv/LOOP_OUTPUT

# bin_dense/LOOP_DENSE_I
set_directive_pipeline bin_dense/LOOP_DENSE_I
set_directive_loop_tripcount -min 1 -max 1 bin_dense/LOOP_DENSE_O
set_directive_loop_tripcount -min 1 -max 64 bin_dense/LOOP_DENSE_I

# Each ping-poing buffer in the dmem has 2048 words
# For input layer we need 3*32*32/2 = 1536 words total, divided into blocks of 512
set_directive_array_partition top dmem           -dim 1 -type complete
set_directive_array_partition top dmem           -dim 2 -type complete
set_directive_array_partition top wt_mem         -dim 1 -type complete
set_directive_array_partition bin_conv line_buffer     -dim 0 -type complete
set_directive_array_partition bin_conv conv_params     -dim 0 -type complete
set_directive_array_partition bin_conv fixed_buffer    -dim 2 -type complete
set_directive_array_partition bin_conv fixed_temp      -dim 0 -type complete
set_directive_array_partition bin_conv word_buffer     -dim 0 -type complete
set_directive_array_partition bin_conv old_word_buffer -dim 0 -type complete
set_directive_array_partition bin_conv lb              -dim 0 -type complete
set_directive_array_partition bin_conv rb              -dim 0 -type complete
set_directive_array_partition bin_conv wt_word_buffer  -dim 0 -type complete
set_directive_array_partition bin_conv conv_out_buffer -dim 0 -type complete
set_directive_array_partition fp_conv win       -dim 0 -type complete
set_directive_array_partition fp_conv lbuf      -dim 0 -type complete
set_directive_array_partition fp_conv outwords  -dim 0 -type complete
set_directive_array_partition fp_conv wtbuf     -dim 0 -type complete
set_directive_array_partition bin_fc  sum_m     -dim 0 -type complete
#=========================================================================
