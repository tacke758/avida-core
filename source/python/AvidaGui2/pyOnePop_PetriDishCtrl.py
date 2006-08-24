# -*- coding: utf-8 -*-

from descr import *

from qt import *
from pyMapProfile import pyMapProfile
from pyOnePop_PetriDishView import pyOnePop_PetriDishView
import os
from pyReadFreezer import pyReadFreezer
from pyGradientScaleView import pyGradientScaleView
from pyAncestorLegendView import pyAncestorLegendView
from pyAncestorLegendObjectCtrl import pyAncestorLegendObjectCtrl
from pyQuitDialogCtrl import pyQuitDialogCtrl
from pyButtonListDialog import pyButtonListDialog
from pyOrgSquareCtrl import pyOrgSquareCtrl

class pyOnePop_PetriDishCtrl(pyOnePop_PetriDishView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOnePop_PetriDishView.__init__(self,parent,name,fl)
    self.m_gradient_scale_view = pyGradientScaleView(self,"m_gradient_scale_ctrl")
#    self.m_ancestor_legend_view = pyAncestorLegendView(self,"m_ancestor_legend_ctrl")
    self.m_gradient_scale_is_rescaling = False

#   deleteme
#    self.m_kgn_sv = QScrollView()
#    self.m_kgn_sv.show()
#    self.m_kgn_hbox = QHBox(self.m_kgn_sv.viewport())
#    self.m_kgn_hbox.show()
#    self.m_kgn_sv.addChild(self.m_kgn_hbox)

#    self.m_kgn_label_list = []
#    label = QLabel("%d" % len(self.m_kgn_label_list), self.m_kgn_hbox)
#    self.m_kgn_label_list.append(label)
#    label.show()

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.dishDisabled = False
    self.m_num_ancestor_objects_added = 0
    self.a = []
    print "_______________________________________________________________________________________________________________________________________________"


    #test ancesltor_legend_object
#    print "VIEWPORT WIDTH IS ", self.m_ancestor_legend_ctrl.viewport().size().width()
#    self.m_ancestor_legend_object_ctrl = pyAncestorLegendObjectCtrl(self.m_ancestor_legend_ctrl.viewport(),)
#    self.m_ancestor_legend_object_ctrl.setAncestorName("foo")

    #a fourth
#    self.m_ancestor_legend_object_ctrl_3 = pyAncestorLegendObjectCtrl(self.m_ancestor_legend_ctrl.viewport(),)
#    self.m_ancestor_legend_object_ctrl_2.setMaximumSize(QSize(100,100))
#    self.m_ancestor_legend_object_ctrl_3.setAncestorName("tiny----------------------------------------->")


#    self.pyAncestorLegendCtrlLayout = QHBoxLayout(self.m_ancestor_legend_ctrl.viewport(),0,0,"pyAncestorLegendCtrlLayout")
#    self.pyAncestorLegendCtrlLayout.addWidget(self.m_ancestor_legend_object_ctrl)


#    pyAncestorLegendCtrlLayout.addWidget(self.m_ancestor_legend_object_ctrl_3)
    self.m_big_hbox = QHBox(self.m_ancestor_legend_ctrl.viewport())
    self.m_ancestor_legend_ctrl.addChild(self.m_big_hbox)  
        
    self.m_petri_dish_ctrl.construct(self.m_session_mdl)

    self.m_gradient_scale_ctrl.construct(self.m_session_mdl)
    self.m_gradient_scale_view.construct(self.m_session_mdl)

#    print "ABOUT TO CONSTRUCT ANCESTOR LEGEND *************************************"
#    self.m_ancestor_legend_ctrl.construct(self.m_session_mdl)
#    self.m_ancestor_legend_view.construct(self.m_session_mdl)

    self.m_live_controls_ctrl.construct(self.m_session_mdl)
    self.m_petri_configure_ctrl.construct(self.m_session_mdl)
    self.connect(self.m_session_mdl.m_session_mdtr,
      PYSIGNAL("freezeDishPhaseISig"),
      self.m_petri_dish_ctrl.extractPopulationSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("setAvidaSig"), self.setAvidaSlot)
    self.connect(self.m_petri_dish_toggle, SIGNAL("clicked()"), 
      self.ToggleDishSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("startNewExperimentSig"), self.SetMapModeAndGraphModeToDefaultsSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("doDefrostDishSig"), self.DefrostSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
       PYSIGNAL("doDisablePetriDishSig"), self.SetDishDisabledSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
       PYSIGNAL("doEnablePetriDishSig"), self.SetDishEnabledSlot)
    self.connect(self.m_zoom_spinbox, SIGNAL("valueChanged(int)"), 
       self.m_petri_dish_ctrl.zoomSlot)
    self.connect(self.m_petri_dish_ctrl, PYSIGNAL("zoomSig"), 
       self.m_zoom_spinbox.setValue)
    self.connect(self.m_mode_combobox, SIGNAL("activated(int)"), 
       self.modeActivatedSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("petriDishDroppedInPopViewSig"), self.petriDropped)  
    self.connect(self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("finishedPetriDishSig"), self.finishedPetriDishSlot)  
    # self.connect( self.m_session_mdl.m_session_mdtr,
    #   PYSIGNAL("petriDishDroppedInPopViewSig"), self.petriDroppedSlot)

    self.m_mode_combobox.clear()
    self.m_mode_combobox.setInsertionPolicy(QComboBox.AtBottom)
    self.m_map_profile = pyMapProfile(self.m_session_mdl)
    for i in range(self.m_map_profile.getSize()):
      self.m_mode_combobox.insertItem(self.m_map_profile.getModeName(i))
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("initializeWithDefaultPetriDishSig"),())

 
    # Start with second map mode -- "Fitness".
    self.m_mode_combobox.setCurrentItem(2)
    self.m_mode_index = self.m_mode_combobox.currentItem()
    self.modeActivatedSlot(self.m_mode_index)
    # self.m_petri_dish_ctrl.emit(PYSIGNAL("zoomSig"), 
    #   (self.m_petri_dish_ctrl.m_initial_target_zoom,))


  def setAvidaSlot(self, avida):
    print "pyOnePop_PetriDishCtrl.setAvidaSlot() ..."
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      print "pyOnePop_PetriDishCtrl.setAvidaSlot() disconnecting old_avida ..."
      self.disconnect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if(self.m_avida):
      print "pyOnePop_PetriDishCtrl.setAvidaSlot() connecting self.m_avida ..."
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

    if self.m_avida:
      self.m_map_profile.getUpdater(self.m_mode_combobox.currentItem()).resetRange(self.m_avida.m_population)
      self.modeActivatedSlot(self.m_mode_combobox.currentItem())
    #a third
