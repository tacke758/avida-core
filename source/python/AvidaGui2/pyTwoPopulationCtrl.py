# -*- coding: utf-8 -*-

from descr import descr

from qt import *
from pyTwoPopulationView import pyTwoPopulationView


class pyTwoPopulationCtrl(pyTwoPopulationView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyTwoPopulationView.__init__(self,parent,name,fl)
  def construct(self, session_mdl):
    """does nothing yet."""
    descr()
  def aboutToBeLowered(self):
    """does nothing yet."""
    descr()
  def aboutToBeRaised(self):
    """does nothing yet."""
    descr()
