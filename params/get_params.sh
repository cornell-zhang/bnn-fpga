#!/usr/bin/env bash

CURR_DIR=`pwd`
DIR="$( cd "$( dirname "${BASH_ROUCE[0]}" )" && pwd )"
cd $DIR

declare -a files=("cifar10_parameters_nb.zip" "cifar10_parameters_nb.npz")

for i in "${files[@]}" ; do
  if [ -f $i ] ; then
    echo "$i already exists"
  else
    wget "www.csl.cornell.edu/~rzhao/files/$i"
  fi
done

cd $CURR_DIR
