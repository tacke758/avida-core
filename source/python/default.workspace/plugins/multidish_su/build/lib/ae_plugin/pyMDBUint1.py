# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyMDBUint1.ui'
#
# Created: Wed May 6 07:11:58 2009
#      by: The PyQt User Interface Compiler (pyuic) 3.17.6
#
# WARNING! All changes made in this file will be lost!


from qt import *


class MDBUForm(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("MDBUForm")


        MDBUFormLayout = QGridLayout(self,1,1,11,6,"MDBUFormLayout")

        layout5 = QVBoxLayout(None,0,6,"layout5")

        self.textEdit1 = QTextEdit(self,"textEdit1")
        layout5.addWidget(self.textEdit1)

        layout4 = QHBoxLayout(None,0,6,"layout4")

        self.avidaRunPushButton = QPushButton(self,"avidaRunPushButton")
        layout4.addWidget(self.avidaRunPushButton)

        self.avidaFreezePushButton = QPushButton(self,"avidaFreezePushButton")
        layout4.addWidget(self.avidaFreezePushButton)
        layout5.addLayout(layout4)

        MDBUFormLayout.addLayout(layout5,0,0)

        self.languageChange()

        self.resize(QSize(624,634).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)


    def languageChange(self):
        self.setCaption(self.__tr("MultiDish BU"))
        self.avidaRunPushButton.setText(self.__tr("Run"))
        self.avidaFreezePushButton.setText(self.__tr("Freeze"))


    def __tr(self,s,c = None):
        return qApp.translate("MDBUForm",s,c)
