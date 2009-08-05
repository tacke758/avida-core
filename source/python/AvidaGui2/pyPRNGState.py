"""
pyPRNGState.py

Provides a global stack for interrupting the use of the global C++ pseudo-random number generator and 
using it for a set of computations.

Provides a registry for status and pause functions for processes that use the global C++ PRNG.

"""

#import ctools
#from AvidaCore import cAnalyzeGenotype, cGenome, cInstruction, cInstUtil, cRandom, cString, cTools
from AvidaCore import cRandom, cTools

import qt

class StackType():
    """
    StackType
    Abstract stack data type. 
    Mostly a direct use of Python list methods, but offers the "top" 
    operation in addition and a push() alias for append().
    """
    def __init__(self):
        # A list is the basic data structure
        # An empty list is the start
        self.stk = []
    
    def push(self,item):
        # Add an item to the top of the stack
        self.stk.append(item)

    def pop(self):
        # Remove an item from the top of the stack
        return(self.stk.pop())

    def top(self):
        return(self.stk[-1])

# Make the class one that can generate and receive signals by basing it on
# a Qt object. Not used so far, but doesn't hurt to have it.
class PRNGState(QObject):
    """
    PRNGState

    Theory of operation:

    A PRNGState object should be instantiated on the start of Avida-ED.

    All processes requiring the use of the global pseudo-random number
    generator (PRNG) should provide six items: 
    - name of the process
    - a method that provides the status of the process
    - a method that pauses the process
    - a method that will resume the process
    - whether the random seed to be used is based on time()
    - a random seed value (will be 1 for the Organism view)

    The global PRNG-using process should be added to the PRNGState with
    the add() method before it first begins. The add() method will set up
    the global PRNG state and preserve the state needed by any previous
    user. The process may then be started safely.

    On end of the current process, the PRNGState remove() method should 
    be called, which will restore the previous user process by setting
    the PRNG state last used and then running the resume() function of the
    prior process.
    
    """
    def setPRNG(self,prng_state):
        """
        setPRNG
        Method to restore a global PRNG state preparatory to resuming a 
        PRNG-using process.
        """
        # Restore random number generator state.
        cTools.globalRandom().Clone(prng_state)

    def resetPRNG(self,seed_based_on_time,seed):
        """
        resetPRNG
        Method to start the PRNG with a new seed.
        """
        # Reset random number generator state.
        if self.m_seed_based_on_time:
            cTools.globalRandom().ResetSeed(-1)
        else:
            cTools.globalRandom().ResetSeed(seed)

    def __init__(self):
        """
        __init__
        Create the various dictionaries and data structures to handle the global PRNG.
        """
        # Create empty status dict
        self.status = {}
        # Create empty pause dict
        self.pause = {}
        # Create empty resume dict
        self.resume = {}
        
        # Create an empty global PRNG users and state stacks
        self.users = StackType()
        self.states = StackType()

    def add(self,processname,statusfunc,pausefunc,resumefunc,seed_based_on_time,seed):
        """
        add
        Method to add a process to the users stack, pausing any previous global PRNG-using
        process.
        """
        # A process needs the global PRNG
        # Pause the currently running global PRNG user, if there is one
        olduser = self.users.top()
        if (None <> olduser):      # If there is no current user, nothing special need be done
            # Need to pause the old user of the global PRNG
            self.pause[olduser]()
            # Push the global PRNG state for the just-paused process
            # Save random number generator state.
            random_number_generator_state = cRandom(cTools.globalRandom())
            self.states.push(random_number_generator_state)
        # Now set the new process up for global PRNG use
        self.users.push(processname)
        self.status[processname] = statusfunc
        self.pause[processname] = pausefunc
        self.resume[processname] = resumefunc
        self.resetPRNG(seed_based_on_time,seed)

    def remove(self):
        """
        remove
        Method to remove the top value from the users stack and
        restore the prior PRNG state.
        """
        ruser = self.users.pop()
        prng_state = self.states.pop()
        if (None == prng_state):       # This is a problem; should never happen
            print "PRNG remove ERROR: None state encountered. **** VERY BAD THING ****"
        else:
            # There is a prior state, so set it up
            setPRNG(prng_state)
            # Now to resume the process of the prior user
            olduser = self.states.top()
            if (None <> olduser):
                self.resume[olduser]()




class AbstractProc(QObject):
    def __init__(self,name):
        self.name = name
        self.run_state = "Not started"

    def status(self):
        return(self.run_state)

    def pause(self):
        self.run_state = "Paused"

    def resume(self):
        self.run_state = "Running"

    def stop(self):
        self.run_state = "Stopped"

    def do_process(self):
        if ("Running" == self.run_State):
            self.process_step()

    def process_step(self):
        pass


class ProcA(AbstractProc):
    def __init__(self,name):
        AbstractProc.__init__(name)

    def status(self):
        return(self.run_state)

    def pause(self):
        self.run_state = "Paused"

    def resume(self):
        self.run_state = "Running"

    def stop(self):
        self.run_state = "Stopped"

    def do_process(self):
        if ("Running" == self.run_State):
            self.process_step()

    def process_step(self):
        pass
        


if (1): # Testing

    print "Testing PRNGState...\n"
    myprngctrl = PRNGState()


    proca = ProcA("ProcA")


    print "ProcA name = %s, state = %s\n" % (proca.name,proca.status())

    print "Testing PRNGState... Done.\n";
