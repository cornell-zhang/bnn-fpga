#include "DataIO.h"


Cifar10TestInputs::Cifar10TestInputs(unsigned n)
  : m_size(n*CHANNELS*ROWS*COLS)
{
  data = new float[m_size];

  std::string full_filename = get_root_dir() + filename;
  DB_PRINT(2, "Opening data archive %s\n", full_filename.c_str());
  unzFile ar = open_unzip(full_filename.c_str());
  unsigned nfiles = get_nfiles_in_unzip(ar);
  assert(nfiles == 1);

  // We read m_size*4 bytes from the archive
  unsigned fsize = get_current_file_size(ar);
  assert(m_size*4 <= fsize);

  DB_PRINT(2, "Reading %u bytes\n", m_size*4);
  read_current_file(ar, (void*)data, m_size*4);
  
  unzClose(ar);
}

Cifar10TestLabels::Cifar10TestLabels(unsigned n)
  : m_size(n)
{
  data = new float[m_size];

  std::string full_filename = get_root_dir() + filename;
  DB_PRINT(2, "Opening data archive %s\n", full_filename.c_str());
  unzFile ar = open_unzip(full_filename.c_str());
  unsigned nfiles = get_nfiles_in_unzip(ar);
  assert(nfiles == 1);

  // We read n*4 bytes from the archive
  unsigned fsize = get_current_file_size(ar);
  assert(m_size*4 <= fsize);

  DB_PRINT(2, "Reading %u bytes\n", m_size*4);
  read_current_file(ar, (void*)data, m_size*4);
  unzClose(ar);
}

