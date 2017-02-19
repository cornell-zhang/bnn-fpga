//------------------------------------------------------------------------
// Class to read the parameters from a .zip archive.
// Requires zlib and minizip.
//------------------------------------------------------------------------
#ifndef PARAM_IO_H
#define PARAM_IO_H

#include <cstdint>
#include <string>
#include "../minizip/unzip.h"
#include "Debug.h"

/* Parameters are organized into arrays. A layer may have multiple arrays of
 * params. For example Weight and Bias are two arrays for a Conv layer
 */
struct Params {
  static const unsigned MAX_LAYERS = 64;

  std::string m_filename;
  unsigned m_arrays;
  unsigned m_array_size[MAX_LAYERS];
  float* m_data[MAX_LAYERS];

  public:
    // Read a zip archive containing NN params. We make the assumption
    // that each file in the archive is one array. The data is stored
    // as just an array of bytes
    Params(std::string zipfile);
    // Safely deletes params
    ~Params();

    // Get the number of layers
    unsigned num_arrays() const { return m_arrays; }
    // Get the size of the params array in bytes
    unsigned array_size(unsigned i) const {
      return m_array_size[i];
    }
    // Get a pointer to the params for layer <i>
    float* array_data(unsigned i) const {
      return m_data[i];
    }

    float* float_data(unsigned i) const {
      return array_data(i);
    }
};

#endif
