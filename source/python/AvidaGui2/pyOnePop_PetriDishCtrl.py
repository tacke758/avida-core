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
    self.m_gradient_scale_is_rescaling = False

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.dishDisabled = False
    self.a = []

    self.m_big_hbox = QHBox(self.m_ancestor_legend_ctrl.viewport())
    self.m_ancestor_legend_ctrl.addChild(self.m_big_hbox)  
        
    self.m_petri_dish_ctrl.construct(self.m_session_mdl)

    self.m_gradient_scale_ctrl.construct(self.m_session_mdl)
    self.m_gradient_scale_view.construct(self.m_session_mdl)

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
       self.setZoomSlot)
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
    self.m_mode_combobox.setCurrentItem(1)
    self.m_mode_index = self.m_mode_combobox.currentItem()
    self.modeActivatedSlot(self.m_mode_index)
    # self.m_petri_dish_ctrl.emit(PYSIGNAL("zoomSig"), 
    #   (self.m_petri_dish_ctrl.m_initial_target_zoom,))


  def setAvidaSlot(self, avida):
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      self.disconnect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if(self.m_avida):
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

    if self.m_avida:
      self.m_map_profile.getUpdater(self.m_mode_combobox.currentItem()).resetRange(self.m_avida.m_population)
      self.modeActivatedSlot(self.m_mode_combobox.currentItem())

    #the setup has changed, remove the old ancestor objects from the scroll view
    for ancestor_object in self.a:
      self.m_big_hbox.removeChild(ancestor_object)

    #now add the new ones
    for label, name in self.m_session_mdl.m_ancestors_dict.iteritems():
      
      an_ancestor_legend_object = pyAncestorLegendObjectCtrl(self.m_big_hbox)
      an_ancestor_legend_object.setAncestorName(name)
      self.a.append(an_ancestor_legend_object)
      an_ancestor_legend_object.show()

    k = 0
    for label, name in self.m_session_mdl.m_ancestors_dict.iteritems():

      lineage_range = len(self.m_session_mdl.m_ancestors_dict)
      non_normalized_index = int(label) + 1
      normalized_index = float(non_normalized_index) / float(lineage_range)
      print "self.m_petri_dish_ctrl is: ", self.m_petri_dish_ctrl
      a_sensible_color = self.m_petri_dish_ctrl.m_color_lookup_functor(normalized_index)

      #the following ugly code is brought to you by the fact that I can't delete objects
      #out of a
      self.a[len(self.a)-len(self.m_session_mdl.m_ancestors_dict)+k].setAncestorColor(a_sensible_color)
      k = k+1
                                    
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
    if (self.m_mode_index==3):
      self.m_map_legend_widget_stack.raiseWidget(1)
    else:
      self.m_map_legend_widget_stack.raiseWidget(0)

    self.m_ancestor_legend_ctrl.setHScrollBarMode(QScrollView.Auto)
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
    if (e.source is self):
      return

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
      
      # Check that this is not an organism from the petri dish acidently
      # dropped onto itself

      if freezer_item_list.startswith("organism."):
        return

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
        info("You can not drag both petri dishes and organisms at the same time")
        return

      # if the user only has organism let the Ancestor box drop method handle
      # the list

      elif list_has_orgs:
        self.m_session_mdl.m_session_mdtr.emit(
          PYSIGNAL("petriDishDroppedAncestorSig"), (e,))
        return

      freezer_item_names = freezer_item_list.split("\t")[1:]
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
      else:
        # pass
        info("You can only drag one dish at a time into this viewer")

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

  def setZoomSlot(self, zoomValue):
    print("BDB, zoom set to " + str(zoomValue))
    self.m_zoom_spinbox.setValue(zoomValue)
    self.m_zoom_spinbox.setFont(QFont("Arial",13))
