# -*- coding: utf-8 -*-

from qt import *
from pyNavBarView import *


class pyNavBarCtrl(pyNavBarView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyNavBarView.__init__(self,parent,name,fl)
    self.setAcceptDrops( True )

  def construct(self, session_mdl):
    self.m_list_view.setSelectionMode(QListView.Single)
    self.m_list_view.setSortColumn(-1)
    self.m_list_view.clear()

    self.m_one_analyze_cli = QListViewItem(self.m_list_view, "Analysis")
    self.m_one_organism_cli = QListViewItem(self.m_list_view, "Organism")
    self.m_one_population_cli = QListViewItem(self.m_list_view, "Population")

    self.m_one_population_cli.setPixmap(0, self.image0)
    self.m_one_organism_cli.setPixmap(0, self.image1)
    self.m_one_analyze_cli.setPixmap(0, self.image2)

