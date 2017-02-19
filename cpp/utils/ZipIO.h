#ifndef ZIP_IO_H
#define ZIP_IO_H

#include <cstdint>
#include <cstdlib>
#include <string>
#include "../minizip/zip.h"
#include "../minizip/unzip.h"

//------------------------------------------------------------------------
// Functions for reading a zip archive
//------------------------------------------------------------------------
unzFile open_unzip(const std::string filename);
unsigned get_nfiles_in_unzip(unzFile ar);
unsigned get_current_file_size(unzFile ar);
void read_current_file(unzFile ar, void* buffer, unsigned bytes);

//------------------------------------------------------------------------
// Writes a buffer to a new file with name [fname] inside the 
// zip archive [zf].
//------------------------------------------------------------------------
void write_buffer_to_zip(
    zipFile zf,         // zip archive handle
    std::string fname,  // name of file instead archive to write
    void* buf,          // buffer of data
    unsigned len        // how many bytes to write
    );

//------------------------------------------------------------------------
// SArray to and from zipfile
//------------------------------------------------------------------------
// Read zip archive to SArray, used when you know the archive contains
// one array and it has an exact size
template<typename Array>
void unzip_to_sarray(std::string filename, Array &buf) {
  unzFile ar = open_unzip(filename);
  unsigned fsize = get_current_file_size(ar);
  assert(fsize == sizeof(buf.data));

  read_current_file(ar, (void*)buf.ptr(), fsize);
  unzClose(ar);
}

// write an array to a zip archive containing one file
template<typename Array>
void sarray_to_zip(std::string filename, const Array &buf, unsigned n_elems=0) {
  zipFile ar = zipOpen(filename.c_str(), 0);
  n_elems = (n_elems == 0) ? Array::size() : n_elems;

  // copy the SArray data to an array of float
  float* data = new float[n_elems];
  for (unsigned i = 0; i < n_elems; ++i) {
    data[i] = buf[i];
  }

  // store the array of float
  write_buffer_to_zip(ar, "arr_0", (void*)data, n_elems*sizeof(float));
  delete[] data;
  int err = zipClose(ar, NULL);
  assert(err == ZIP_OK);
}

//------------------------------------------------------------------------
// C Array to and from zipfile
//------------------------------------------------------------------------
template<typename T>
void unzip_to_array(std::string filename, T buf[]) {
  unzFile ar = open_unzip(filename);
  unsigned fsize = get_current_file_size(ar);
  assert(fsize != 0 && fsize % 4 == 0);

  read_current_file(ar, (void*)buf, fsize);
  unzClose(ar);
}

template<typename T>
void bitarray_to_zip(std::string filename, T buf[], unsigned n_elems) {
  zipFile ar = zipOpen(filename.c_str(), 0);
  const unsigned elem_size = buf[0].length();

  // copy the array data to an array of float
  float* data = new float[n_elems];
  for (unsigned i = 0; i < n_elems; ++i) {
    data[i] = (buf[i/elem_size][i%elem_size] == 0) ? 1.0 : -1.0;
  }
  // store the array of float
  write_buffer_to_zip(ar, "arr_0", (void*)data, n_elems*sizeof(float));
  delete[] data;
  int err = zipClose(ar, NULL);
  assert(err == ZIP_OK);
}

#endif
