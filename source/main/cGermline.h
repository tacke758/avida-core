#ifndef _C_GERMLINE_H_
#define _C_GERMLINE_H_

#include <vector>

#include "cGenome.h"
#include "cMerit.h"

class cGermline {
public:
	cGenome& GetLatest() { return _germline.back(); }
	void Add(const cGenome& genome) { _germline.push_back(genome); }
	unsigned int Size() const { return _germline.size(); }
  
  const cMerit& GetMerit() { return _merit; }
  void UpdateMerit(double v) { _merit = v; }

protected:
	std::vector<cGenome> _germline;
  cMerit _merit;
};

#endif
