#include <assert.h>

#include "ParamIO.h"
#include "ZipIO.h"
#include "Common.h"

Params::Params(std::string zipfile)
  : m_filename(zipfile),
    m_arrays(0)
{
  // Open file
  DB_PRINT(2, "Opening params archive %s\n", m_filename.c_str());
  unzFile ar = open_unzip(m_filename);

  // Get number of files in the archive
  m_arrays = get_nfiles_in_unzip(ar);
  DB_PRINT(2, "Number of param arrays: %u\n", m_arrays);
  assert(m_arrays <= MAX_LAYERS);

  // Read each array
  for (unsigned i = 0; i < m_arrays; ++i) {
    unsigned fsize = get_current_file_size(ar);
    m_array_size[i] = fsize;  // size in bytes
    
    m_data[i] = new float[fsize/4];
    read_current_file(ar, (void*)m_data[i], fsize);
    
    unzGoToNextFile(ar);
  }

  unzClose(ar);
}

Params::~Params() {
  for (unsigned i = 0; i < m_arrays; ++i)
    delete[] m_data[i];
}

