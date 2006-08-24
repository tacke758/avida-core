# -*- coding: utf-8 -*-

from qt import *
from pyOneAna_PetriDishView import pyOneAna_PetriDishView
import os

class pyOneAna_PetriDishCtrl(pyOneAna_PetriDishView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOneAna_PetriDishView.__init__(self,parent,name,fl)
    self.m_one_ana_pop_name.setText('Drag A Full Petri Dish Into This Window To Analyze It')

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl     
    # XXX temporarily disabled nonfunctioning gui element, reenable in
    # future when it works. @kgn
    self.m_ana_petri_dish_ctrl.hide()
    self.m_ana_gradient_scale_ctrl.hide()
    self.m_ana_live_controls_ctrl.hide()
#    self.connect( self.m_session_mdl.m_session_mdtr, 
#      PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"),
#      self.freezerItemDropped)  
#    self.connect( self.m_session_mdl.m_session_mdtr, 
#      PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"),
#      self.freezerItemDoubleClickedOn)  


  def freezerItemDropped(self, e):
    descr("BDB")
    # Try to decode to the data you understand...
    freezer_item_name = QString()
    if ( QTextDrag.decode( e, freezer_item_name ) ) :
      freezer_item_name = str(e.encodedData("text/plain"))
      short_name = os.path.splitext((os.path.split(freezer_item_name)[1]))[0]
      self.m_one_ana_pop_name.setText(short_name)

  def freezerItemDoubleClickedOn(self, freezer_item_name):
    descr("BDB")
    short_name = os.path.split(os.path.splitext(os.path.split(freezer_item_name)[0])[0])[1]
    self.m_one_ana_pop_name.setText(short_name)

