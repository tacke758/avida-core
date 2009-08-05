"""
pyMkEnv.py

Module to assist in writing out Avida environment files for Multi-Dish runs.
"""

import pyGridMaster

class MkEnv:
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

        # Need to order the tasks
        self.tasks = ('not', 'nand', 'and', 'orn', 'or', 'andn', 'nor', 'xor', 'equ', )

    def mkTemplate(self,envTempName):
        """
        mkTemplate

        Method to output an enviroment file suitable for processing to
        a specific set of sub-dish settings. This sets up
        spatially-defined resources that span the entire multi-dish,
        but specifies each resource-subgrid pair separately.
        
        Implementation: Each sub-dish gets its own resource/reaction/task definition.
        """

        f_env = open(envTempName, 'w')
 
        f_env.write("###############\n")
        f_env.write("#\n")
        f_env.write("# Multi-Dish environment template file\n")
        f_env.write("#\n")
        f_env.write("###############\n\n")

        # List of resources

        # All logic-9 resources get an entry per sub-dish, with coordinates
        # defined to fit the boundary of the sub-dish.

        print "Setting resources"
        for mx in range(1,self.mdx+1):
            for my in range(1,self.mdy+1):

                #if (((3 == mx) and (3 == my)) or ((4 == mx) and (4==my))):
                #if ((3 == mx) and (3 != my)):
                if (1):
                #if (my < (self.mdy - 1)):    
                    # Find upper left and lower right grid cells for the
                    # subdish in the multi-dish
                    
                    mdcul,mdulx,mduly = self.grid.clientCell2ServerCell(self.mdx,self.mdy,mx,my,self.sdx,self.sdy,0)
                    mdclr,mdlrx,mdlry = self.grid.clientCell2ServerCell(self.mdx,self.mdy,mx,my,self.sdx,self.sdy,self.sdx*self.sdy - 1)

                    print "UL %s ULX %s ULY %s LR %s LRX %s LRY %s" % (mdcul, mdulx, mduly, mdclr, mdlrx, mdlry) 
                    #mdlry /= 2
                    #print "UL %s ULX %s ULY %s LR %s LRX %s LRY %s" % (mdcul, mdulx, mduly, mdclr, mdlrx, mdlry) 

                    inflow = self.sdx * self.sdy * 1.0
                    outflow = 0.1

                    for ii in self.tasks:
                        tstr = "RESOURCE %s%s%s%s" + \
                            ":initial=0.00001" + \
                            ":geometry=torus" + \
                            ":inflow=%s" + \
                            ":outflow=%s" + \
                            ":inflowx1=%s" + \
                            ":inflowx2=%s" + \
                            ":inflowy1=%s" + \
                            ":inflowy2=%s" + \
                            ":outflowx1=%s" + \
                            ":outflowx2=%s" + \
                            ":outflowy1=%s" + \
                            ":outflowy2=%s" + \
                            ":xgravity=0" + \
                            ":ygravity=0" + \
                            ":xdiffuse=0" + \
                            ":ydiffuse=0\n"
                        f_env.write(tstr % ("res_",self.rrr[ii][0],mx,my,
                                            inflow,outflow,
                                            mdulx,mdlrx,
                                            mduly,mdlry,
                                            mdulx,mdlrx,
                                            mduly,mdlry))

                f_env.write("\n")

        f_env.write("\n\n")
        
        for mx in range(1,self.mdx+1):
            for my in range(1,self.mdy+1):

                # if ((3 == mx) and (3 == my)):
                # if (((3 == mx) and (3 == my)) or ((4 == mx) and (4==my))):
                #if ((3 == mx) and (3 != my)):
                #if (1):
                if (my < (self.mdy - 1)):    
                    for ii in self.tasks:
                        tstr = "REACTION  %s%s%s%s  %s   process" + \
                            ":resource=%s%s%s%s" + \
                            ":value=1.0" + \
                            ":max=1.0" + \
                            ":min=0.001" + \
                            ":frac=0.5:" + \
                            "product=%s%s%s%s" + \
                            ":conversion=1.0:" + \
                            "type=pow" + \
                            " requisite" + \
                            ":max_count=1\n" 
                        f_env.write(tstr % ("REA_",self.rrr[ii][1],mx,my,
                                            self.rrr[ii][0],
                                            "res_",self.rrr[ii][0],mx,my,
                                            "res_",self.rrr[ii][0],mx,my))

                f_env.write("\n")

        f_env.write("\n\n")
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

    def mkMDTemplate(self,envTempName,mdset):
        """
        mkMDTemplate

        Method to output an enviroment file suitable for processing to
        a specific set of sub-dish settings. This sets up
        spatially-defined resources that span the entire multi-dish,
        but specifies each resource-subgrid pair separately.
        
        Implementation: Each sub-dish gets its own resource/reaction/task definition.
        """
        print "pyMDMkEnv.mkMDTemplate()"

        # Base-0 or Base-1 for position encoding in keys?
        base1 = False       # Use Base-0, not Base-1
        # base1 = True      # Use Base-1, not Base-0

        print mdset

        f_env = open(envTempName, 'w')
 
        f_env.write("###############\n")
        f_env.write("#\n")
        f_env.write("# Multi-Dish environment file\n")
        f_env.write("#\n")
        f_env.write("###############\n\n")

        # List of resources

        # All logic-9 resources get an entry per sub-dish, with coordinates
        # defined to fit the boundary of the sub-dish.

        print "Setting resources"
        for mx in range(1,self.mdx+1):
            for my in range(1,self.mdy+1):

                #if (((3 == mx) and (3 == my)) or ((4 == mx) and (4==my))):
                #if ((3 == mx) and (3 != my)):
                if (1):
                #if (my < (self.mdy - 1)):    
                    # Find upper left and lower right grid cells for the
                    # subdish in the multi-dish
                    
                    mdcul,mdulx,mduly = self.grid.clientCell2ServerCell(self.mdx,self.mdy,mx,my,self.sdx,self.sdy,0)
                    mdclr,mdlrx,mdlry = self.grid.clientCell2ServerCell(self.mdx,self.mdy,mx,my,self.sdx,self.sdy,self.sdx*self.sdy - 1)

                    print "UL %s ULX %s ULY %s LR %s LRX %s LRY %s" % (mdcul, mdulx, mduly, mdclr, mdlrx, mdlry) 
                    #mdlry /= 2
                    #print "UL %s ULX %s ULY %s LR %s LRX %s LRY %s" % (mdcul, mdulx, mduly, mdclr, mdlrx, mdlry) 

                    inflow = self.sdx * self.sdy * 1.0
                    outflow = 0.1

                    for ii in self.tasks:
                        tstr = "RESOURCE %s%s%s%s" + \
                            ":initial=0.00001" + \
                            ":geometry=torus" + \
                            ":inflow=%s" + \
                            ":outflow=%s" + \
                            ":inflowx1=%s" + \
                            ":inflowx2=%s" + \
                            ":inflowy1=%s" + \
                            ":inflowy2=%s" + \
                            ":outflowx1=%s" + \
                            ":outflowx2=%s" + \
                            ":outflowy1=%s" + \
                            ":outflowy2=%s" + \
                            ":xgravity=0" + \
                            ":ygravity=0" + \
                            ":xdiffuse=0" + \
                            ":ydiffuse=0\n"
                        # Set inflow and outflow differently based on whether the resource is "YES" in MD dict
                        if (base1):
                            # Base-1 for x,y position
                            currkey = "RES_%s%s_%s" % (mx,my,str(ii).upper())
                        else:
                            # Base-0 for x,y position
                            currkey = "RES_%s%s_%s" % (mx-1,my-1,str(ii).upper())
                        # Check for reward of this task in this subdish
                        # Two steps: does the key exist? Then, what is the value?
                        if (mdset.has_key(currkey) and ("YES" == mdset[currkey])):
                            print "currkey YES : %s" % (currkey)
                            inflow = self.sdx * self.sdy * 1.0
                            outflow = 0.1
                        else:
                            print "currkey NO : %s" % (currkey)
                            inflow = self.sdx * self.sdy * 0.000001
                            outflow = 0.9

                        f_env.write(tstr % ("res_",self.rrr[ii][0],mx,my,
                                            inflow,outflow,
                                            mdulx,mdlrx,
                                            mduly,mdlry,
                                            mdulx,mdlrx,
                                            mduly,mdlry))

                        print tstr % ("res_",self.rrr[ii][0],mx,my,
                                      inflow,outflow,
                                      mdulx,mdlrx,
                                      mduly,mdlry,
                                      mdulx,mdlrx,
                                      mduly,mdlry)


                f_env.write("\n")

        f_env.write("\n\n")
        
        for mx in range(1,self.mdx+1):
            for my in range(1,self.mdy+1):

                if (1):
                    for ii in self.tasks:
                        tstr = "REACTION  %s%s%s%s  %s   process" + \
                            ":resource=%s%s%s%s" + \
                            ":value=1.0" + \
                            ":max=1.0" + \
                            ":min=0.001" + \
                            ":frac=0.5:" + \
                            "product=%s%s%s%s" + \
                            ":conversion=1.0:" + \
                            "type=pow" + \
                            " requisite" + \
                            ":max_count=1\n" 
                        f_env.write(tstr % ("REA_",self.rrr[ii][1],mx,my,
                                            self.rrr[ii][0],
                                            "res_",self.rrr[ii][0],mx,my,
                                            "res_",self.rrr[ii][0],mx,my))
                        print tstr % ("REA_",self.rrr[ii][1],mx,my,
                                      self.rrr[ii][0],
                                      "res_",self.rrr[ii][0],mx,my,
                                      "res_",self.rrr[ii][0],mx,my)

                f_env.write("\n")

        f_env.write("\n\n")
        f_env.close()
