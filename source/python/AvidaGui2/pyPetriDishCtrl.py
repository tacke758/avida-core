
from descr import *

from AvidaCore import cConfig
from AvidaCore import cInitFile, cString

from qt import *
from math import exp
from qtcanvas import QCanvas, QCanvasRectangle
from pyPetriCanvasView import pyPetriCanvasView
from pyPopulationCellItem import pyPopulationCellItem
#from pyPetriDishView import pyPetriDishView

class pySquareVBoxLayout(QVBoxLayout):
  def __init__(self, *args): apply(QVBoxLayout.__init__, (self,)+args)
  def hasHeightForWidth(self): return True
  def heightForWidth(self, w): return w

class pyPetriDishCtrl(QWidget):
  def __init__(self,parent = None,name = None,fl = 0):
    #pyPetriDishView.__init__(self,parent,name,fl)
    QWidget.__init__(self,parent,name,fl)
    if not name: self.setName("pyPetriDishCtrl")
    QToolTip.add(self, "Shows selected variable for organisms in the population")

    #self.resize(QSize(202,202).expandedTo(self.minimumSizeHint()))
    self.clearWState(Qt.WState_Polished)

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None

    self.m_canvas = None
    self.m_cell_info = None
    self.m_zoom_factor = 11
    self.m_target_dish_width = 350
    self.m_target_dish_scaling = 5.
    self.m_map_cell_width = 5
    self.m_h_scrollbar_offset = 0
    self.m_v_scrollbar_offset = 0
    self.m_zoom_old_value = self.m_zoom_factor
    self.m_stat_task_button_states = [0,0,0,0,0,0,0,0,0]

    self.m_petri_dish_layout = pySquareVBoxLayout(self,0,0,"m_petri_dish_layout")
    print "pyPetriDishCtrl.construct() self.m_petri_dish_layout.heightForWidth(20) :", self.m_petri_dish_layout.heightForWidth(20)
    self.m_canvas_view = pyPetriCanvasView(None, self,"m_canvas_view",session_mdl)

    #hiding the scroll bars, the pre-packaged ones were not working so we are adding them manually elsewhere
    self.m_canvas_view.setVScrollBarMode(QScrollView.AlwaysOff)
    self.m_canvas_view.setHScrollBarMode(QScrollView.AlwaysOff)

    #adding manual ones
    self.m_scroll_bar_width = 15
    # the following settings get overridden once the scroll bars are necessary, so they are junk
    self.m_petri_dish_ctrl_h_scrollBar = QScrollBar(0,371,0,20,185,Qt.Horizontal,self.m_canvas_view)
    self.m_petri_dish_ctrl_v_scrollBar = QScrollBar(0,371,0,20,185,Qt.Vertical,self.m_canvas_view)
    # end junk settings

    self.m_petri_dish_ctrl_h_scrollBar.setGeometry(0,365, 380,self.m_scroll_bar_width)
    self.m_petri_dish_ctrl_v_scrollBar.setGeometry(365,0,self.m_scroll_bar_width,365)

    self.m_petri_dish_layout.addWidget(self.m_canvas_view)
    self.m_changed_cell_items = []
    self.m_indexer = None
    self.m_color_lookup_functor = None
    self.m_background_rect = None
    self.m_change_list = None
    self.m_org_clicked_on_item = None
    self.m_occupied_cells_ids = []
    self.m_avida_has_started = False

    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("doStartAvidaSig"),
      self.doStartAvidaSlot)
    self.connect( self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"), 
      self.setAvidaSlot)
    self.connect( self.m_canvas_view, PYSIGNAL("orgClickedOnSig"), 
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("orgClickedOnSig"))
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("orgClickedOnSig"), self.updateOrgClickedOutlineCellNumberSlot)
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("orgClickedOnSig"), self.setDragSlot)
    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("statsViewTaskButtonStateChangeSig"), self.statsViewTaskButtonStateChangeSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("petriCanvasResizedSig"),
      self.petriCanvasResizedSlot)
    self.connect(self.m_petri_dish_ctrl_h_scrollBar, 
                 SIGNAL("valueChanged(int)"), 
                 self.moveCanvasHorizontallySlot)    
    self.connect(self.m_petri_dish_ctrl_h_scrollBar, 
                 SIGNAL("nextLine()"), 
                 self.hbarScrollNextLineSlot)    
    self.connect(self.m_petri_dish_ctrl_h_scrollBar, 
                 SIGNAL("prevLine()"), 
                 self.hbarScrollPrevLineSlot)    
    self.connect(self.m_petri_dish_ctrl_v_scrollBar, 
                 SIGNAL("valueChanged(int)"), 
                 self.moveCanvasVerticallySlot)    
    self.connect(self.m_petri_dish_ctrl_v_scrollBar, SIGNAL("nextLine()"), 
                 self.vbarScrollNextLineSlot)    
    self.connect(self.m_petri_dish_ctrl_v_scrollBar, SIGNAL("prevLine()"), 
                 self.vbarScrollPrevLineSlot)    

  def doStartAvidaSlot (self):
    self.m_avida_has_started = True



  def restart(self):
    self.m_cell_info = None
    self.m_changed_cell_items = []
    self.m_color_lookup_functor = None
    self.m_background_rect = None
    self.m_change_list = None
    self.m_org_clicked_on_item = None
    self.m_occupied_cells_ids = []

  def statsViewTaskButtonStateChangeSlot(self, stat_task_button_states):
    self.m_stat_task_button_states = stat_task_button_states
    descr("self.m_stat_task_button_states is: ", self.m_stat_task_button_states)
    self.updateCellItems(True)

  def petriCanvasResizedSlot(self,petriCanvasSize):
    descr()
    self.m_petri_dish_ctrl_h_scrollBar.setGeometry(0,petriCanvasSize.height()
      -self.m_scroll_bar_width+2, petriCanvasSize.width()+2,self.m_scroll_bar_width)
    self.m_petri_dish_ctrl_v_scrollBar.setGeometry(petriCanvasSize.width()
      -self.m_scroll_bar_width+2,0,self.m_scroll_bar_width,
      petriCanvasSize.height()-self.m_scroll_bar_width+2)



  def setColorLookupFunctor(self, color_lookup_functor):
    self.m_color_lookup_functor = color_lookup_functor

  def createNewCellItem(self, n):
    self.m_occupied_cells_ids.append(n)
    return pyPopulationCellItem(
      self.m_avida.m_population.GetCell(n),
      (n%self.m_world_w) * self.m_map_cell_width,
      (n/self.m_world_w) * self.m_map_cell_width,
      self.m_map_cell_width,
      self.m_map_cell_width,
      self.m_canvas)

  def setAvidaSlot(self, avida):
    descr(avida)
    #print "pyPetriDishCtrl.setAvidaSlot() ..."
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      print "pyPetriDishCtrl.setAvidaSlot() deleting old_avida ..."
      self.m_org_clicked_on_item = None
      self.m_last_cell_outlined = None
      self.m_last_cell_outlined_color = (QPen(Qt.NoPen))

      del old_avida
    if(self.m_avida):
      pass

    if self.m_avida is not None:
      self.m_change_list = self.m_avida.m_avida_threaded_driver.GetChangeList()
      self.m_world_w = cConfig.GetWorldX()
      self.m_world_h = cConfig.GetWorldY()
    else:
      self.m_world_w = 30
      self.m_world_h = 30

    self.m_avida_has_started = False
      
    self.m_initial_target_zoom = int(self.m_target_dish_width / self.m_world_w)
    self.emit(PYSIGNAL("zoomSig"), (self.m_initial_target_zoom,))

    if self.m_canvas: del self.m_canvas
    self.m_canvas = QCanvas(self.m_map_cell_width * self.m_world_w, 
      self.m_map_cell_width * self.m_world_h)
    self.m_canvas.setBackgroundColor(Qt.darkGray)
    self.m_canvas_view.setCanvas(self.m_canvas)

    if self.m_background_rect: del self.m_background_rect
    self.m_background_rect = QCanvasRectangle(
      0, 0,
      self.m_map_cell_width * self.m_world_w,
      self.m_map_cell_width * self.m_world_h,
      self.m_canvas)
    self.m_background_rect.setBrush(QBrush(Qt.black))
    self.m_background_rect.setPen(QPen(Qt.black))
    self.m_background_rect.show()
    self.m_background_rect.setZ(0.0)

    if self.m_cell_info: del self.m_cell_info
    self.m_cell_info = [None] * self.m_world_w * self.m_world_h
    self.m_occupied_cells_ids = []

    if self.m_avida is not None:
      m_founding_cells_dict = self.m_session_mdl.m_founding_cells_dict
      self.m_session_mdl.m_cell_num_ancestor_name_dict = {}
      for k, v in m_founding_cells_dict.iteritems():
        cell_info_item = self.updateCellItem(int(k))
        self.m_session_mdl.m_cell_num_ancestor_name_dict[k] = \
          self.m_session_mdl.m_ancestors_dict[v]
        cell_info_item.setPen(QPen(QColor(Qt.gray)))
      descr("BDB" + str(self.m_session_mdl.m_cell_num_ancestor_name_dict))

    self.m_thread_work_cell_item_index = 0
    self.m_cs_min_value = 0
    self.m_cs_value_range = 0
    self.m_changed_cell_items = self.m_cell_info[:]
    self.updateCellItems(True)

  def setDragSlot(self, org_clicked_on_item = None):
    descr(org_clicked_on_item)
    if org_clicked_on_item:
      clicked_cell_num = org_clicked_on_item.m_population_cell.GetID()
      clicked_cell = self.m_avida.m_population.GetCell(int(clicked_cell_num))
      organism = clicked_cell.GetOrganism()

      #if the organism is not an empty org (e.g. ancestor dragged in that has
      #not been run yet)
      if organism is not None:
        # tee up drag information
        dragHolder = self.itemDrag( ('organism.' + str(organism.GetGenome().AsString())), self )
        dragHolder.dragCopy()

  def setRange(self, min, max):
    self.m_cs_min_value = min
    self.m_cs_value_range = max - min

  def setIndexer(self, indexer):
    self.m_indexer = indexer

  def updateOrgClickedOutlineCellNumberSlot(self, org_clicked_on_item = None):
    if self.m_org_clicked_on_item:
      self.m_org_clicked_on_item.setPen(self.m_last_cell_outlined_color)
    self.m_org_clicked_on_item = org_clicked_on_item
    if self.m_org_clicked_on_item:
      self.m_last_cell_outlined_color = self.m_org_clicked_on_item.pen()
      self.updateCellItems(True)
      self.m_last_m_org_clicked_on_item = self.m_org_clicked_on_item
    else:
      if hasattr(self,"m_last_cell_outlined"):
        if self.m_last_cell_outlined is not None:
            self.m_last_cell_outlined.setPen(self.m_last_cell_outlined_color)
