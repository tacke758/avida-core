# -*- coding: utf-8 -*-

from descr import descr

from qt import *
from pyTwoOrganismView import pyTwoOrganismView


class pyTwoOrganismCtrl(pyTwoOrganismView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyTwoOrganismView.__init__(self,parent,name,fl)
  def construct(self, session_mdl):
    """does nothing yet."""
    descr()
  def aboutToBeLowered(self):
    """does nothing yet."""
    descr()
  def aboutToBeRaised(self):
    """does nothing yet."""
    descr()
