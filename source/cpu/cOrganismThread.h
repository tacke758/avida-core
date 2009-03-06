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

class cString;

class cOrganismThread {
private:
	
protected:
	int m_id;
	bool interrupted;        //<! is thread interrupted
	int interruptMessageType;

public:
	cOrganismThread() : m_id(-1), interrupted(false), interruptMessageType(-1) {;}
	virtual ~cOrganismThread() {;}
	
	int GetID() const { return m_id; }
	void SetID(int in_id) { m_id = in_id; }
	bool isInterrupted() const { return interrupted; }
	int getInterruptMessageType() const { return interruptMessageType; }
	
	virtual void saveState() = 0;  //!< saves thread's current state
	virtual void restoreState() = 0;  //!< restores thread's saved state
	virtual bool initializeInterruptState(const cString&) = 0;  //!< sets thread state to default interrupted state if interrupt header exists
	virtual void interruptContextSwitch(int) = 0;  //!< performs context switch between normal thread execution and interrupt handler
};

#endif
