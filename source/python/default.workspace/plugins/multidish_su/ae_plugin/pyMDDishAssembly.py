"""
pyMDDishAssembly

Multi-dish petri dish assembly module
"""

import os
from pyReadFreezer import pyReadFreezer
from pyWriteToFreezer import pyWriteToFreezer
import pyGridMaster
import pyMDLineage
import sys
import string
from descr import *

class MDDishAssembly():
    """
    MDDishAssembly

    Multi-dish dish assembly class
    """

    def __init__(self):
        pass
        
    def Configure(self,sdx,sdy,sdnx,sdny):
        # How many sub-dishes make up the multi-dish?
        self.sd_x = sdx
        self.sd_y = sdy
        self.sd_total = self.sd_x * self.sd_y

        # What is the size of each subdish?
        self.sd_sizex = sdnx
        self.sd_sizey = sdny

        # What is the size of the multi-dish?
        self.md_cells = self.sd_total * self.sd_sizex * self.sd_sizey

        # Memory limits?
        # 4GB RAM, SD 4x4, max size = 42x42
        # 4GB RAM, SD 5x5, max size = 22x22

        # Clear sub-dish registry
        self.sdreg = {}
        self.sdary = {}
        self.sdpos = {}
        self.sdbypos = {}
        self.sd_cnt = 0

        # Get gridmaster object
        self.mdgrid = pyGridMaster.Grids(self.sd_x,self.sd_y,self.sd_sizex,self.sd_sizey)

        # Get Lineage object
        self.lineage = pyMDLineage.MDLineage(self.sd_total)

    def addSubDish(self,sd_name,sd_file,sd_pos = None):
        """
        addSubDish

        """
        print "addSubDish(%s,%s,%s)" % (sd_name,sd_file,sd_pos)

        

        # Have we already got all the subdishes we can handle?
        # -- allowing overwrite instead
        #if (self.sd_cnt >= self.sd_total):
        #    print "addSubDish error: too many subdishes"
        #    return(-1)

        # File must exist
        if (False == os.path.exists(sd_file)):
            print "addSubDish error: file %s does not exist" % (sd_file)
            return(-2)

        # Checking complete, so add it

        self.sdreg[sd_name] = sd_file
        self.sdary[sd_pos] = sd_name
        self.sdpos[sd_name] = sd_pos   # This doesn't work, as subdishes are not necessarily unique

        self.sd_cnt += 1

    def makeMultiDish(self,md_name,workspace_dir):
        """
        makeMultiDish
        
        """
        print "makeMultiDish(%s,%s)" % (md_name,workspace_dir)

        # Clear temporary dictionaries
        settings_dict = {}
        organisms_dict = {}
        anc_names_dict = {}
        anc_links_dict = {}
        cells_dict = {}
        md_dict = {}

        # Check various things

        # Multi-dish needs to have ".full" suffix
        md_name = str(md_name)
        if (False == md_name.endswith(".full")):
            md_name = md_name + ".full"

        # The workspace should exist
        if (False == os.path.exists(workspace_dir)):
            print "makeMultiDish error: path %s does not exist" % (workspace_dir)
            return(-1)

        # Full MD name
        mdfullpath = workspace_dir + '/' + md_name
        # The dish name should not exist
        if (0 and (True == os.path.exists(mdfullpath))):
            print "makeMultiDish error: file %s does exist" % (mdfullpath)
            return(-2)

        # Add a check here that the sub-dish is not too large
        # TBD
        # Size of multi-dish
        settings_dict['WORLD-X'] = self.sd_sizex * self.sd_x
        settings_dict['WORLD-Y'] = self.sd_sizey * self.sd_y
        md_cell_limit = settings_dict['WORLD-X'] * settings_dict['WORLD-Y']
        
        print "makeMultiDish : checks OK, processing"
        # Checks complete, so make the multi-dish
        # Create the directory
        print "Making directory: %s" % (mdfullpath)
        os.makedirs(str(mdfullpath))

        # Make the full filename
        mdfname = "%s/%s.full" % (workspace_dir,md_name)
        
        # Create "petri_dish" file
        pdname = mdfullpath + '/petri_dish'

        # Base settings

        # Turn off all global rewards; rewards to be established spatially
        settings_dict['REWARD_NOT'] = "NO"
        settings_dict['REWARD_NAND'] = "NO"
        settings_dict['REWARD_OR'] = "NO"
        settings_dict['REWARD_AND'] = "NO"
        settings_dict['REWARD_ORN'] = "NO"
        settings_dict['REWARD_ANDN'] = "NO"
        settings_dict['REWARD_NOR'] = "NO"
        settings_dict['REWARD_XOR'] = "NO"
        settings_dict['REWARD_EQU'] = "NO"

        settings_dict["COPY_MUT_PROB"] = 0.02

        settings_dict["BIRTH_METHOD"] = 4
        settings_dict["MAX_UPDATES"] = -1
        settings_dict["RANDOM_SEED"] = 0

        # New settings for multi-dish
        # Putting these in both "SETTINGS" and "MULTI_DISH"
        # It turns out that while all the keys and values are read from the 
        # multi-dish file, somewhere in processing only certain keys are
        # retained in the "SETTINGS" dict. Therefore, I can either track
        # down that and change it, or pass along the information in another
        # fashion. So I'm going with "another fashion" for now in the 
        # interest of saving time.
        settings_dict["MULTI_DISH"] = "YES"
        md_dict["MULTI_DISH"] = "YES"
        # The number of sub-dishes in each row of the multi-dish
        settings_dict["MD_SD_X"] = self.sd_x
        md_dict["MD_SD_X"] = self.sd_x
        # The number of sub-dishes in each column of the multi-dish
        settings_dict["MD_SD_Y"] = self.sd_y
        md_dict["MD_SD_Y"] = self.sd_y
        # The number of grid cells in each row of a sub-dish
        settings_dict["MD_SD_SIZEX"] = self.sd_sizex
        md_dict["MD_SD_SIZEX"] = self.sd_sizex
        # The number of grid cells in each column of a sub-dish 
        settings_dict["MD_SD_SIZEY"] = self.sd_sizey
        md_dict["MD_SD_SIZEY"] = self.sd_sizey

        # Need to globally track multi-dish number of organisms
        md_org_n = 1

        # Flag for grabbing settings 
        settings_done = 0

        # Go through each sub-dish and collect the information needed to make the multi-dish
        # Calculate highest cell index to pay attention to in subdish.
        # - this handles the case where a subdish is used that is larger than those
        # specified for this multi-dish
        cell_limit = self.sd_sizex * self.sd_sizey
        self.sds = {}
        for sdn in self.sdreg:  # sdn holds a sub-dish name
            # Have sub-dish, read the information
            sdnfull = "%s.full" % (sdn)
            print "sub-dish: %s" % (sdnfull)
            print "sdreg[sdn] = %s" % (self.sdreg[sdn])
            petridish = "%s/petri_dish" % (self.sdreg[sdn])
            print "petridish = %s" % (petridish)
            self.sds[sdn] = pyReadFreezer(petridish)

        print "subdish position array"
        print self.sdary
        print "subdish position array keys"
        print self.sdary.keys()

        # Now go through and fill in the dictionaries for the multi-dish
        # for sdn in self.sdpos:  # sdn is a sub-dish name
        for sdp in self.sdary.keys():  # sdp is a sub-dish position

            #print "sdp : %s" % (sdp)

            sdn = self.sdary[sdp]     # sdn is a sub-dish name
            # sdp = self.sdpos[sdn]   # sdp is a sub-dish position
            # Calculate sub-dish x, y indices
            csdx = sdp % self.sd_x
            csdy = sdp / self.sd_x

            # print "SubDish position: (%s,%s)" % (csdx,csdy) 
            # print "x = %s = %s mod %s" % (csdx,sdp,self.sd_x)
            # print "y = %s = %s / %s" % (csdy,sdp,self.sd_x)
            sys.stdout.flush()


            # For each organism in sub-dish
            sdcells = {}
            sdorgs = {}
            sdset = {}
            
            sdcells = self.sds[sdn].dictionary["CELLS"]
            sdorgs = self.sds[sdn].dictionary["ORGANISMS"]
            sdset = self.sds[sdn].dictionary["SETTINGS"]

            # Check for right-sized sub-dish
            if ((str(sdset['WORLD-X']) != str(self.sd_sizex)) or (str(sdset['WORLD-Y']) != str(self.sd_sizey))):
                print "Sub-dish %s has wrong dimensions, skipping it: X %s != %s, Y %s != %s" % (sdn,sdset['WORLD-X'],self.sd_sizex,sdset['WORLD-Y'],self.sd_sizey)
                # How about a messagebox for this? TBD
                # warning("You are a bonehead and set the grid sizes wrong ")
                warning("The sub-dish expected size is %sx%s, but sub-dish %s is %sx%s " % (self.sd_sizex,self.sd_sizey,sdn,sdset['WORLD-X'],sdset['WORLD-Y']) )
                continue

            # Clear the sub-dish to multi-dish organism index dict
            sd_orgn = {}
            for orgn in sdorgs.keys():  # orgn has sub-dish organism number
                # Sanity check: will this map to a valid place in the multi-dish?
                # TBD
                # Problem: Organism maps to possibly many cells, so can we trim the 
                # organism list to only those that exist in valid cells?

                # sub-dish org to multi-dish org conversion
                sd_orgn[orgn] = md_org_n    # Associate it with the current multi-dish organism number
                md_org_n += 1               # Increment the current multi-dish organism number

                # Lineage label and name
                # Need new lineage identifier of organism in sub-dish
                ll = self.lineage.sd2lineage(orgn,sdp)
                # Ancestor name for the organism comprises the sub-dish name plus the subdish position plus the organism number
                lname = "%s-%s-%s" % (sdn,sdp,orgn)
                
                # Add values to the dictionaries
                # Genome
                organisms_dict[sd_orgn[orgn]] = sdorgs[orgn]
                # Ancestor_links
                anc_links_dict[sd_orgn[orgn]] = ll
                # ancestor_names
                anc_names_dict[ll] = lname
            # END: for orgn in sdorgs.keys()

            for cell in sdcells:  # cell is the sub-dish cell index
                # Get organism that goes in the cell
                orgn = sdcells[cell]  # orgn is the index of the organism
                orgg = sdorgs[orgn]   # orgg is the genome of the organism
                # Need MDC of organism in sub-dish, in base-1
                sdcellx = int(cell) % int(self.sd_sizex)
                sdcelly = int(cell) / int(self.sd_sizey)
                #print "cell = %s" % (cell)
                #print "sd x = %s = (%s mod %s)" % (sdcellx,cell,self.sd_sizex)
                #print "sd y = %s = (%s / %s)" % (sdcelly,cell,self.sd_sizey)
                sys.stdout.flush()
                
                # Need base-1 sub-dish position coordinates
                (mdc, mdx, mdy) = self.mdgrid.clientCell2ServerCell(int(self.sd_x),int(self.sd_y),int(csdx)+1,int(csdy)+1,int(self.sd_sizex),int(self.sd_sizey),int(cell))
                #print "(%s, %s, %s) = self.mdgrid.clientCell2ServerCell(int(%s),int(%s),int(%s),int(%s),int(%s),int(%s),int(%s))" % (mdc, mdx, mdy, self.sd_x,self.sd_y,csdx+1,csdy+1,self.sd_sizex,self.sd_sizey,cell)

                #print "md cell = %s, md x = %s, md y = %s" % (mdc, mdx, mdy)
                sys.stdout.flush()

                # Sanity check: will this map to a valid place in the multi-dish?
                if (mdc < md_cell_limit):
                    # Set it in the dict
                    cells_dict[mdc] = sd_orgn[orgn]
                else:
                    # Probably should warn the user that there is a problem here
                    # TBD
                    pass
            # END: for cell in sdcells

            # Handle settings
            resources = ['not','nand','and','orn','oro','ant','nor','xor','equ']
            sdsettings = {}
            # Default all resources to "NO", change to YES if it can be disproved
            for thisres in resources:
                newkey = "res_%s%s_%s" % (csdx+1,csdy+1,str(thisres))
                sdsettings[newkey] = "NO"

            for sdsetkey in sdset.keys():  #
                if (0 == settings_done):        # Use first sub-dish's settings for general global settings
                    if ("COPY_MUT_PROB" == sdsetkey):
                        settings_dict["COPY_MUT_PROB"] = sdset[sdsetkey]
                    #elif (0 == strcmp("BIRTH_METHOD",sdsetkey)):
                    elif ("BIRTH_METHOD" == sdsetkey):
                        settings_dict["BIRTH_METHOD"] = sdset[sdsetkey]
                    #elif (0 == strcmp("MAX_UPDATES",sdsetkey)):
                    elif ("MAX_UPDATES" == sdsetkey):
                        settings_dict["MAX_UPDATES"] = sdset[sdsetkey]
                    #elif (0 == strcmp("RANDOM_SEED",sdsetkey)):
                    elif ("RANDOM_SEED" == sdsetkey):
                        settings_dict["RANDOM_SEED"] = sdset[sdsetkey]

                # Keys for resources become "res_" plus base-zero sub-dish placement coordinates plus lower-case resource label.
                # These go in the new "MULTI_DISH" dictionary

                # print "looking at subdish setting %s = %s" % (sdsetkey,sdset[sdsetkey])
                if (("REWARD_NOT" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"not")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_NAND" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"nand")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_AND" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"and")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_OR" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"or")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_ORN" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"orn")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_ANDN" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"andn")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_NOR" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"nor")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_XOR" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"xor")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                elif (("REWARD_EQU" == sdsetkey) and ("YES" == sdset[sdsetkey])):
                    newkey = "res_%s%s_%s" % (csdx,csdy,"equ")
                    md_dict[newkey] = "YES"
                    # print "set %s = YES" % (newkey)
                


            # END: for sdset in settings_dict
                
            # Stop change in settings block
            settings_done += 1
                    
                    

        # END: for sdn in sdpos
        
        # All the information is available, so write out the multi-dish file
        # First, collect the dictionaries
        md_all = {}
        md_all["SETTINGS"] = settings_dict
        md_all["MULTI_DISH"] = md_dict
        md_all["ORGANISMS"] = organisms_dict
        md_all["CELLS"] = cells_dict
        md_all["ANCESTOR_LINKS"] = anc_links_dict
        md_all["ANCESTOR_NAMES"] = anc_names_dict

        # Pass along the dictionary collection and the complete file name
        # including the ".full" suffix
        md_name = str(mdfullpath)
        if (False == md_name.endswith(".full")):
            md_name = md_name + ".full"
        print "calling pyWriteToFreezer(md_all,%s)" % (md_name)
        pwtf = pyWriteToFreezer(md_all,str(md_name))

        # Should have the MD file in the freezer now.

        sys.stdout.flush()


