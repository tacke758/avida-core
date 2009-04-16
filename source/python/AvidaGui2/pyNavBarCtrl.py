
# -*- coding: utf-8 -*-

import os
from pyReadFreezer import pyReadFreezer
from qt import *
from pyNavBarView import image0_data, image1_data, image2_data
from descr import *

class pyNavBarListView(QListView):
  def __init__(self, *args):
    apply(QListView.__init__,(self,) + args)
    self.viewport().setAcceptDrops( True )

    self.m_viewer_dict = {}

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl

  #this code allows you to control the universal no in sub areas of the menu, but it does not fire if
  #accept drops in the view port is on, so we can't use it unless we figure out how to make it accept drops in a way that does
  #not turn itself off
#  def dragMoveEvent(self,e):
#    if e.source() is self:
#      return

#    descr("JMC")
    # Check if you want the drag...
#    dropped_item_name = QString()

#    QTextDrag.decode(e,dropped_item_name)

#    picked_item = self.itemAt(e.pos())
#    descr("picked_item",picked_item)
#    if(picked_item is not None):
#      viewer_dropped_in =  str(picked_item.text(0))
#      descr("viewer dropped in is: ", viewer_dropped_in)
#      if dropped_item_name[:9] == 'organism.':
  #      descr("that was an organism dragged from the petri dish")
#        if(viewer_dropped_in == 'Organism'):
#          e.accept()
#          self.viewport().setAcceptDrops( False ) 
#        else:
#          e.ignore()
#          self.viewport().setAcceptDrops( False ) 
#      elif dropped_item_name[-9:] == '.organism':
  #      descr("that was an organism dragged from the freezer")
#        if(viewer_dropped_in == 'Organism'):
#          e.accept()
#          descr("proposed action is: ",e.proposedAction())
#          self.viewport().setAcceptDrops( True ) 
#        else:
#          descr("---")
#          e.ignore()
#          self.viewport().setAcceptDrops( False ) 
#      else:
#        e.ignore()
#        self.viewport().setAcceptDrops( False ) 

 #   if (canDecode(e)):
 #     descr("JMC -- canDecode")
 #     e.accept()

  def contentsDropEvent(self, e):
    dropped_item_name = QString()
    if e.source() is self:
      return

    # dropped_item_list is a string...a file name or raw genotype 

    picked_item = self.itemAt(e.pos())
    viewer_dropped_in = 'nada'
    if(picked_item is not None):
      viewer_dropped_in =  str(picked_item.text(0))
          
    if ( QIconDrag.canDecode(e)):
      format = QDropEvent.format(e, 0)

    # this if statement actually does the decoding too

    if ( QTextDrag.decode( e, dropped_item_name ) ) : 
      
      if(viewer_dropped_in == 'Organism'):
        if (len(str(dropped_item_name).split("\t")[1:]) > 1):
          info ("You cannot drag more than one item into the Organism Viewer.")
        elif (dropped_item_name[:9] == 'organism.' or dropped_item_name[-9:] == '.organism'): #drop was an organism
          self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("raiseOrgViewSig"),())
          if dropped_item_name[:9] == 'organism.':       #organism from petri dish
            dropped_item_name = dropped_item_name[9:]
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("parseOrganismGenomeSig"), (str(dropped_item_name),))
          elif dropped_item_name[-9:] == '.organism':    #organism from freezer
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("parseOrganismFileSig"), (str(dropped_item_name[1:]),))
        else:
          info ("Only organisms can be dragged into the Organism Viewer.")

      elif(viewer_dropped_in == 'Population'):
        if (dropped_item_name[-5:] == '.full' or dropped_item_name[-6:] == '.empty'):

          # put the dish name and path into the format doDefrostDishSig wants

          file_name = str(dropped_item_name[1:])
          if(len(file_name.split("\t"))>1):
            info ("You cannot drag more than one dish into the Population Viewer.")
          else:
            #note, the pop viewer gets raised on the receiving end in DefrostSlot in pyOnePop_PetriDishCtrl.py
            dish_name = os.path.basename(file_name)
            dish_name = os.path.splitext(dish_name)[0]
            if (file_name.endswith(".full")):
              file_name = os.path.join(file_name, "petri_dish")
            thawed_item = pyReadFreezer(file_name)

            #send defrost sig
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doDefrostDishSig"),
              (dish_name, thawed_item,))
        else:
          info ("Only petri dishes (populated or configured) can be dragged into the Population Viewer.")
          
      elif(viewer_dropped_in == 'Analysis'):
        if (dropped_item_name[-5:] == '.full'):
          self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("itemDroppedInAnalysisViewerMenuBox"),
            (str(dropped_item_name),))
          self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("raiseAnaViewSig"),())
        else:
          info ("Only populated petri dishes can be dragged into the Analysis Viewer.")

      else:
        # Check whether <viewer_dropped_in> is in dictionary.
        # Lookup <viewer_dropped_in> is in dictionary.
        # Call HandleDropEvent method of object in dictionary, passing <dropped_item_name> as argument
        pass
          
          
