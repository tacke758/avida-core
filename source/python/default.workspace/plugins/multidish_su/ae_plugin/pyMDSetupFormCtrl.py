"""
pyMDSetupFormCtrl

Code for setup form user interface
"""


from qt import *
import os
import sys
from pyMDDishAssembly import MDDishAssembly
from pyDiversitySandbox import *
#from pyWutils import *

def before(instr,pat):
    """
    before
    
    Returns the part of the string that occurs before the first
    appearance of pat, or the complete string.
    """
    idx = instr.find(pat)
    if (-1 == idx):
        return(instr)
    else:
        return(instr[:idx])
    
def after(instr,pat):
    """
    after
    
    Returns the part of the string that follows pat, or the null
    string.
    """
    idx = instr.find(pat)
    if (-1 == idx):
        return("")
    else:
        return(instr[idx + len(pat):])
    
def psplit(instr,pat):
    """
    psplit
    
    Returns a tuple with two strings, the 
    """
    b = instr.before(pat)
    a = instr.after(pat)
    return((b,a,))


class pyMDSetupFormCtrl():
    """
    pyMDSetupFormCtrl

    Class to instantiate various user interface things regarding multi-dish setup
    """

    def __init__(self):
        print "pyMDSetupFormCtrl.__init__()"
        pass

    def construct(self, view_form, session_model):
        """
        construct
        """
        print "pyMDSetupFormCtrl.construct()"
        self.view_form = view_form
        self.session_model = session_model
        self.mdda = MDDishAssembly()


    def MDSSetValuesButtonClick(self):
        """
        MDSSetValuesButtonClick

        Method to set values in object based on the setup form
        """
        print "MDSSetValuesButtonClick"
        self.sdx = self.view_form.sdx_spinBox.value()
        self.sdy = self.view_form.sdy_spinBox.value()
        self.sdnx = self.view_form.sdnx_spinBox.value()
        self.sdny = self.view_form.sdny_spinBox.value()
        self.mdx = self.sdx * self.sdnx
        self.mdy = self.sdy * self.sdny
        self.sdtot = self.sdx * self.sdy

        self.mdname = self.view_form.md_name_lineEdit.text()

        ostr = "MD: SDx=%s, SDy=%s, SDnx=%s, SDny=%s, size=%sx%s" % (self.sdx,
                                                                     self.sdy,
                                                                     self.sdnx,
                                                                     self.sdny,
                                                                     self.mdx,
                                                                     self.mdy)
        #print ostr
        self.view_form.md_values_textLabel.setText(ostr)

        # Clear the layout text
        self.view_form.md_layout_textEdit.clear()

        # SubDish position dict
        self.mdsdpos = {}
        for ii in range(self.sdtot):
            self.mdsdpos[ii+1] = "%12s %2d" % ("_",ii+1)

        self.MDDisplayLayout()

        # Update dish assembly object
        self.mdda.Configure(self.sdx,self.sdy,self.sdnx,self.sdny)


    def MDSGenerateMDButtonClick(self):
        """
        MDSGenerateMDButtonClick

        Method to actually write the multi-dish to the current workspace
        """
        print "MDSGenerateMDButtonClick"
        self.mdda.makeMultiDish(self.mdname,self.session_model.m_current_freezer)

        # Refresh the freezer list
        self.session_model.m_session_mdtr.emit(
            PYSIGNAL("doRefreshFreezerInventorySig"), ())

    def MDSSubDishListRefreshButtonClick(self):
        """
        MDSSubDishListRefreshButtonClick

        Method to collect the full dish list from the current
        workspace and present it in the combo-box
        """
        print "MDSSubDishListRefreshButtonClick"
        # Clear the combo box
        self.view_form.sd_pick_comboBox.clear()

        # Get all the names of populated dishes from the current workspace
        # oscwd = os.getcwd()
        # print "current directory : %s" % oscwd
        # print "current workspace : %s" % (self.session_model.m_current_workspace)
        # print "current freezer : %s" % (self.session_model.m_current_freezer)
        # Current freezer is what we want
        dirlist = os.listdir(self.session_model.m_current_freezer)
        
        fulllist = []

        # Remove all but "full" files from dirlist
        for ii in range(len(dirlist)):
            #print "item %s = %s" % (ii,dirlist[ii])
            if (dirlist[ii].endswith(".full")):
                #print " was kept"
                fulllist.append(dirlist[ii])
                pass
        #print fulllist
        sys.stdout.flush()

        # Now populate the empty combobox
        self.view_form.sd_pick_comboBox.insertStrList(fulllist)

        # Refresh the freezer list
        #self.session_model.m_session_mdtr.emit(
        #    PYSIGNAL("doRefreshFreezerInventorySig"), ())


    def MDSSubDishAddButtonClick(self):
        """
        MDSSubDishAddButtonClick

        Method to associate a subdish with a position in the
        mutli-dish
        """
        print "MDSSubDishAddButtonClick"

        # Track the new assignment in the layout
        sdpos = self.view_form.sd_pos_in_md_spinBox.value()
        self.mdsdpos[sdpos] = "%12s %2d" % (before(str(self.view_form.sd_pick_comboBox.currentText()),".full"),sdpos)

        self.MDDisplayLayout()

        file = "%ss" % (str(self.view_form.sd_pick_comboBox.currentText()))
        print file
        sys.stdout.flush()
        (name,null) = file.split(".full")

        file = "%s/%s" % (self.session_model.m_current_freezer,str(self.view_form.sd_pick_comboBox.currentText()))
        
        # Dish assembly object expects base zero position
        print "adding %s %s %s" % (name,file,sdpos-1)
        sys.stdout.flush()
        self.mdda.addSubDish(name,file,sdpos-1)

    def MDDisplayLayout(self):
        """
        MDDisplayLayout
        """
        # Clear the layout text
        self.view_form.md_layout_textEdit.clear()

        cnt = 1
        allstr = ""
        for jj in range(self.sdy):
            tstr = ""
            tlst = []
            for ii in range(self.sdx):
                tlst.append(self.mdsdpos[cnt])
                cnt += 1
            tstr = " ".join(["%s " % (k) for k in tlst])
            allstr = allstr + tstr + "\n\n"
        self.view_form.md_layout_textEdit.setText(allstr)


        
    def updateMDRunStatsSlot(self):
        tasklabels = ("Not", "Nan", "And", "Orn", "Oro", "Ant", "Nor", "Xor", "Equ", )

        self.view_form.MDRunStatistics_textEdit.clear()
        #self.view_form.MDRunStatistics_textEdit.append("In updateMDRunStatsSlot.")

        # Now add to the textedit the information gathered about this run
        # Current update
        # Current population
        # # of organisms performing each task
        # Ave. age
        # Ave. gestation time
        # Ave. fitness


        # Information is passed in dictionary
        # self.session_model.md_runstats 
        
        tstr = "Update: %6s | Total Orgs: %6s" % (self.session_model.md_runstats["UPDATE"],self.session_model.md_runstats["TOTAL_ORGS"])
        self.view_form.MDRunStatistics_textEdit.append(tstr)

        tstr = " "
        self.view_form.MDRunStatistics_textEdit.append(tstr)

        #for tstr in self.session_model.md_runstats["TASKS"]:
        #    self.view_form.MDRunStatistics_textEdit.append(tstr)

        tstr = ""
        t2str = ""
        for ii in range(len(tasklabels)):
            tstr = tstr + "%8s | " % (tasklabels[ii])
            t2str = t2str + "%8s | " % (self.session_model.md_runstats["TASKSRAW"][ii])

        self.view_form.MDRunStatistics_textEdit.append(tstr)
        self.view_form.MDRunStatistics_textEdit.append(t2str)

        tstr = " "
        self.view_form.MDRunStatistics_textEdit.append(tstr)

        tstr = " %20s | %11s | %11s | %11s " % ("Avg. Fitness",
                                                "Avg. Merit.",
                                                "Avg. Gest.", 
                                                "Avg. Age")
        t2str = " %20s | %11s | %11s | %11s " % (self.session_model.md_runstats["AVERAGES"]["AVG_FITNESS"],
                                                 self.session_model.md_runstats["AVERAGES"]["AVG_MERIT"],
                                                 self.session_model.md_runstats["AVERAGES"]["AVG_GESTATION"],
                                                 self.session_model.md_runstats["AVERAGES"]["AVG_AGE"])

        self.view_form.MDRunStatistics_textEdit.append(tstr)
        self.view_form.MDRunStatistics_textEdit.append(t2str)



    def updateMDDiversitySlot(self):
        # print "updateMDDiversitySlot"
        self.view_form.MDDiversity_textEdit.clear()
        if (False == self.session_model.m_is_multi_dish):
            print "Not a mutli-dish, therefore exiting"
            return
        if (0 >= self.session_model.md_runstats["UPDATE"]):
            # Run hasn't started
            print "Run at %s has not started" % (self.session_model.md_runstats["UPDATE"])
            return
        if (0 >= len(self.session_model.md_diversity["POP_GEN"]["TOTAL"])):
            print "No data to work with, skip it."
            return

        #self.view_form.MDDiversity_textEdit.append("In updateMDDiversitySlot.")

        # Now add to the textedit the information gathered about diversity
        # Information is passed in dictionary
        # self.session_model.md_diversity

        divcalc = diversity()

        tstr = "Total pop. size = %s" % (self.session_model.md_diversity["POP_COUNTS"]["TOTAL"])
        self.view_form.MDDiversity_textEdit.append(tstr)

        tstr = " "
        self.view_form.MDDiversity_textEdit.append(tstr)

        popgen_basediv = divcalc.base_diversity(self.session_model.md_diversity["POP_GEN"]["TOTAL"])
        tstr = "Whole pop. Diversity Measure 1 (DM01) of genomes = %s" % (popgen_basediv)
        self.view_form.MDDiversity_textEdit.append(tstr)

        (pcnt,ave_pi) = divcalc.tide_diversity(self.session_model.md_diversity["POP_GEN"]["TOTAL"], 0.1)

        tstr = "Whole pop. Diversity Measure 2 (DM02) of genomes pair count = %s, DM02 = %s" % (pcnt,ave_pi)
        self.view_form.MDDiversity_textEdit.append(tstr)

        tstr = " "
        self.view_form.MDDiversity_textEdit.append(tstr)

        popphen_basediv = divcalc.base_diversity(self.session_model.md_diversity["POP_PHEN"]["TOTAL"])
        tstr = "Whole pop. DM01 of phenotypes = %s" % (popphen_basediv)
        self.view_form.MDDiversity_textEdit.append(tstr)

        (pcnt,ave_pi) = divcalc.tide_diversity(self.session_model.md_diversity["POP_PHEN"]["TOTAL"], 0.1)
        tstr = "Whole pop. DM02 of phenotypes pair count = %s, DM02 = %s" % (pcnt,ave_pi)
        self.view_form.MDDiversity_textEdit.append(tstr)
        
        tstr = " "
        self.view_form.MDDiversity_textEdit.append(tstr)

        # Native v. immigrant composition of subdishes
        # How many subdishes are there?
        md_sds = int(self.session_model.m_multi_dish_dict["MD_SD_X"]) * int(self.session_model.m_multi_dish_dict["MD_SD_Y"])

        # Native/immigrant data in one long table
        if (0):
          tstr =  "           |"
          t2str = "Natives    |"
          t3str = "Immigrants |"
          for ii in range(md_sds):
              tstr = tstr + " SD %2s |" % (ii+1)
              t2str = t2str + " %5s |" % (self.session_model.md_diversity["NATIVES"][str(ii)])
              t3str = t3str + " %5s |" % (self.session_model.md_diversity["IMMIGRANTS"][str(ii)])
          self.view_form.MDDiversity_textEdit.append(tstr)
          self.view_form.MDDiversity_textEdit.append(t2str)
          self.view_form.MDDiversity_textEdit.append(t3str)
  
          tstr = " "
          self.view_form.MDDiversity_textEdit.append(tstr)

        # Native / immigrant data laid out as in multi-dish
        sdx = int(self.session_model.m_multi_dish_dict["MD_SD_X"])
        sdy = int(self.session_model.m_multi_dish_dict["MD_SD_Y"])
        for ii in range(sdy): # number of rows
            tstr =  "           |"
            t2str = "Natives    |"
            t3str = "Immigrants |"
            t4str = "Emigrants  |"
            for jj in range(sdx): # number of columns
                sdpos = (sdx * ii) + jj
                tstr = tstr + " SD %2s |" % (sdpos+1)
                t2str = t2str + " %5s |" % (self.session_model.md_diversity["NATIVES"][str(sdpos)])
                t3str = t3str + " %5s |" % (self.session_model.md_diversity["IMMIGRANTS"][str(sdpos)])
                t4str = t4str + " %5s |" % (self.session_model.md_diversity["EMIGRANTS"][str(sdpos)])
            self.view_form.MDDiversity_textEdit.append(tstr)
            self.view_form.MDDiversity_textEdit.append(t2str)
            self.view_form.MDDiversity_textEdit.append(t3str)
            self.view_form.MDDiversity_textEdit.append(t4str)

            #tstr = " "
            #self.view_form.MDDiversity_textEdit.append(tstr)
                

        tstr = " "
        self.view_form.MDDiversity_textEdit.append(tstr)

        tstr = "Report of Diversity Measures (Genome length = %s, Phenotype length = %s" % (len(self.session_model.md_diversity["POP_GEN"]["TOTAL"][0]),
                                                                                            len(self.session_model.md_diversity["POP_PHEN"]["TOTAL"][0]))
        self.view_form.MDDiversity_textEdit.append(tstr)
        tstr = " "
        self.view_form.MDDiversity_textEdit.append(tstr)

        # Gen/Phen diversity per subdish
        sdx = int(self.session_model.m_multi_dish_dict["MD_SD_X"])
        sdy = int(self.session_model.m_multi_dish_dict["MD_SD_Y"])
        for ii in range(sdy): # number of rows
            tstr =  "               |"
            t2str = "Genome DM01    |"
            t3str = "Genome DM02    |"
            
            t4str = "Phenotype DM01 |"
            t5str = "Phenotype DM02 |"
            
            for jj in range(sdx): # number of columns
                sdpos = (sdx * ii) + jj

                if (self.session_model.md_diversity["POP_GEN"].has_key(str(sdpos))):
                    if (0 < len(self.session_model.md_diversity["POP_GEN"][str(sdpos)])):
                        #print "subdish genotype data for %s" % (sdpos)
                        #print self.session_model.md_diversity["POP_GEN"][str(sdpos)]
                        popgen_basediv = divcalc.base_diversity(self.session_model.md_diversity["POP_GEN"][str(sdpos)])
                        #tstr = "Whole pop. 'base' diversity of genomes H' = %s" % (popgen_basediv)
                        #self.view_form.MDDiversity_textEdit.append(tstr)
  
                        (gpcnt,gave_pi) = divcalc.tide_diversity(self.session_model.md_diversity["POP_GEN"][str(sdpos)], 0.1)
      
                        #tstr = "Whole pop. 'tide' diversity of genomes pair count = %s, ave. p_i = %s" % (pcnt,ave_pi)
                        #self.view_form.MDDiversity_textEdit.append(tstr)
                      
                        #tstr = " "
                        #self.view_form.MDDiversity_textEdit.append(tstr)
        
                        popphen_basediv = divcalc.base_diversity(self.session_model.md_diversity["POP_PHEN"][str(sdpos)])
                        #tstr = "Whole pop. 'base' diversity of phenotypes H' = %s" % (popphen_basediv)
                        #self.view_form.MDDiversity_textEdit.append(tstr)
                        
                        (ppcnt,pave_pi) = divcalc.tide_diversity(self.session_model.md_diversity["POP_PHEN"][str(sdpos)], 0.1)
                        #tstr = "Whole pop. 'tide' diversity of phenotypes pair count = %s, ave. p_i = %s" % (pcnt,ave_pi)
                        #self.view_form.MDDiversity_textEdit.append(tstr)
                    else:
                        popgen_basediv = "-"
                        popphen_basediv = "-"
                        gpcnt = "-"
                        gave_pi = "-"
                        ppcnt = "-"
                        pave_pi = "-"
                        
                else:
                    print "No such key: %s as subdish index in diversity data" % (sdpos)
                    pass
  
  
                tstr = tstr +   "   SD %2s      |" % (sdpos+1)
                t2str = t2str + "       %6s |" % (str(popgen_basediv)[:6])
                t4str = t4str + "       %6s |" % (str(popphen_basediv)[:6])
                t3str = t3str + " %4s, %6s |" % (gpcnt,str(gave_pi)[:6])
                t5str = t5str + " %4s, %6s |" % (ppcnt,str(pave_pi)[:6])

            self.view_form.MDDiversity_textEdit.append(tstr)
            self.view_form.MDDiversity_textEdit.append(t2str)
            self.view_form.MDDiversity_textEdit.append(t3str)
            self.view_form.MDDiversity_textEdit.append(t4str)
            self.view_form.MDDiversity_textEdit.append(t5str)

            #tstr = " "
            #self.view_form.MDDiversity_textEdit.append(tstr)


