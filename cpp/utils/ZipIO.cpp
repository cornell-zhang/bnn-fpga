#include <assert.h>

#include "ZipIO.h"
#include "Debug.h"

//------------------------------------------------------------------------
unzFile open_unzip(const std::string filename) {
  unzFile ar = unzOpen(filename.c_str());
  if (ar == NULL) {
    fprintf(stderr, "Error opening %s\n", filename.c_str());
    exit(-1);
  }
  return ar;
}

//------------------------------------------------------------------------
unsigned get_nfiles_in_unzip(unzFile ar) {
  unz_global_info info;
  int err = unzGetGlobalInfo(ar, &info);
  assert(!err);
  return info.number_entry;
}

//------------------------------------------------------------------------
unsigned get_current_file_size(unzFile ar) {
  unz_file_info finfo;
  char fname[50];
  
  int err = unzGetCurrentFileInfo(ar, &finfo, fname, 50, NULL, 0, NULL, 0);
  assert(!err);
  unsigned fsize = finfo.uncompressed_size;
  
  DB_PRINT(3, "Reading %10s, %u bytes\n", fname, fsize);
  
  assert(fsize > 0);
  return fsize;
}

//------------------------------------------------------------------------
void read_current_file(unzFile ar, void* buffer, unsigned bytes) {
  int err = unzOpenCurrentFile(ar);
  assert(!err);

  unsigned b = unzReadCurrentFile(ar, buffer, bytes);
  assert(b == bytes);

  unzCloseCurrentFile(ar);
}

//------------------------------------------------------------------------
void write_buffer_to_zip(zipFile zf, std::string fname, void* buf, unsigned len) {
  int err;
  DB_PRINT(3, "Writing %10s, %u bytes\n", fname.c_str(), len);

  // Open new file
  zip_fileinfo zi = {0};
  err = zipOpenNewFileInZip(zf, fname.c_str(), &zi,
                            NULL, 0, NULL, 0, NULL,
                            0,                      /* method */
                            Z_DEFAULT_COMPRESSION); /* level */
  assert(err == ZIP_OK);

  // Write data
  err = zipWriteInFileInZip(zf, buf, len);
  assert(err == ZIP_OK);

  // Close file
  err = zipCloseFileInZip(zf);
  assert(err == ZIP_OK);
}

