# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'testclient.ui'
#
# Created: Fri Apr 17 18:49:02 2009
#      by: The PyQt User Interface Compiler (pyuic) 3.17.6
#
# WARNING! All changes made in this file will be lost!


from qt import *


class Form1(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("Form1")


        Form1Layout = QVBoxLayout(self,11,6,"Form1Layout")

        layout4 = QHBoxLayout(None,0,6,"layout4")

        layout3 = QVBoxLayout(None,0,6,"layout3")

        self.IPGroupBox = QGroupBox(self,"IPGroupBox")
        self.IPGroupBox.setColumnLayout(0,Qt.Vertical)
        self.IPGroupBox.layout().setSpacing(6)
        self.IPGroupBox.layout().setMargin(11)
        IPGroupBoxLayout = QVBoxLayout(self.IPGroupBox.layout())
        IPGroupBoxLayout.setAlignment(Qt.AlignTop)

        self.IPLabel1 = QLabel(self.IPGroupBox,"IPLabel1")
        IPGroupBoxLayout.addWidget(self.IPLabel1)

        self.IPLineEdit = QLineEdit(self.IPGroupBox,"IPLineEdit")
        IPGroupBoxLayout.addWidget(self.IPLineEdit)

        self.IPPushButton = QPushButton(self.IPGroupBox,"IPPushButton")
        IPGroupBoxLayout.addWidget(self.IPPushButton)
        layout3.addWidget(self.IPGroupBox)

        self.CommandButtonGroup = QButtonGroup(self,"CommandButtonGroup")
        self.CommandButtonGroup.setColumnLayout(0,Qt.Vertical)
        self.CommandButtonGroup.layout().setSpacing(6)
        self.CommandButtonGroup.layout().setMargin(11)
        CommandButtonGroupLayout = QVBoxLayout(self.CommandButtonGroup.layout())
        CommandButtonGroupLayout.setAlignment(Qt.AlignTop)

        self.radioButton1 = QRadioButton(self.CommandButtonGroup,"radioButton1")
        CommandButtonGroupLayout.addWidget(self.radioButton1)

        self.radioButton2 = QRadioButton(self.CommandButtonGroup,"radioButton2")
        CommandButtonGroupLayout.addWidget(self.radioButton2)

        self.radioButton3 = QRadioButton(self.CommandButtonGroup,"radioButton3")
        CommandButtonGroupLayout.addWidget(self.radioButton3)

        self.radioButton4 = QRadioButton(self.CommandButtonGroup,"radioButton4")
        CommandButtonGroupLayout.addWidget(self.radioButton4)

        self.radioButton5 = QRadioButton(self.CommandButtonGroup,"radioButton5")
        CommandButtonGroupLayout.addWidget(self.radioButton5)
        layout3.addWidget(self.CommandButtonGroup)
        layout4.addLayout(layout3)

        self.MsgTextEdit = QTextEdit(self,"MsgTextEdit")
        layout4.addWidget(self.MsgTextEdit)
        Form1Layout.addLayout(layout4)

        layout1 = QHBoxLayout(None,0,6,"layout1")

        self.SendPushButton = QPushButton(self,"SendPushButton")
        layout1.addWidget(self.SendPushButton)

        self.ConsoleTextEdit = QTextEdit(self,"ConsoleTextEdit")
        layout1.addWidget(self.ConsoleTextEdit)
        Form1Layout.addLayout(layout1)

        self.languageChange()

        self.resize(QSize(289,397).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)


    def languageChange(self):
        self.setCaption(self.__tr("Form1"))
        self.IPGroupBox.setTitle(self.__tr("IP Address"))
        self.IPLabel1.setText(self.__tr("Server IP:"))
        self.IPPushButton.setText(self.__tr("Change IP"))
        self.CommandButtonGroup.setTitle(self.__tr("Commands"))
        self.radioButton1.setText(self.__tr("Send Request"))
        self.radioButton2.setText(self.__tr("Join Request"))
        self.radioButton3.setText(self.__tr("Remove Request"))
        self.radioButton4.setText(self.__tr("Submit Request"))
        self.radioButton5.setText(self.__tr("Update Request"))
        self.SendPushButton.setText(self.__tr("Send"))


    def __tr(self,s,c = None):
        return qApp.translate("Form1",s,c)