#            self.updateCellItems(self.m_last_m_org_clicked_on_item.m_population_cell.GetID())
            self.updateCellItems(True)

  def updateCellItem(self, cell_id):
    if self.m_cell_info[cell_id] is None:
      self.m_cell_info[cell_id] = self.createNewCellItem(cell_id)
    cell_info_item = self.m_cell_info[cell_id]
    self.m_indexer(cell_info_item, self.m_cs_min_value, self.m_cs_value_range)
    #the following could be moved to a one off function at the beginning of a run
    #for speed efficiency. currenly it is checked every time a cell is updated
    if ( (cell_info_item.pen().color() == QColor((Qt.gray))) & self.m_avida_has_started == True):
      cell_info_item.setPen(QPen(Qt.NoPen))
      
    #if it is not the pre-run outline color (gray) or the org_clicked_on highlight
    #color (bright green) then take away the outline (it will be added back later
    #in this section if it is a cell that does a task)  
    if not (cell_info_item.pen().color() == QColor(0,255,0)\
        or cell_info_item.pen().color() == QColor(Qt.gray)):
      cell_info_item.setPen(QPen(Qt.NoPen))

    if(self.m_avida.m_population.GetCell(cell_id).GetOrganism() is not None):
      passedATest = False      #refers to tests regarding whether a task button is on and that task is performed
      failedATest = False      #refers to tests regarding whether a task button is on and that task is performed
      for i in range(len(self.m_stat_task_button_states)):
        if (self.m_stat_task_button_states[i]):
