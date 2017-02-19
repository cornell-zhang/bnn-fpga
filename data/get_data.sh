#!/usr/bin/env bash

CURR_DIR=`pwd`
DIR="$( cd "$( dirname "${BASH_ROUCE[0]}" )" && pwd )"
cd $DIR

declare -a files=("cifar10_test_inputs.zip" "cifar10_test_labels.zip" 
                  "cpp_conv1_maps.zip" "cpp_conv2_maps.zip"
                  "cpp_conv3_maps.zip" "cpp_conv4_maps.zip"
                  "cpp_conv5_maps.zip" "cpp_conv6_maps.zip"
                  "cpp_dense0_maps.zip" "cpp_dense1_maps.zip"
                  "cpp_dense2_maps.zip")

for i in "${files[@]}" ; do
  if [ -f $i ] ; then
    echo "$i already exists"
  else
    wget "www.csl.cornell.edu/~rzhao/files/$i"
  fi
done

cd $CURR_DIR
