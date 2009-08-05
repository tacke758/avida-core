"""

pyAvidaCtrl.py

Routines to handle running avidacore plus updating a database backend.

Theory of operation:

Initialize, run, pause, or reset a single instance of Avida

Within a run: 
- collect data on a per-update basis.

"""

class AvidaCtrl:
    def __init__(self):
        """
        Basic initialization for Avida instance
        """
        pass

    def beginRun(self):
        """
        Start a run based on current settings
        """
        pass

    def pauseRun(self):
        """
        Pause an ongoing run
        """
        pass

    def killRun(self):
        """
        Take down a run.
        """

        pass

    def atUpdate(self):
        """
        Method called at the end of an update. 
        This is a place to connect things that should run at 
        each update.
        
        """
        pass

    def perUpdateDataDump(self):
        """
        Method to dump the data from a run at each update
        """
        # Check for being in a run, exit if not
        # Go through entire multidish to extract informtion
        # Cell traversal
        # Organism:
        # - Is it new? Add a new record?
        # - Fitness, Met. Rate, Gest. Time, 
        # - Phenotype
        # Population properties
        # Diversity measures
        pass


