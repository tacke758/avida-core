//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "inst_util.hh"

#include "../tools/string.hh"
#include "../tools/file.hh"

#include "../cpu/hardware_method.hh"
#include "inst_lib.hh"
#include "genome.hh"


using namespace std;


cGenome cInstUtil::LoadGenome(const cString & filename,
			      const cInstLib & inst_lib)
{
  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cerr << "Cannot open file: " << filename << endl;
    return cGenome(1);
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // Setup the code array...
  cGenome new_genome(input_file.GetNumLines());

  for (int line_num = 0; line_num < new_genome.GetSize(); line_num++) {
    cString cur_line = input_file.RemoveLine();
    new_genome[line_num] = inst_lib.GetInst(cur_line);

    if (new_genome[line_num] == cInstLib::GetInstError()) {
      // You're using the wrong instruction set!  YOU FOOL!
      cerr << "Cannot load organism '" << filename << "'" << endl
	   << "       Unknown line: " << cur_line
	   << " (best match is '" << inst_lib.FindBestMatch(cur_line) << "')"
	   << endl;
      exit(1);
    }
  }

  return new_genome;
}

cGenome cInstUtil::LoadInternalGenome(istream & fp, const cInstLib &inst_lib)
{
  assert(fp.good()); // Invalid stream to load genome from!

  int num_lines = -1;
  fp >> num_lines;

  if (num_lines <= 0) { return cGenome(1); }

  // Setup the code array...
  cGenome new_genome(num_lines);
  cString cur_line;

  for (int line_num = 0; line_num < new_genome.GetSize(); line_num++) {
    fp >> cur_line;
    new_genome[line_num] = inst_lib.GetInst(cur_line);

    if (new_genome[line_num] == cInstLib::GetInstError()) {
      // You're using the wrong instruction set!  YOU FOOL!
      cerr << "Cannot load creature from stream:" << endl
	   << "       Unknown line: " << cur_line << endl;
    }
  }
  return new_genome;
}

void cInstUtil::SaveGenome(ostream & fp, const cInstLib & lib,
			   const cGenome & gen)
{
  for (int i = 0; i < gen.GetSize(); i++) {
    fp << lib.GetName(gen[i]) << endl;
  }
}

void cInstUtil::SaveInternalGenome(ostream & fp, const cInstLib & lib,
				   const cGenome & gen)
{
  fp << gen.GetSize() << endl;
  SaveGenome(fp, lib, gen);
}


cGenome cInstUtil::RandomGenome(int length, const cInstLib & inst_lib)
{
  cGenome genome(length);
  for (int i = 0; i < length; i++) {
    genome[i] = inst_lib.GetRandomInst();
  }
  return genome;
}

