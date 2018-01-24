#-------------------------------------------------------------------------
# Converts a .npz numpy archive into a zip archive which can be read
# by many more utilities as well as our C++ code.
#
# Primarily used to convert the BNN parameters to C++ readable format.
#
# This file only deals with Floating Point parameters, for binarization
# use npz2zip_binary.py
#-------------------------------------------------------------------------
from in_memory_zip import in_memory_zip
import numpy as np
import struct
import sys

if __name__ == "__main__":
  if len(sys.argv) < 3:
    print "Usage: npz2zip.py <input.npz> <output.zip>"
    exit(-1)

  # initialize a zip archive
  imz = in_memory_zip()
  
  # load the npz file
  f = np.load(sys.argv[1])
  
  for i in range(len(f.keys())):
    k = 'arr_' + str(i)
    val = f[k]
    val = val.flatten()

    print "Writing", k, "\tsize =", len(val), "floats"

    # write the data to a file in plaintext
    #fp = open('frompy', 'w')
    #for v in val:
    #    fp.write(('%10.8f' % v) +'\n')
    #fp.close()

    # generate a string by packing each float as 4 bytes
    buf = struct.pack('%sf' % len(val), *val)

    imz.append(k, buf)

  # write zip archive to disk
  imz.writetofile(sys.argv[2])
