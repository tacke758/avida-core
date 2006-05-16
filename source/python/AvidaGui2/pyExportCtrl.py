# Export dialog for exporting stats to Excel
from qt import *
from pyExportView import pyExportView

class pyExportCtrl(pyExportView):
  def __init__(self):
    pyExportView.__init__(self)

#  def accept(self):
#    self.export()
#    pyExportView.accept(self)

  def showDialog(self):
    "Show the dialog, returning list of stats to export"
    items = []
    self.exec_loop()
    res = self.result()
    if res == 0:
      return []
    else:
      # PyQt 3.15 missing QListViewItemIterator
      #for item in QListItemViewIterator(self.listView1):
      i = self.listView1.firstChild()
      if i != None and i.isOn():
        items.append(str(i.text()))
      while i:
        i = i.itemBelow()
        if i != None and i.isOn():
          items.append(str(i.text()))
    return items
