# -*- coding: utf-8 -*-

from qt import *
from pyNavBarView import *
from descr import *

class pyNavBarListView(QListView):
  def __init__(self, *args):
    apply(QListView.__init__,(self,) + args)
    self.setAcceptDrops( True )
    self.viewport().setAcceptDrops( True )

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl

    descr("in pyNavBarListView self is:",self)

    #@JMC testing
#    self.connect(
#      self.m_session_mdl.m_session_mdtr, PYSIGNAL("orgClickedOnSig"),
#      self.jmcTestSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("jmcTestSig"),
      self.jmcTestSlot)

  def jmcTestSlot(self, clicked_cell_item = None):
    descr("no way this works in list view++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")

  def contentsDropEvent(self, e):
    freezer_item_name = QString()
    if e.source() is self:
      return

    # freezer_item_name is a string...the file name 
 
    print type(e)
    descr("in contentsDrop event")
    descr("freezer_item_name is: ", freezer_item_name)
    test_item_name = 'jeff'
    descr("self is: ", self)
    #self.emit(PYSIGNAL("jmcTestSig"), (test_item_name,))
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("jmcTestSig"),())

    if ( QIconDrag.canDecode(e)):
      format = QDropEvent.format(e, 0)
    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      descr("in decode, freezer_item_name is: ", freezer_item_name[:])
      if freezer_item_name[:9] == 'organism.':
        descr("that was an organism")
        freezer_item_name = freezer_item_name[9:]
#        self.FreezeOrganism(freezer_item_name)
      else:
        pass

    # Check if item is icon

    #if (pyNewIconView.canDecode(e)):
    #  info("You can not drag an ancestor back into the freezer")

    
  #def FreezeOrganism(self, freezer_item_name):
  #  tmp_dict = {1:freezer_item_name}
  #  pop_up_organism_file_name = pyFreezeOrganismCtrl()
  #  file_name = pop_up_organism_file_name.showDialog(self.m_session_mdl)

  #  file_name_len = len(file_name.rstrip())
  #  if (file_name_len > 0):
  #    freezer_file = pyWriteToNavBar(tmp_dict, file_name)
  #  
  #  self.m_session_mdl.m_session_mdtr.emit(
  #    PYSIGNAL("doRefreshNavBarInventorySig"), ())


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
       
        self.m_list_view.setSelectionMode(QListView.Single)
        self.m_list_view.setSortColumn(-1)
        self.m_list_view.clear()

        self.m_one_analyze_cli = QListViewItem(self.m_list_view, "Analysis")
        self.m_one_organism_cli = QListViewItem(self.m_list_view, "Organism")
        self.m_one_population_cli = QListViewItem(self.m_list_view, "Population")

        self.m_one_population_cli.setPixmap(0, self.image0)
        self.m_one_organism_cli.setPixmap(0, self.image1)
        self.m_one_analyze_cli.setPixmap(0, self.image2)



