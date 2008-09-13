/*
 *  cOrganismThread.h
 *  Avida
 *
 *  Created by Benjamin Beckmann on 9/12/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef cOrganismThread_h
#define cOrganismThread_h

class cOrganismThread {
private:
	
protected:
	int m_id;

public:
	int GetID() const { return m_id; }
	void SetID(int in_id) { m_id = in_id; }
	
	void saveState() {;}
	void restoreState() {;}
};

#endif
