
# Replacement for the standard code for Qt's IconView mainly to allow for the
# dragging of the icon name

from qt import *
from descr import *

def canDecode( e ):
  
  # use the same mime as standard IconView

  return e.provides( "application/x-qiconlist" ) 

class pyNewIconDrag(QIconDrag):

  def __init__(self, dragSource, name=None):
    QIconDrag.__init__(self, dragSource, name)
    self.ancestor_name = str(name)

  def encodedData(self,mime):
    if ( str(mime).startswith("application/x-qiconlist") ):
      a = self.ancestor_name
    else:
      a = ""
    data = QByteArray(a)
    return data

class pyNewIconView(QIconView):
  def __init__(self, parent, widget_name):
    QIconView.__init__(self, parent, widget_name)
    # self.setAcceptDrops(1)

#   def canDecode(self, e):
#     descr("BDB")
#  
#     # use the same mime as standard IconView
# 
#     return e.provides( "application/x-qiconlist" )

  def dragObject(self):
    item = self.firstItem();
    while(item):
      if (item.isSelected()):
        ds = pyNewIconDrag(self,str(item.text()))
        ds.dragCopy()
        return
      item = item.nextItem()

  def dragEnterEvent( self, e ):

    # Check if you want the drag...
     
    if (canDecode(e)):
      e.accept()

  def contentsDropEvent(self, e):
    if e.source() is self:
      return
    self.emit(PYSIGNAL("DroppedOnNewIconViewSig"),(e,))
