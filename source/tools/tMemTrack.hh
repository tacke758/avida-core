#ifndef TMEM_TRACK_HH
#define TMEM_TRACK_HH

template <class T> class tMemTrack {
private:
  static int obj_count;
public:
  // These "New" commands can only be used with no arguments.
  static T * New() {
    obj_count++;
    return new T;
  }
  static T * New(int count) {
    obj_count += count;
    return new T[count];
  }

  static void Free(T * ptr) {
    obj_count--;
    free ptr;
  }
  static void Free(T * ptr, int count) {
    obj_count -= count;
    free [] ptr;
  }

  // This New and Free just help keep track of things handled elsewhere...
  static void MarkNew() { obj_count++; }
  static void MarkFree() { obj_count--; }

 
  // And some methods to keep track of what's going on...
  static int GetCount() { return obj_count; }
};

#endif