#    self.m_ancestor_legend_object_ctrl_2 = pyAncestorLegendObjectCtrl(self.m_ancestor_legend_ctrl.viewport(),)
#    self.m_ancestor_legend_object_ctrl_2.setMaximumSize(QSize(100,100))
#    self.m_ancestor_legend_object_ctrl_2.setAncestorName("a very lengthily described ancestor")
#    self.m_ancestor_legend_object_ctrl_2.m_org_square.paint(Qt.blue)
#    self.m_ancestor_legend_object_ctrl_2.setAncestorColor(Qt.yellow)
#    self.pyAncestorLegendCtrlLayout.addWidget(self.m_ancestor_legend_object_ctrl_2)
      

    num_ancestor_object_counter = 0

    for label, name in self.m_session_mdl.m_ancestors_dict.iteritems():
      print "::::::::::::::::::::::::::::::::: Label is ",label
      print "::::::::::::::::::::::::::::::::: Name is ", name
      if (num_ancestor_object_counter < self.m_num_ancestor_objects_added):
        num_ancestor_object_counter = num_ancestor_object_counter+1
        continue

      alo = pyAncestorLegendObjectCtrl(self.m_big_hbox)
 #     a.append(alo)
#      self.pyAncestorLegendCtrlLayout.addWidget(alo)
#      self.m_ancestor_legend_ctrl.addChild(alo)
#      alo.setMaximumSize(QSize(100,100))

#     in the psuedo working version
      descr("BDB: name = " + str(name))
      alo.setAncestorName(name)
#     end in the pseudo

#      alo.setAncestorName(name)
#      lineage_range = len(self.m_session_mdl.m_ancestors_dict)
#      non_normalized_index = label + 1
#      normalized_index = float(non_normalized_index) / float(lineage_range)
#      a_sensible_color = self.m_petri_dish_ctrl.m_color_lookup_functor(normalized_index)
#      alo.setAncestorColor(a_sensible_color)



