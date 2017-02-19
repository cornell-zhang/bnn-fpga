#include "Common.h"

//------------------------------------------------------------------------
std::string get_root_dir() {
  char* root = getenv("CRAFT_BNN_ROOT");
  if (!root) {
    fprintf(stderr, "Cannot find CRAFT_BNN_ROOT directory\n");
    exit(-1);
  }
  return std::string(root);
}

