
# Replacement for the standard code for Qt's IconView mainly to allow for the
# dragging of the icon name

from qt import *
from descr import *
import re

imageAncestor_data = [
"24 16 23 1",
"t c #3c553e",
"n c #42a985",
"l c #782424",
"f c #7e9a3e",
"i c #8c7c75",
"d c #914c54",
"q c #a1ca5d",
"o c #a6a8a3",
"r c #b3b5b2",
"s c #b5868b",
"c c #bba4a6",
"m c #c02a30",
"p c #c0bebe",
"a c #cbcccb",
"j c #d7d7d6",
"# c #e64d4a",
"e c #e7c9cc",
"u c #eb3c88",
"b c #efefef",
"k c #f2ddde",
"g c #f4756e",
"h c #f7a798",
". c #ffffff",
"....................#a..",
"..................bcde..",
"................bcdcfb..",
"...........becghcdijca..",
"..........kclliml#dejn..",
"........jeddefoolooi###c",
".......kpdjqaooipjaooole",
".bbgbb.pdiaariosb.barr..",
"biddstm#gdsens#s........",
"jcatoisdsdmgggsk........",
".renaatdooosdsb.........",
".ba.qrl....b............",
".jarsdg.................",
"..roupb.................",
".baae...................",
"..bj...................."
]

imageAncestor = QPixmap(imageAncestor_data)

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
    self.ancestor_name_genome_dict = {}
    self.setVScrollBarMode(QIconView.Auto)
    self.setHScrollBarMode(QIconView.Auto)
    self.setArrangement(QIconView.TopToBottom)


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

  def addGenomeToDict(self, in_key, in_value):
    final_key = str(in_key)

    # check if ancestor name already exist in the list.  If it does append
    # a (#) to the end of it.

    while (self.ancestor_name_genome_dict.has_key(final_key)):
      pattern = re.compile(r"(\()([0-9]+)(\)$)")
      search_result = pattern.search(final_key)
      if (search_result):
        i = int(search_result.group(2)) + 1
        final_key = in_key + "(" + str(i) + ")"
      else:
        final_key = in_key + "(1)"
    self.ancestor_name_genome_dict[final_key] = str(in_value)
    return(final_key)

  def getGenomeFromDict(self, in_key):
    return(self.ancestor_name_genome_dict[str(in_key)])

  def clearGenomeDict(self):
    self.ancestor_name_genome_dict = {}

  def removeGenomeFromDict(self, in_key):
    del self.ancestor_name_genome_dict[in_key]

class pyNewIconViewItem(QIconViewItem):

   def __init__(self, parent_view, organism):
     QIconViewItem.__init__(self, parent_view, organism, imageAncestor)