#      a.append(pyAncestorLegendObjectCtrl(self.m_ancestor_legend_ctrl.viewport()))
#      self.pyAncestorLegendCtrlLayout.addWidget(a[len(a)-1])
#      a[len(a)-1].setMaximumSize(QSize(100,100))
#      a[len(a)-1].setAncestorName(name)
#      self.m_ancestor_legend_ctrl.repaintContents()
      #blah = QLabel("blah", self.m_big_hbox)
      #blah2= QLabel("blah", self.m_big_hbox)
      #a.append(1)
      #a.append(2)
      self.a.append(alo)
      alo.show()
      self.m_num_ancestor_objects_added = len(self.a)
      
    print "len(a) is ", len(self.a)

    k = 0
    for label, name in self.m_session_mdl.m_ancestors_dict.iteritems():

      lineage_range = len(self.m_session_mdl.m_ancestors_dict)
      non_normalized_index = int(label) + 1
      normalized_index = float(non_normalized_index) / float(lineage_range)
      a_sensible_color = self.m_petri_dish_ctrl.m_color_lookup_functor(normalized_index)
      descr("BDB: k = " + str(k) + " label = " + str(label) + " name = " + name)
      self.a[k].setAncestorColor(a_sensible_color)
      k = k+1
                                    


#      alo = pyAncestorLegendObjectCtrl(self.m_kgn_hbox)
#      alo.setMaximumSize(QSize(100,100))
#      alo.setAncestorName(name)
#      label = QLabel("%d" % len(self.m_kgn_label_list), self.m_kgn_hbox)
#      self.m_kgn_label_list.append(alo)
#      alo.show()

#    for thing in a:
#      label = QLabel("%d" % len(a), self.m_big_hbox)
#      label.show()

#    print "self.m_big_hbox:", self.m_big_hbox
#    for thing in self.m_big_hbox.children():
#      print "thing:", thing
#    print self.m_big_hbox
#      a.append(name)
#    print "list is ", a

#/Users/jclune/Desktop/ajeff/avida/Avida-Ed/appBundles/06-07-10/workspace1.workspace/freezer/blue.organism    print "a[len(a)] is ", a[len(a)-1]

  def ToggleDishSlot (self):
    current_page = self.m_petri_dish_widget_stack.visibleWidget()
    current_page_int = self.m_petri_dish_widget_stack.id(current_page)

    # if the petri dish is visiable flip to env control

    if (current_page_int == 0):
       self.m_petri_dish_widget_stack.raiseWidget(1)
       self.FlipButtonTextLabel.setText("<p align=\"right\">Flip to<br>Petri Dish</p>")
       QToolTip.remove(self.m_petri_dish_toggle)
       QToolTip.add(self.m_petri_dish_toggle,
         "Flip to see the <b><i>Petri Dish</i></b>")

    # else flip the env control to see the petri dish

    else:
       self.m_petri_dish_widget_stack.raiseWidget(0)
       self.FlipButtonTextLabel.setText("<p align=\"right\">Flip to<br>Settings</p>")
       QToolTip.remove(self.m_petri_dish_toggle)
       QToolTip.add(self.m_petri_dish_toggle,
         "Flip to see the <b><i>Environmental Settings</i></b>")
       if (self.dishDisabled == False):
         self.m_session_mdl.m_session_mdtr.emit(
           PYSIGNAL("doInitializeAvidaPhaseISig"),
           (self.m_session_mdl.m_tempdir,))
         
  # if the configuration page is not visable make it so (also adjust labling
  # on flip button correctly
 
  def MakeConfigVisibleSlot (self):
    if self.dishDisabled:
      return
    current_page = self.m_petri_dish_widget_stack.visibleWidget()
    current_page_int = self.m_petri_dish_widget_stack.id(current_page)
    if (current_page_int != 1):
       self.m_petri_dish_widget_stack.raiseWidget(1)
       self.FlipButtonTextLabel.setText("<p align=\"right\">Flip to<br>Petri Dish</p>")
       QToolTip.remove(self.m_petri_dish_toggle)
       QToolTip.add(self.m_petri_dish_toggle,
         "Flip to see the <b><i>Petri Dish</i></b>")
       
  def SetDishDisabledSlot(self):
    self.dishDisabled = True

  def SetDishEnabledSlot(self):
    self.dishDisabled = False

  def modeActivatedSlot(self, index):
    self.m_avida and self.m_avida.m_avida_threaded_driver.m_lock.acquire()
    self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("mapModeChangedSig"), (self.m_map_profile.getModeName(index),))
    self.m_mode_index = index
    if (self.m_mode_index==5):
      self.m_map_legend_widget_stack.raiseWidget(1)
    else:
      self.m_map_legend_widget_stack.raiseWidget(0)

#    self.m_ancestor_legend_ctrl.setHScrollBarMode(QScrollView.AlwaysOn)
    self.m_ancestor_legend_ctrl.setHScrollBarMode(QScrollView.Auto)
