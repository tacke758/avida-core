
# Replacement for the standard code for Qt's IconView mainly to allow for the
# dragging of the icon name

from qt import *
from descr import *

def canDecode( e ):
  descr("BDB")
  
  # use the same mime as standard IconView

  return e.provides( "application/x-qiconlist" ) 

class pyNewIconDrag(QIconDrag):

  def __init__(self, dragSource, name=None):
    QIconDrag.__init__(self, dragSource, name)
    self.ancestor_name = str(name)
    descr("BDB")

  def encodedData(self,mime):
    descr("BDB")
    if ( str(mime).startswith("application/x-qiconlist") ):
      a = self.ancestor_name
    else:
      a = ""
    data = QByteArray(a)
    return data

class pyNewIconView(QIconView):
  def __init__(self, parent, widget_name):
    descr("BDB")
    QIconView.__init__(self, parent, widget_name)
    # self.setAcceptDrops(1)

#   def canDecode(self, e):
#     descr("BDB")
#  
#     # use the same mime as standard IconView
# 
#     return e.provides( "application/x-qiconlist" )

  def dragObject(self):
    descr("BDB")
    item = self.firstItem();
    while(not item.isSelected):
      item = item.nextItem
    ds = pyNewIconDrag(self,str(item.text()))
    ds.dragCopy()

  def dragEnterEvent( self, e ):

    # Check if you want the drag...
     
    descr("BDB -- NewIconView.dragEnterEvent")
    descr(e)
    descr("BDB -- NewIconView.dragEnterEvent")
    if (canDecode(e)):
      descr("BDB -- can decode drag")
      e.accept()



