#include "data_entry.hh"

ostream & operator << (ostream & out, cDataEntry & entry)
{
  entry.Print(out);
  return out;
}
