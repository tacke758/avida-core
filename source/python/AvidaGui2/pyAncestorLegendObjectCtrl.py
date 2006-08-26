# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyOrgSquareView.ui'
#
# Created: Wed Jul 5 10:57:42 2006
#      by: The PyQt User Interface Compiler (pyuic) 3.15.1
#
# WARNING! All changes made in this file will be lost!


from qt import *
from pyOrgSquareCtrl import pyOrgSquareCtrl


class pyAncestorLegendObjectCtrl(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("pyAncestorLegendObjectCtrl")


        self.pyAncestorLegendObjectCtrlLayout = QVBoxLayout(self,0,0,"pyAncestorLegendObjectCtrlLayout")
        self.pyAncestorLegendObjectCtrlLayout.setResizeMode(QLayout.Minimum)
#        pyAncestorLegendObjectCtrlLayout.setResizeMode(QLayout.Preferred)


        self.m_org_square = pyOrgSquareCtrl(self,"m_org_square")
        self.m_org_square.construct()
        self.m_org_square.setMinimumSize(QSize(20,20))
        self.m_org_square.setMaximumSize(QSize(20,20))

        self.m_centered_org_square_box = QHBoxLayout(self,0,0,"m_centered_org_square_box")
        hspacer = QSpacerItem ( 2, 20,QSizePolicy.Expanding)
        hspacer1 = QSpacerItem ( 2, 20,QSizePolicy.Expanding)
        self.m_centered_org_square_box.addItem(hspacer)        
        self.m_centered_org_square_box.addWidget(self.m_org_square)
        self.m_centered_org_square_box.addItem(hspacer1)

#        self.pyAncestorLegendObjectCtrlLayout.addWidget(self.m_org_square)
        self.pyAncestorLegendObjectCtrlLayout.addItem(self.m_centered_org_square_box)
        self.m_org_square.paint(Qt.green)

        self.m_ancestor_name = QLabel(self,"m_ancestor_name")
        self.m_ancestor_name.setMinimumSize(QSize(20,20))
        self.m_ancestor_name.setMaximumHeight(20)
        self.m_ancestor_name.setBackgroundMode(QLabel.PaletteShadow)
        m_ancestor_name_font = QFont(self.m_ancestor_name.font())
        m_ancestor_name_font.setPointSize(12)
        self.m_ancestor_name.setFont(m_ancestor_name_font)
#        self.m_ancestor_name.setScaledContents(0)
        self.m_ancestor_name.setAlignment(QLabel.AlignTop)
#        self.pyAncestorLegendObjectCtrlLayout.addWidget(self.m_ancestor_name)
        self.m_ancestor_name.setPaletteBackgroundColor(Qt.darkGray)
        self.m_ancestor_name.setText(self.__tr("mock"))
#        self.resize(QSize(202,202).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

    def setAncestorName(self,ancestorName):
        self.m_ancestor_name.setText(self.__tr(ancestorName))
        self.m_ancestor_name.setMaximumWidth(self.m_ancestor_name.minimumSizeHint().width())
        self.m_ancestor_name.setMinimumWidth(self.m_ancestor_name.minimumSizeHint().width()+10)
        self.pyAncestorLegendObjectCtrlLayout.addWidget(self.m_ancestor_name)
        self.setMaximumWidth(self.m_ancestor_name.minimumSizeHint().width())

    def setAncestorColor(self,color):
        self.m_org_square.paint(color)

    def languageChange(self):
        self.setCaption(self.__tr("pyPetriDishView"))



    def __tr(self,s,c = None):
        return qApp.translate("pyAncestorLegendObjectCtrl",s,c)
