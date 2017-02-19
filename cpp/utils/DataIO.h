//------------------------------------------------------------------------
// Class to read the image data
//------------------------------------------------------------------------
#include <assert.h>

#include "Debug.h"
#include "ZipIO.h"
#include "Common.h"
#include "SArray.h"

// This class will load N cifar10 test images
struct Cifar10TestInputs {
  static const unsigned CHANNELS=3;
  static const unsigned ROWS=32;
  static const unsigned COLS=32;
  static constexpr const char* filename = "/data/cifar10_test_inputs.zip";

  float* data;
  unsigned m_size;

  Cifar10TestInputs(unsigned n);
  ~Cifar10TestInputs() { delete[] data; }
  unsigned size() { return m_size; }
};

struct Cifar10TestLabels {
  static constexpr const char* filename = "/data/cifar10_test_labels.zip";

  float* data;
  unsigned m_size;

  Cifar10TestLabels(unsigned n);
  ~Cifar10TestLabels() { delete[] data; }
  unsigned size() { return m_size; }
};
