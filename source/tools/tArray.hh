//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/*
   tArray.hh
   Array Templates

   Time-stamp: <1999-12-08 22:05:30 travc>

   Constructor:
     tArray( int size )
     tArray( const tArray & rhs )

   Interface:
     tArray & operator= ( const tArray & rhs )

     bool Good () const
       array is defined and ready to take data

     unsigned int GetSize () const
       returns the size of the array

     const T& ElementAt ( const int index ) const
           T& ElementAt ( const int index )
     const T& operator[] ( const int index ) const
           T& operator[] ( const int index )
       return the element at index in the array



*/

#ifndef TARRAY_HH
#define TARRAY_HH

#include "../defs.hh"    // needed for bool and explicit if needed
#include "assert.hh"

/**
 * This class provides an array template.
 **/ 

template <class T> class tArray {

protected:
  // Internal Variables
  T * data;  // Data Elements
  int size;  // Number of Elements

public:
  void ResizeClear(const int in_size){
    size = in_size;
    // Size must be > 0
    assert_error( size > 0, "Invalid size specified for array intialization" );
    if( data != NULL )  delete [] data;  // remove old data if exists
    data = new T[size];  // Allocate block for data
    // Check for allocation error
    assert_error( data != NULL, "Memory Allocation Error: Out of Memory?" );
  }

public:
  // Default Constructor
  explicit tArray() : data(NULL), size(0) { ; }

  // Constructor
  explicit tArray(const int _size) : data(NULL) {
    ResizeClear(_size);
  }

  // Assingment Operator
  tArray & operator= (const tArray & rhs){
    if( size != rhs.GetSize() || data == NULL )  ResizeClear(rhs.GetSize());
    for( int i=0; i<size; ++i )  data[i] = rhs[i];
    return *this;
  }

  // Copy constructor
  explicit tArray(const tArray & rhs) : data(NULL) {
    this->operator=(rhs);
  }

  // Destructor
  virtual ~tArray(){
    if( data!=NULL ) delete [] data;
  }


  // Interface Methods ///////////////////////////////////////////////////////

  bool Good() const { return (data!=NULL); }

  int GetSize() const { return size; }

  void Resize(int new_size) {
    // New size must be > 0
    assert_error( new_size > 0, "Invalid size specified for array resize" );
    if( data != NULL )  {
      T * new_data = new T[new_size];
      for (int i = 0; i < size && i < new_size; i++) {
	new_data[i] = data[i];
      }
      delete [] data;  // remove old data if exists
      data = new_data;
    } else {
      data = new T[new_size];  // Allocate block for data
    }
    // Check for allocation error
    assert_error( data != NULL, "Memory Allocation Error: Out of Memory?" );
    size = new_size;
  }


  void Resize(int new_size, T & empty_value) {
    int old_size = size;
    Resize(new_size);
    if( new_size > old_size ){
      for( int i=old_size; i<new_size; ++i ){
	data[i] = empty_value;
      }
    }
  }


  T & ElementAt(const int index){
    // check range
    assert_error( index >= 0, "Lower Bounds Error" );
    assert_error( index < size, "Upper Bounds Error" );
    return data[index];      // in range, so return element
  }

  const T & ElementAt(const int index) const {
    // check range
    assert_error( index >= 0, "Lower Bounds Error" );
    assert_error( index < size, "Upper Bounds Error" );
    return data[index];      // in range, so return element
  }

        T & operator[](const int index)       { return ElementAt(index); }
  const T & operator[](const int index) const { return ElementAt(index); }


  void SetAll(const T & value){
    for( int i=0; i < size; ++i ){
      data[i] = value;
    }
  }

};



#endif // TARRAY_HH
