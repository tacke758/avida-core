# -*- coding: utf-8 -*-

### pyGridMaster
# Created 2009/02/03 by Wesley R. Elsberry
#
# Python module to handle client/server grid transformations in Avida-ED plugin.
# The server must allow clients to contribute dishes.


# class pySuperDish:
    ### Class to handle multiple contributed dishes
    
class Grids:
    def __init__(self,sx=5,sy=5,cx=60,cy=60):
        # Set default values
        self.serverGridXDefault = sx
        self.serverGridYDefault = sy
        self.clientGridXDefault = cx
        self.clientGridYDefault = cy
        self.serverGridExists = False
        
    # Grid transformations
    # Generic methods of mapping client dishes of arbitrary size together
    # into one server dish, and for mapping back to client dishes
    #

    def clientCell2ServerCell(self,inx,iny,igx,igy,snx,sny,sgc):
        """
        Client dish of size (snx,sny) with cell sgc maps to a server dish cell igc
        given server dish grid size of (inx,iny) and sub-dish of (igx,igy)

        Inputs:
        inx = # of subdishes in a row
        iny = # of subdishes in a column
        igx = index in row of multidish placement for subdish (base 1)
        igy = index in column of multidish placement for subdish (base 1)
        snx = # of cells in row of subdish
        sny = # of cells in column of subdish
        sgc = cell index in subdish

        Output:
        igc = cell index in multidish (base 0)
        """
        irw = inx * snx      # server grid row width
        iro = (igx-1) * snx  # server grid row offset
        sgco = sgc / snx     # client grid column offset, integer division
        sgro = sgc % snx     # client grid row offset
        ico = sgc / irw      # server grid column offset, integer division
        igc = (sgro + iro) + (sgco * irw) + (((igy - 1) * irw) * sny)
        # Now get ix, iy from grid cell
        ix = igc % irw
        iy = igc / irw
        return igc, ix, iy

    def serverCell2ClientCell(self,inx,iny,snx,sny,igc):
        """
        Server grid cell igc in dish size (inx,iny) of sub-grids of size (snx,sny)
        maps back to client grid (sgx,sgy) and client cell (sgc)

        Inputs:
        inx = # of subdishes in a row
        iny = # of subdishes in a column
        snx = # of cells in row of subdish
        sny = # of cells in column of subdish
        igc = cell index in multidish

        Output:
        list containing
        sgc = cell index in subdish (base 0)
        sgx = index in row of subdish (base 0)
        sgy = index in column of subdish (base 0)
        """
        # print "serverCell2ClientCell(%s, %s, %s, %s, %s)" % (inx, iny, snx, sny, igc)
        irw = inx * snx      # server grid row width
        igx = ((igc % irw) / snx) + 1  # Needs integer division 
        igy = (igc / (irw * sny)) + 1  # Needs integer division
        sgx = ((igc % irw) % snx) + 1  # 
        sgy = ((igc / irw) + 1) - ((igy - 1) * sny)
        sgc = ((sgy - 1) * snx) + (sgx - 1)
        # Which subdish did this come from?
        sgpos = ((igy - 1) * inx) + igx - 1
        ccelldata = [sgc, sgx, sgy, igx, igy, sgpos]
        return ccelldata


    def makeServerGrid(self, XSize, YSize):
        if (self.serverGridExists):
            # Error; redundant call to make the server grid, ignore
            return -1
        # Set up server grid
        self.serverGridExists = True
        

"""
Self-test
"""
if __name__ == "__main__":
    sd_nrow = 4
    sd_ncol = 4
    sd_rowsize = 30
    sd_colsize = 30
    mygrid = Grids(sd_nrow,sd_ncol,sd_rowsize,sd_colsize)

    mdcell = 1
    sdinfo = mygrid.serverCell2ClientCell(sd_nrow,sd_ncol,sd_rowsize,sd_colsize,mdcell)
    print "%s = mygrid.serverCell2ClientCell(%s,%s,%s,%s,%s)" % (sdinfo,sd_nrow,sd_ncol,sd_rowsize,sd_colsize,mdcell)

    mdcell = 2 + sd_rowsize * 1
    sdinfo = mygrid.serverCell2ClientCell(sd_nrow,sd_ncol,sd_rowsize,sd_colsize,mdcell)
    print "%s = mygrid.serverCell2ClientCell(%s,%s,%s,%s,%s)" % (sdinfo,sd_nrow,sd_ncol,sd_rowsize,sd_colsize,mdcell)

    mdcell = 3 + sd_rowsize * 2
    sdinfo = mygrid.serverCell2ClientCell(sd_nrow,sd_ncol,sd_rowsize,sd_colsize,mdcell)
    print "%s = mygrid.serverCell2ClientCell(%s,%s,%s,%s,%s)" % (sdinfo,sd_nrow,sd_ncol,sd_rowsize,sd_colsize,mdcell)
