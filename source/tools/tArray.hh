/* 
   tArray.hh 
   Array Templates

   Time-stamp: <98/06/11 23:51:44 travc>

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
  explicit tArray() : data(NULL) { ; }
  
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
    delete [] data;
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


};




////////////////////////////////////
//  tMatrix template...


template <class T> class tMatrix {
protected:
  // Internal Variables
  tArray<T> * data;  // Data Elements
  int num_rows;
public:
  int GetNumRows() const { return num_rows; }
  int GetNumCols() const { return data[0].GetSize(); }

  void ResizeClear(const int _rows, const int _cols){
    if (_rows != GetNumRows()) {
      num_rows = _rows;
      assert_error(_rows > 0, "Invalid size specified for matrix resize" );
      if( data != NULL )  delete [] data;  // remove old data if exists
      data = new tArray<T>[_rows];  // Allocate block for data
      assert_error( data != NULL, "Memory Allocation Error: Out of Memory?" );
    }
    for (int i = 0; i < GetNumRows(); i++) data[i].ResizeClear(_cols);
  }

  void Resize(int _rows, int _cols) {
    // Rows and cols must be > 0
    assert_error( _rows > 0 && _cols > 0, 
		  "Invalid size specified for matrix resize" );

    if( data != NULL )  {     
      tArray<T> * new_data = new tArray<T>[_rows];
      for (int i = 0; i < GetNumRows() && i < _rows; i++) {
	new_data[i] = data[i];
      }
      delete [] data;  // remove old data if exists
      data = new_data;
    } else {
      data = new tArray<T>[_rows];  // Allocate block for data
    }
    assert_error( data != NULL, "Memory Allocation Error: Out of Memory?" );
    num_rows = _rows;

    for (int i = 0; i < _rows; i++) data[i].Resize(_cols);
  }

  T & ElementAt(int _row, int _col) { return data[_row][_col]; }
  const T & ElementAt(int _row, int _col) const { return data[_row][_col]; }
      
        T & operator()(int _r, int _c)       { return ElementAt(_r, _c); }
  const T & operator()(int _r, int _c) const { return ElementAt(_r, _c); }

        tArray<T> & operator[](int row)       { return data[row]; }
  const tArray<T> & operator[](int row) const { return data[row]; }

public:
  // Default Constructor 
  explicit tMatrix() : data(NULL), num_rows(0) { ResizeClear(1,1); }
  
  // Constructor 
  explicit tMatrix(const int _rows, const int _cols) :
    data(NULL), num_rows(0) {
      ResizeClear(_rows, _cols);
  }
  
  // Assingment Operator
  tMatrix & operator= (const tMatrix & rhs){
    if( GetNumRows() != rhs.GetNumRows() || GetNumCols() != rhs.GetNumCols()) {
      ResizeClear(rhs.GetNumRows(), rhs.GetNumCols());
    }
    for (int row = 0; row < GetNumRows(); row++) {
      for (int col = 0; col < GetNumCols(); col++) {
	data[row][col] = rhs.data[row][col];
      }
    }
    return *this;
  }

  // Copy constructor
  explicit tMatrix(const tMatrix & rhs) : data(NULL), num_rows(0) {
    this->operator=(rhs);
  }

  // Destructor
  virtual ~tMatrix(){ delete [] data; }


  
};

#endif // TARRAY_HH
