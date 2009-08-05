# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyMultiDishView.ui'
#
# Created: Tue May 5 07:46:32 2009
#      by: The PyQt User Interface Compiler (pyuic) 3.17.6
#
# WARNING! All changes made in this file will be lost!


from qt import *
from pyMultiDish_PetriDishCtrl import pyMultiDish_PetriDishCtrl
from pyMultiDish_TimelineCtrl import pyMultiDish_TimelineCtrl
from pyMultiDish_GraphCtrl import pyMultiDish_GraphCtrl
from pyMultiDish_StatsCtrl import pyMultiDish_StatsCtrl

from pyPetriConfigureCtrl import nullFunction

class pyMultiDishView(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        print "pyMultiDishView.__init__"
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("pyMultiDishView")


        pyMultiDishViewLayout = QHBoxLayout(self,11,6,"pyMultiDishViewLayout")

        layout7 = QVBoxLayout(None,0,6,"layout7")

        self.m_multidish_petri_dish_ctrl = pyMultiDish_PetriDishCtrl(self,"m_multidish_petri_dish_ctrl")
        self.m_multidish_petri_dish_ctrl.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.MinimumExpanding,0,0,self.m_multidish_petri_dish_ctrl.sizePolicy().hasHeightForWidth()))
        layout7.addWidget(self.m_multidish_petri_dish_ctrl)

        self.m_multidish_timeline_ctrl = pyMultiDish_TimelineCtrl(self,"m_multidish_timeline_ctrl")
        self.m_multidish_timeline_ctrl.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.MinimumExpanding,0,0,self.m_multidish_timeline_ctrl.sizePolicy().hasHeightForWidth()))
        layout7.addWidget(self.m_multidish_timeline_ctrl)
        pyMultiDishViewLayout.addLayout(layout7)

        layout6 = QVBoxLayout(None,0,6,"layout6")

        self.m_multidish_stats_ctrl = pyMultiDish_StatsCtrl(self,"m_multidish_stats_ctrl")
        self.m_multidish_stats_ctrl.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.Expanding,0,0,self.m_multidish_stats_ctrl.sizePolicy().hasHeightForWidth()))
        layout6.addWidget(self.m_multidish_stats_ctrl)

        self.m_multidish_graph_ctrl = pyMultiDish_GraphCtrl(self,"m_multidish_graph_ctrl")
        self.m_multidish_graph_ctrl.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.MinimumExpanding,0,0,self.m_multidish_graph_ctrl.sizePolicy().hasHeightForWidth()))
        layout6.addWidget(self.m_multidish_graph_ctrl)
        pyMultiDishViewLayout.addLayout(layout6)

        self.languageChange()

        self.resize(QSize(716,649).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)
        
        # from pyMultiDish_TimelineCtrl import nullFunction



    def languageChange(self):
        self.setCaption(self.__tr("pyMultiDishView"))


    def __tr(self,s,c = None):
        return qApp.translate("pyMultiDishView",s,c)
