# -*- coding: utf-8 -*-

# Generic dialog box with list of radio buttons or checkboxes
# Three required parameters: dialog title, list title, and list of items
# A fourth parameter lets you choose radio buttons (false) or checkboxes (true)
# Example use:
# dialog = pyButtonListDialog("Dialog title", "Choose item", ["item1", "item2"])
# results = dialog.showDialog()
# results will contain the checked buttons

from qt import *


class pyButtonListDialog(QDialog):
  def __init__(self, title, list_title, items, multi = 0, parent = None,
               name = None, modal = 0, fl = 0):
    QDialog.__init__(self,parent,name,modal,fl)

    if title:
      self.title = title
    if list_title:
      self.list_title = list_title

    if not name:
      self.setName("pyButtonListDialog")

    self.setSizeGripEnabled(1)

    pyButtonListDialogLayout = QVBoxLayout(self,11,6,"pyButtonListDialogLayout")

    self.buttonGroup1 = QButtonGroup(self,"buttonGroup1")
    self.buttonGroup1.setColumnLayout(0,Qt.Vertical)
    self.buttonGroup1.layout().setSpacing(6)
    self.buttonGroup1.layout().setMargin(11)
    buttonGroup1Layout = QVBoxLayout(self.buttonGroup1.layout())
    buttonGroup1Layout.setAlignment(Qt.AlignTop)

    self.buttons = []
    cnt = 0
    for item in items:
      if multi:
        self.buttons.append(QCheckBox(self.buttonGroup1, "button%d" % (cnt)))
      else:
        self.buttons.append(QRadioButton(self.buttonGroup1, "button%d" % (cnt)))
      buttonGroup1Layout.addWidget(self.buttons[-1])
      self.buttons[-1].setText(self.__tr(item))
      cnt += 1

    pyButtonListDialogLayout.addWidget(self.buttonGroup1)

    Layout1 = QHBoxLayout(None,0,6,"Layout1")

    self.buttonHelp = QPushButton(self,"buttonHelp")
    self.buttonHelp.setEnabled(0)
    self.buttonHelp.setAutoDefault(1)
    Layout1.addWidget(self.buttonHelp)
    Horizontal_Spacing2 = QSpacerItem(20, 20, QSizePolicy.Expanding,
                                      QSizePolicy.Minimum)
    Layout1.addItem(Horizontal_Spacing2)

    self.buttonOk = QPushButton(self,"buttonOk")
    self.buttonOk.setAutoDefault(1)
    self.buttonOk.setDefault(1)
    Layout1.addWidget(self.buttonOk)

    self.buttonCancel = QPushButton(self,"buttonCancel")
    self.buttonCancel.setAutoDefault(1)
    Layout1.addWidget(self.buttonCancel)
    pyButtonListDialogLayout.addLayout(Layout1)

    self.languageChange()

    self.resize(QSize(312,131).expandedTo(self.minimumSizeHint()))
    self.clearWState(Qt.WState_Polished)

    self.connect(self.buttonOk,SIGNAL("clicked()"),self.accept)
    self.connect(self.buttonCancel,SIGNAL("clicked()"),self.reject)


  def languageChange(self):
    self.setCaption(self.__tr(self.title))
    self.buttonGroup1.setTitle(self.__tr(self.list_title))
    self.buttonHelp.setText(self.__tr("&Help"))
    self.buttonHelp.setAccel(self.__tr("F1"))
    self.buttonOk.setText(self.__tr("&OK"))
    self.buttonOk.setAccel(QString.null)
    self.buttonCancel.setText(self.__tr("&Cancel"))
    self.buttonCancel.setAccel(QString.null)


  def __tr(self,s,c = None):
    return qApp.translate("pyButtonListDialog",s,c)

  def showDialog(self):
    "Show the dialog, returning list of items selected"
    items = []
    self.exec_loop()
    res = self.result()
    if res == 0:
      return []
    else:
      for button in self.buttons:
        if button.isOn():
          items.append(str(button.text()))
    return items