#          print "IN THE STATE LOOP, WITH I = ", i
          if(self.m_avida.m_population.GetCell(cell_id).GetOrganism().GetPhenotype().GetCurTaskCount()[i]>=1):
            passedATest = True
          else:
            failedATest = True
          paint = False
      if (passedATest and not failedATest):  
        cell_info_item.setPen(QPen(QColor(0,255,255))) 

    brush_color = cell_info_item.updateColorUsingFunctor(self.m_color_lookup_functor)

    if self.m_org_clicked_on_item:
      if cell_info_item.m_population_cell.GetID == self.m_org_clicked_on_item.m_population_cell.GetID:
        cell_info_item.setPen(QPen(QColor(0,255,0),2))
      else:
#        cell_info_item.setPen(QPen(Qt.NoPen))
        self.m_last_cell_outlined = cell_info_item



    return cell_info_item

  def updateCellItems(self, should_update_all = False):
    #descr(should_update_all)
    if self.m_cell_info:

      #descr("self.m_cell_info:", self.m_cell_info)
      self.m_avida and self.m_avida.m_avida_threaded_driver.m_lock.acquire()
      #descr("self.m_change_list:", self.m_change_list)
      if self.m_change_list:
        #descr("self.m_change_list.GetChangeCount():", self.m_change_list.GetChangeCount())
        for index in xrange(self.m_change_list.GetChangeCount()):
          self.updateCellItem(self.m_change_list[index])
        self.m_change_list.Reset()
      self.m_avida and self.m_avida.m_avida_threaded_driver.m_lock.release()

      if should_update_all:
        for cell_id in self.m_occupied_cells_ids:
          self.updateCellItem(cell_id)

      if self.m_canvas: self.m_canvas.update()

  def extractPopulationSlot(self, send_reset_signal = False, send_quit_signal = False):

    # If there is an active Avida object find all the cells that are occupied
    # and place them in a dictionary.  Fire off the signal for the next freezer
    # phase with that signal.

    population_dict = {}
    ancestor_dict = {}
    if self.m_avida != None:
      for x in range(self.m_world_w):
        for y in range(self.m_world_h):
          cell = self.m_avida.m_population.GetCell(x + self.m_world_w*y)
          if cell.IsOccupied() == True:
            organism = cell.GetOrganism()
            genome = organism.GetGenome()
            population_dict[cell.GetID()] = str(genome.AsString())
            ancestor_dict[cell.GetID()] = str(organism.GetLineageLabel())
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("freezeDishPhaseIISig"), 
      (population_dict, ancestor_dict, send_reset_signal, send_quit_signal, ))

  def zoomSlot(self, zoom_factor):
    if hasattr(self,"m_world_h") == False:
       return

    self.m_zoom_factor = zoom_factor
    self.m_world_matrix = QWMatrix()
    if self.m_canvas_view:
      self.m_world_matrix.scale(self.m_zoom_factor/self.m_target_dish_scaling, self.m_zoom_factor/self.m_target_dish_scaling)
      trans_h_preadjust = ((self.m_canvas_view.size().height()-self.m_scroll_bar_width) - (self.m_map_cell_width * self.m_world_h)*
        (self.m_zoom_factor/self.m_target_dish_scaling))/2
      trans_w_preadjust = ((self.m_canvas_view.size().width()-self.m_scroll_bar_width) - (self.m_map_cell_width * self.m_world_w)*
        (self.m_zoom_factor/self.m_target_dish_scaling))/2

      trans_h = trans_h_preadjust + self.m_v_scrollbar_offset*(self.m_zoom_factor/self.m_target_dish_scaling)
      trans_w = trans_w_preadjust + self.m_h_scrollbar_offset*(self.m_zoom_factor/self.m_target_dish_scaling)


      if self.m_zoom_factor == 0:
        self.m_world_matrix.translate(trans_w/(1/self.m_target_dish_scaling),trans_h/(1/self.m_target_dish_scaling))
      else:
        self.m_world_matrix.translate(trans_w/(self.m_zoom_factor/self.m_target_dish_scaling),trans_h/(self.m_zoom_factor/self.m_target_dish_scaling))    
      self.m_canvas_view.setWorldMatrix(self.m_world_matrix)

      black_box = (self.m_map_cell_width * self.m_world_h)*(self.m_zoom_factor/self.m_target_dish_scaling)
      outside = black_box - 371
      if outside < 0: 
        outside = 1

      self.m_setting_scrollbar_values = 1

      self.m_petri_dish_ctrl_h_scrollBar.setMaxValue( 30 + outside/(self.m_zoom_factor/self.m_target_dish_scaling) )
      self.m_petri_dish_ctrl_v_scrollBar.setMaxValue( 30 + outside/(self.m_zoom_factor/self.m_target_dish_scaling) )

      self.m_h_scrollbar_offset = self.m_h_scrollbar_offset*(float(self.m_zoom_factor)/self.m_zoom_old_value) 
      self.m_v_scrollbar_offset = self.m_v_scrollbar_offset*(float(self.m_zoom_factor)/self.m_zoom_old_value) 

      self.m_petri_dish_ctrl_h_scrollBar.setValue( (self.m_petri_dish_ctrl_h_scrollBar.maxValue()/2) - self.m_h_scrollbar_offset)
      self.m_petri_dish_ctrl_v_scrollBar.setValue( (self.m_petri_dish_ctrl_v_scrollBar.maxValue()/2) - self.m_v_scrollbar_offset)

      self.m_zoom_old_value = self.m_zoom_factor

      self.m_setting_scrollbar_values = 0

  def moveCanvasHorizontallySlot(self,horizontal_value):
  
    if horizontal_value < 0:
      return
  
    if self.m_setting_scrollbar_values == 1:
      return

    h_scroll_adjustment_needed = (horizontal_value- (self.m_petri_dish_ctrl_h_scrollBar.maxValue()/2)+ self.m_h_scrollbar_offset)*-1
    self.m_h_scrollbar_offset = h_scroll_adjustment_needed + self.m_h_scrollbar_offset
	
    if self.m_canvas_view:
      self.m_world_matrix.translate(h_scroll_adjustment_needed,0)
      self.m_canvas_view.setWorldMatrix(self.m_world_matrix)

  def moveCanvasVerticallySlot(self,vertical_value):
    if vertical_value < 0:
      return
  
    if self.m_setting_scrollbar_values == 1:
      return

    v_scroll_adjustment_needed = (vertical_value- (self.m_petri_dish_ctrl_v_scrollBar.maxValue()/2)+ self.m_v_scrollbar_offset)*-1
    self.m_v_scrollbar_offset = v_scroll_adjustment_needed + self.m_v_scrollbar_offset

    if self.m_canvas_view:
      self.m_world_matrix.translate(0,v_scroll_adjustment_needed)
      self.m_canvas_view.setWorldMatrix(self.m_world_matrix)


  def hbarScrollNextLineSlot(self):
    if self.m_petri_dish_ctrl_h_scrollBar.value() < self.m_petri_dish_ctrl_h_scrollBar.maxValue():
      self.m_petri_dish_ctrl_h_scrollBar.setValue(self.m_petri_dish_ctrl_h_scrollBar.value()+1)

  def vbarScrollNextLineSlot(self):
    if self.m_petri_dish_ctrl_v_scrollBar.value() < self.m_petri_dish_ctrl_v_scrollBar.maxValue():
      self.m_petri_dish_ctrl_v_scrollBar.setValue(self.m_petri_dish_ctrl_v_scrollBar.value()+1)

  def hbarScrollPrevLineSlot(self):
    if self.m_petri_dish_ctrl_h_scrollBar.value() > self.m_petri_dish_ctrl_h_scrollBar.minValue():
      self.m_petri_dish_ctrl_h_scrollBar.setValue(self.m_petri_dish_ctrl_h_scrollBar.value()-1)

  def vbarScrollPrevLineSlot(self):
    if self.m_petri_dish_ctrl_v_scrollBar.value() > self.m_petri_dish_ctrl_v_scrollBar.minValue():
      self.m_petri_dish_ctrl_v_scrollBar.setValue(self.m_petri_dish_ctrl_v_scrollBar.value()-1)

  class itemDrag(QTextDrag):
    def __init__(self, item_name, parent=None, name=None):
        QStoredDrag.__init__(self, 'item name (QString)', parent, name)
        self.setText(item_name)
        descr(item_name)

