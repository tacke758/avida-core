"""
pyMkEnv.py

Module to assist in writing out Avida environment files for Multi-Dish runs.
"""

import pyGridMaster

class MkEnv():
    """
    MkEnv

    Class with methods for environment-writing
    """
    

    def __init__(self, mdx, mdy, sdx, sdy):
        # Number of sub-dishes in a row
        self.mdx = mdx
        # Number of sub-dishes in a column
        self.mdy = mdy
        # Number of cells in a sub-dish row
        self.sdx = sdx
        # Number of cells in a sub-dish column
        self.sdy = sdy

        self.resources = ('not', 'nand', 'and', 'orn', 'oro', 'andn', 'nor', 'xor', 'equ', )
        self.reactions = ('NOT', 'NAND', 'AND', 'ORN', 'ORO', 'ANDN', 'NOR', 'XOR', 'EQU', )

        self.grid = pyGridMaster.Grids(self.mdx,self.mdy,self.sdx,self.sdy)

        # Dictionary of tasks, resources, reactions, and rewards
        self.rrr = {'not':('not','NOT',1, ),
                    'nand':('nand','NAND',1, ),
                    'and':('and','AND',2, ),
                    'orn':('orn','ORN',2, ),
                    'or':('or','OR',3, ),
                    'andn':('andn','ANDN',3, ),
                    'nor':('nor','NOR',4, ),
                    'xor':('xor','XOR',4, ),
                    'equ':('equ','EQU',5, )
                    }

    def mkTemplate(self,envTempName):
        """
        mkTemplate

        Method to output an enviroment file suitable for processing to
        a specific set of sub-dish settings. This sets up
        spatially-defined resources that span the entire multi-dish,
        but specifies each resource-subgrid pair separately.
        
        Idea is that it is likely easier to add or remove the initial
        "#" character to turn off or on the use of a resource in a 
        subgrid.
        """

        f_env = open(envTempName, 'w')
 
        f_env.write("###############\n")
        f_env.write("#\n")
        f_env.write("# Multi-Dish environment template file\n")
        f_env.write("#\n")
        f_env.write("###############\n\n")

        # List of resources
        # All logic-9 resources get an entry.
        endidx = self.mdx * self.sdx - 1
        for ii in self.rrr:
            tstr = "RESOURCE %s" + \
                ":initial=0.1" + \
                ":geometry=torus" + \
                ":inflow=1.0" + \
                ":outflow=0.0" + \
                ":inflowx1=0" + \
                ":inflowx2=%s" + \
                ":inflowy1=0" + \
                ":inflowy2=%s" + \
                ":outflowx1=0" + \
                ":outflowx2=%s" + \
                ":outflowy1=0" + \
                ":outflowy2=%s" + \
                ":xgravity=0" + \
                ":ygravity=0" + \
                ":xdiffuse=0" + \
                ":ydiffuse=0\n"
            f_env.write(tstr % (self.rrr[ii][0],endidx,endidx,endidx,endidx))

        f_env.write("\n\n")
        
        for ii in self.rrr:
            tstr = "REACTION  %s  %s   process" + \
                ":value=1.0" + \
                ":max=1.0" + \
                ":min=0.001" + \
                ":frac=0.5:" + \
                "product=%s" + \
                ":conversion=1.0:" + \
                "type=pow" + \
                " requisite" + \
                ":max_count=1\n" 
            f_env.write(tstr % (self.rrr[ii][1],self.rrr[ii][0],self.rrr[ii][0]))

        f_env.write("\n\n")

        # Cell section. Write out complete set of resources and ranges.
        for mx in range(1,self.mdx+1):
            for my in range(1,self.mdy+1):

                f_env.write("\n# Sub-dish [%s,%s]\n" % (mx,my))
                for ii in self.rrr:
                    f_env.write("CELL %s:" % (ii))
        
                    sdrange = []
                    for jj in range(self.sdy):
                        mdc = self.grid.clientCell2ServerCell(self.mdx,self.mdy,mx,my,self.sdx,self.sdy,jj * self.sdx)
                        subrange = "%s..%s" % (mdc, mdc + self.sdx - 1)
                        sdrange.append(subrange)
                    thisrange = ",".join(sdrange)
                    f_env.write(thisrange)
                    
                    f_env.write(":initial=1.0:inflow=0.0:outflow=0.0\n")

                        
        f_env.close()


    def mkEnvFile(self,envName,sgv):
        """
        mkTemplate

        Method to output an enviroment file specifying a set of
        sub-dish settings. This sets up spatially-defined resources
        that span the entire multi-dish, but specifies each
        resource-subgrid pair separately.
        
        Inputs:
        envName : filename of environment configuration file
        sgv : dictionary with the task information
        """

        f_env = open(envTempName, 'w')
 
        f_env.write("###############\n")
        f_env.write("#\n")
        f_env.write("# Multi-Dish environment template file\n")
        f_env.write("#\n")
        f_env.write("###############\n\n")

        # List of resources
        # All logic-9 resources get an entry.
        endidx = self.mdx * self.sdx - 1
        for ii in self.rrr:
            tstr = "RESOURCE %s" + \
                ":initial=0.1" + \
                ":geometry=torus" + \
                ":inflow=1.0" + \
                ":outflow=0.0" + \
                ":inflowx1=0" + \
                ":inflowx2=%s" + \
                ":inflowy1=0" + \
                ":inflowy2=%s" + \
                ":outflowx1=0" + \
                ":outflowx2=%s" + \
                ":outflowy1=0" + \
                ":outflowy2=%s" + \
                ":xgravity=0" + \
                ":ygravity=0" + \
                ":xdiffuse=0" + \
                ":ydiffuse=0\n"
            f_env.write(tstr % (self.rrr[ii][0],endidx,endidx,endidx,endidx))

        f_env.write("\n\n")
        
        for ii in self.rrr:
            tstr = "REACTION  %s  %s   process" + \
                ":value=1.0" + \
                ":max=1.0" + \
                ":min=0.001" + \
                ":frac=0.5:" + \
                "product=%s" + \
                ":conversion=1.0:" + \
                "type=pow" + \
                " requisite" + \
                ":max_count=1\n" 
            f_env.write(tstr % (self.rrr[ii][1],self.rrr[ii][0],self.rrr[ii][0]))

        f_env.write("\n\n")

        # Cell section. Write out complete set of resources and ranges.
        for mx in range(1,self.mdx+1):
            for my in range(1,self.mdy+1):
                sdkey = ','.join(mx,my)
                print sdkey
        
                f_env.write("\n# Sub-dish [%s,%s]\n" % (mx,my))
                for ii in self.sgv:
                    f_env.write("CELL %s:" % (ii))
        
                    sdrange = []
                    for jj in range(self.sdy):
                        mdc = self.grid.clientCell2ServerCell(self.mdx,self.mdy,mx,my,self.sdx,self.sdy,jj * self.sdx)
                        subrange = "%s..%s" % (mdc, mdc + self.sdx - 1)
                        sdrange.append(subrange)
                    thisrange = ",".join(sdrange)
                    f_env.write(thisrange)
                    
                    f_env.write(":initial=1.0:inflow=0.0:outflow=0.0\n")
                        
        f_env.close()