#    print "*********************: self.m_ancestor_legend_ctrl.horizontalScrollBar is ", self.m_ancestor_legend_ctrl.horizontalScrollBar
#    self.m_ancestor_legend_ctrl.setHBarGeometry(self.m_ancestor_legend_ctrl.horizontalScrollBar,0,0,100,10)
    print "ancestor label HEIGHT, Width IS---------------->", self.m_ancestor_legend_ctrl.height(),self.m_ancestor_legend_ctrl.width()
    print "m_map_legend_widget_stack HEIGHT IS____________>", self.m_map_legend_widget_stack.height(),self.m_map_legend_widget_stack.width()
      
    self.m_petri_dish_ctrl.setIndexer(self.m_map_profile.getIndexer(self.m_mode_index))
    self.m_petri_dish_ctrl.setColorLookupFunctor(self.m_map_profile.getColorLookup(self.m_mode_index))
    self.m_petri_dish_ctrl.m_should_update_all = True
    self.m_gradient_scale_ctrl.setColorLookup(self.m_map_profile.getColorLookup(self.m_mode_index))
    self.m_updater = self.m_map_profile.getUpdater(self.m_mode_index)
    self.m_updater and self.m_updater.reset(True)

    self.m_avida and self.m_avida.m_avida_threaded_driver.m_lock.release()
    self.avidaUpdatedSlot(True)

  def avidaUpdatedSlot(self, should_update_all = False):
    if self.m_updater:
      (old_min, old_max) = self.m_updater.getRange()
      (min, max) = self.m_avida and self.m_updater.updateRange(self.m_avida.m_population) or (0, 0)
      #descr(self.m_avida)
      if self.m_updater.shouldReset() or ((old_min, old_max) != (min, max)):
        self.m_gradient_scale_ctrl.setRange(min, max)
        self.m_gradient_scale_ctrl.activate(True)
        self.m_petri_dish_ctrl.setRange(min, max)
        if (old_min, old_max) != (min, max):
          self.m_gradient_scale_ctrl.setMapModeSlot(self.m_map_profile.getModeName(self.m_mode_index) + ' (rescaling)')
          self.m_gradient_scale_is_rescaling = True
        # Force subsequent resets until valid range is obtained.
        if ((min, max) != (0, 0)):
          self.m_updater.reset(False)
        should_update_all = True
      elif self.m_gradient_scale_is_rescaling:
        self.m_gradient_scale_ctrl.setMapModeSlot(self.m_map_profile.getModeName(self.m_mode_index))
        self.m_gradient_scale_ctrl.activate(True)
        self.m_gradient_scale_is_rescaling = False
    else:
      self.m_gradient_scale_ctrl.setRange(0, 0)
      self.m_gradient_scale_ctrl.activate(True)
      self.m_petri_dish_ctrl.setRange(0, 0)
    self.m_petri_dish_ctrl.updateCellItems(should_update_all)

    stats = update = None
    if self.m_avida: stats = self.m_avida.m_population.GetStats()
    if stats: update = stats.GetUpdate()
    if update: 
      self.m_update_label.setText("Time (Updates): " + str(QString("%1").arg(update)))

      # BDB -- Pause at certain update hack

      if int(update) == int(self.m_session_mdl.m_update_to_pause):
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doPauseAvidaSig"), ())
    
  def RenameDishSlot(self, dishName):
    descr()
    if (not self.dishDisabled):
      self.PopulationTextLabel.setText(dishName)
    
  def petriDropped(self, e):
    descr("BDB")

    current_page = self.m_petri_dish_widget_stack.visibleWidget()
    current_page_int = self.m_petri_dish_widget_stack.id(current_page)

    # if the petri dish control panel is visiable let it's drop methods
    # do the work

    if (current_page_int == 1):
      self.m_session_mdl.m_session_mdtr.emit(
        PYSIGNAL("petriDishDroppedInPetriConfigSig"), (e,))
      return

    freezer_item_list = QString()
    if ( QTextDrag.decode( e, freezer_item_list ) ) :
      freezer_item_list = str(e.encodedData("text/plain"))

      # Do a quick look at the list and be sure the user is not mixing up
      # petri dish files and organisms

      if freezer_item_list.find('.organism') > -1:
        list_has_orgs = True
      else:
        list_has_orgs = False
      if (freezer_item_list.find('.empty') > -1 or 
          freezer_item_list.find('.full') > -1):
        list_has_dishes = True
      else:
        list_has_dishes = False
      if (list_has_orgs and list_has_dishes):
        warningNoMethodName("You can not drag both petri dishes and organisms at the same time")
        return

      # if the user only has organism let the Ancestor box drop method handle
      # the list

      elif list_has_orgs:
        self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("petriDishDroppedAncestorSig"), (e,))
        return

      freezer_item_names = freezer_item_list.split("\t")[1:]
      descr("BDB -- if decode true" + freezer_item_list)
      if (len(freezer_item_names) == 1):
        freezer_item_name = freezer_item_names[0]

        # Organisms should not be dragged from the freezer to the main 

        if freezer_item_name[-8:] == 'organism':
          return
        elif freezer_item_name[-4:] == 'full':
          freezer_item_name_temp = os.path.join(freezer_item_name, 'petri_dish')
          self.m_session_mdl.new_full_dish = True
        else:
          freezer_item_name_temp = freezer_item_name
          self.m_session_mdl.new_empty_dish = True
        thawed_item = pyReadFreezer(freezer_item_name_temp)
        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doDefrostDishSig"),  
          (os.path.splitext((os.path.split(str(freezer_item_name))[1]))[0], thawed_item,))

  def SetMapModeAndGraphModeToDefaultsSlot(self):
    descr()
    self.m_mode_combobox.setCurrentItem(2)
    self.m_mode_index = self.m_mode_combobox.currentItem()
    self.modeActivatedSlot(self.m_mode_index)
    
  def DefrostSlot(self, dish_name, petri_dict):
      descr()

