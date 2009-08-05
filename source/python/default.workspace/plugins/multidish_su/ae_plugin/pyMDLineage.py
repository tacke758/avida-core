"""
pyMDLineage

Module to handle lineage tracking in multi-dish runs.
"""

class MDLineage():
    """
    class MDLineage

    Class to handle lineage tracking for multi-dish
    """

    def __init__(self,sdn):
        self.n_subdishes = sdn

    def sd2lineage(self,sdc,sdndx):
        """
        sdc2lineage

        Converts a subdish cell index to a lineage number.

        Inputs:
        sdc = subdish cell
        sdndx = subdish index, base zero number to indicate subdish index
        """
        lineage = (self.n_subdishes * int(sdc)) + int(sdndx)
        return(lineage)

    def lineage2sd(self,lineage):
        """
        lineage2sd

        """
        sdc = lineage / self.n_subdishes
        sdndx = lineage % self.n_subdishes
        return((sdc,sdndx))

"""
Testing
"""


if __name__ == "__main__":

    sdn = 16
    print "Number of subdishes = %d" % (sdn)
    mdl = MDLineage(sdn)
    # mdl.__init__(sdn)

    sdc = 3
    sdndx = 15
    lineage = mdl.sd2lineage(sdc,sdndx)
    
    print "SDC = %s, SD index = %s, lineage = %s" % (sdc,sdndx,lineage)

    (csdc,csdndx) = mdl.lineage2sd(lineage)
    print "Given lineage %s, calculate SDC = %s and SD index = %s" % (lineage,csdc,csdndx)



    
