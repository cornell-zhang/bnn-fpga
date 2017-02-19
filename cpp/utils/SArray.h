#ifndef SARRAY_H
#define SARRAY_H

//------------------------------------------------------------------------
// Static array organized as N SxS images
//------------------------------------------------------------------------
template<typename T, unsigned SIZE>
struct SArray {
  typedef T ElemType;
  T data[SIZE];

  static constexpr unsigned size() { return SIZE; }

  // get base ptr
  T* ptr() { return data; }
  const T* ptr() const { return data; }

  // array subscript
  T& operator[](unsigned i) { return data[i]; }
  const T& operator[](unsigned i) const { return data[i]; }

  void set(T x) {
    for (unsigned i = 0; i < size(); ++i)
      data[i] = x;
  }

  void clear() {
    set(0);
  }

  // copy data from another array or SArray
  template<typename A>
  void copy_from(const A& other, unsigned siz=size()) {
    assert(siz <= size());
    for (unsigned i = 0; i < siz; ++i)
      data[i] = other[i];
  }

  // copy data and binarize
  template<typename A>
  void binarize_from(const A& other, unsigned siz=size()) {
    assert(siz <= size());
    for (unsigned i = 0; i < siz; ++i)
      data[i] = (other[i] >= 0) ? 0 : 1;
  }

  // print a subimage
  void print_sub(unsigned n, unsigned S,
                 unsigned maxs=8, char fmt='f') const {
    maxs = (maxs >= S) ? S : maxs;
    assert(n*S*S < size());
    for (unsigned r = 0; r < maxs; ++r) {
      for (unsigned c = 0; c < S; ++c) {
        if (fmt == 'f') 
          printf ("%6.3f ", (float)data[n*S*S + r*S + c]);
        else
          printf ("%5d ", (int)data[n*S*S + r*S + c]);
      }
      printf ("\n");
    }
  }
  
  // print an image
  void print(unsigned n=0, unsigned S=32, char fmt='f') const {
    print_sub(n, S, S, fmt);
  }
 
};

#endif