class pyNavBarCtrl(QWidget):

    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        self.image0 = QPixmap(image0_data)
        self.image1 = QPixmap(image1_data)
        self.image2 = QPixmap(image2_data)

        if not name:
            self.setName("pyNavBarView")

        self.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.MinimumExpanding,0,0,self.sizePolicy().hasHeightForWidth()))

        pyNavBarViewLayout = QHBoxLayout(self,0,6,"pyNavBarViewLayout")

        self.m_list_view = pyNavBarListView(self,"m_list_view")
        self.m_list_view.addColumn(self.__tr("Viewers"))
        self.m_list_view.header().setClickEnabled(0,self.m_list_view.header().count() - 1)
        self.m_list_view.header().setResizeEnabled(0,self.m_list_view.header().count() - 1)
        self.m_list_view.setSizePolicy(QSizePolicy(QSizePolicy.Preferred,QSizePolicy.Preferred,0,0,self.m_list_view.sizePolicy().hasHeightForWidth()))
        m_list_view_font = QFont(self.m_list_view.font())
        m_list_view_font.setPointSize(9)
        self.m_list_view.setFont(m_list_view_font)
        self.m_list_view.setAcceptDrops(1)
        self.m_list_view.setFrameShape(QListView.StyledPanel)
        self.m_list_view.setFrameShadow(QListView.Sunken)
        self.m_list_view.setHScrollBarMode(QListView.AlwaysOff)
        self.m_list_view.setSelectionMode(QListView.Single)
        self.m_list_view.setAllColumnsShowFocus(1)
        self.m_list_view.setRootIsDecorated(0)
        self.m_list_view.setResizeMode(QListView.AllColumns)
        pyNavBarViewLayout.addWidget(self.m_list_view)

        self.languageChange()

        self.resize(QSize(210,390).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)


    def languageChange(self):
        self.setCaption(self.__tr("pyNavBarView"))
        self.m_list_view.header().setLabel(0,self.__tr("Viewers"))
        self.m_list_view.clear()
        item = QListViewItem(self.m_list_view,None)
        item.setText(0,self.__tr(" Populations"))
        item.setPixmap(0,self.image0)

        item = QListViewItem(self.m_list_view,item)
        item.setText(0,self.__tr(" Organisms"))
        item.setPixmap(0,self.image1)

        item = QListViewItem(self.m_list_view,item)
        item.setText(0,self.__tr(" Analysis"))
        item.setPixmap(0,self.image2)

        QToolTip.add(self.m_list_view,self.__tr("Go to major sections of Avida-ED"))

    def __tr(self,s,c = None):
        return qApp.translate("pyNavBarView",s,c)


    def construct(self, session_mdl):
        self.m_session_mdl = session_mdl
       
        self.m_list_view.construct(session_mdl)
        self.m_list_view.setSelectionMode(QListView.Single)
        self.m_list_view.setSortColumn(-1)
        self.m_list_view.clear()

        self.m_one_analyze_cli = QListViewItem(self.m_list_view, "Analysis")
        self.m_one_organism_cli = QListViewItem(self.m_list_view, "Organism")
        self.m_one_population_cli = QListViewItem(self.m_list_view, "Population")

        self.m_one_population_cli.setPixmap(0, self.image0)
        self.m_one_organism_cli.setPixmap(0, self.image1)
        self.m_one_analyze_cli.setPixmap(0, self.image2)