#    if self.isVisible():

      # If the petri dish is already filled prompt the user if they want to 
      # freeze the existing dish
      
      if self.m_petri_configure_ctrl.DishDisabled:

        self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doPauseAvidaSig"), ())

        # check to see if the user needs/wants to save (recycle the quit dialog
        # widget) the dish

        if (not self.m_session_mdl.saved_full_dish):
          m_check_to_freeze = pyQuitDialogCtrl("Start New Experiment")
          quit_return = m_check_to_freeze.showDialog()
 
          # if user chose not to freeze but only load the new dish

          if quit_return == m_check_to_freeze.QuitFlag:
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doEnablePetriDishSig"), ())

          # if user user chose to freeze the existing population

          elif quit_return == m_check_to_freeze.FreezeQuitFlag:
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("doEnablePetriDishSig"), ())
            self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("freezeDishPhaseISig"), (True, False, ))
  
          # if the user clicked the cancel button
  
          else:
            return

      self.RenameDishSlot(dish_name)
      self.finishedPetriDish = False
      self.m_session_mdl.m_session_mdtr.emit(PYSIGNAL("FillDishSig"), 
        (dish_name, petri_dict, ))
 
      # Be sure that the petri dish is finished being filled before going on

      while (not self.finishedPetriDish):
        pass

      self.m_session_mdl.m_session_mdtr.emit(
       PYSIGNAL("restartPopulationSig"), (self.m_session_mdl, ))


  def finishedPetriDishSlot(self):
    descr()
    self.finishedPetriDish = True

  def restart(self, session_mdl):
    descr()
    descr(session_mdl)
    self.dishDisabled = False

  def getPetriDishPixmap(self):
    "Return QPixmap of petri dish and scale"
    dish_height = self.m_petri_dish_ctrl.m_canvas_view.height()
    # Hide the scrollbars so they aren't painted
    self.m_petri_dish_ctrl.m_petri_dish_ctrl_h_scrollBar.hide()
    self.m_petri_dish_ctrl.m_petri_dish_ctrl_v_scrollBar.hide()
    dish_pix = QPixmap.grabWidget(self.m_petri_dish_ctrl.m_canvas_view, 0, 0,
                                  self.m_petri_dish_ctrl.m_canvas_view.width(),
                                  dish_height)
    self.m_petri_dish_ctrl.m_petri_dish_ctrl_h_scrollBar.show()
    self.m_petri_dish_ctrl.m_petri_dish_ctrl_v_scrollBar.show()
    scale_pix = QPixmap.grabWidget(self.m_gradient_scale_ctrl, 0, 0,
                                   self.m_gradient_scale_ctrl.width(),
                                   self.m_gradient_scale_ctrl.height())
    p = QPixmap(max(self.m_petri_dish_ctrl.m_canvas_view.width(),
                    self.m_gradient_scale_ctrl.width()),
                dish_height + self.m_gradient_scale_ctrl.height())
    painter = QPainter(p)
    painter.drawPixmap(0, 0, dish_pix)
    painter.drawPixmap(0, dish_height, scale_pix)
    painter.end()
    return p

  def printPetriDishSlot(self):
    "Print the petri dish and fitness scale"
    printer = QPrinter()
    if printer.setup():
      painter = QPainter(printer)
      painter.drawPixmap(0, 0, self.getPetriDishPixmap())
      painter.end()
