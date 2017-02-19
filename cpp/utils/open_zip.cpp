//------------------------------------------------------------------------
// Simple test program for minizip, it checks the contents (files) in
// a zip archive and reports the size of each file
// Requires zlib and minizip.
//------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../minizip/unzip.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <file to unzip>\n", argv[0]);
    return 0;
  }

  int err;

  // Open the zipfile
  // A "zipfile" means the zip archive, which can be a directory containing
  // many files.
  unzFile zipfile = unzOpen(argv[1]);
  if (zipfile == NULL) {
    printf("Could not open %s\n", argv[1]);
    return -1;
  }
  
  // Get the number of files (entries) inside
  unz_global_info info;
  err = unzGetGlobalInfo(zipfile, &info);
  assert (!err);
  unsigned entries = info.number_entry;
  printf ("Entries: %u\n", entries);

  float** data = new float*[entries];

  for (unsigned i = 0; i < entries; ++i) {
    // Get some info on the file
    unz_file_info finfo;
    char filename[50];
    err = unzGetCurrentFileInfo(zipfile, &finfo, filename, 50, NULL, 0, NULL, 0);
    assert (!err);
    printf ("  File %s: size=%luKB, compsize=%luKB\n", filename,
        finfo.uncompressed_size>>10, finfo.compressed_size>>10);

    // The data is an array of floats packed into 4-byte chunks
    unsigned array_size = finfo.uncompressed_size/4;
    assert(finfo.uncompressed_size % 4 == 0);
    assert(array_size > 0);
    data[i] = new float[array_size];

    // Read the data from the current file 4 bytes at a time,
    // convert the bytes to a float and store it
    unzOpenCurrentFile(zipfile);
    float f;

    for (unsigned j = 0; j < array_size; ++j) {
      // unzReadCurrentFile returns the number of bytes read
      // or 0 if end of file is reached
      err = unzReadCurrentFile(zipfile, (void*)&f, 4);
      assert((err == 4) || (j == array_size-1));
      data[i][j] = f;

      if (j < 5)
        printf ("  %5.2f", data[i][j]);
    }
    printf ("\n");

    unzCloseCurrentFile(zipfile);
    unzGoToNextFile(zipfile);
  }

  unzClose(zipfile);

  return 0;
}
