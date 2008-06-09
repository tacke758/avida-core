
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

imageTrashCan_data = \
    "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d" \
    "\x49\x48\x44\x52\x00\x00\x00\x1b\x00\x00\x00\x25" \
    "\x08\x06\x00\x00\x00\xb3\xa4\x92\x88\x00\x00\x03" \
    "\x02\x49\x44\x41\x54\x78\x9c\xbd\x97\x21\x74\x1b" \
    "\x37\x18\xc7\x7f\xdf\xde\xde\x8b\x14\x12\x29\xa4" \
    "\x77\x41\xbe\xa0\x25\x68\x71\x51\x07\xd3\x91\xba" \
    "\x70\xac\x81\x09\x6a\xd8\xde\xd8\x1a\x94\x8e\x35" \
    "\x63\x2e\xca\x15\x6d\xa3\x23\x4b\x60\x69\x59\xca" \
    "\x16\xa3\xd8\xc8\x36\x9a\x2e\x24\xba\x22\x0d\x9c" \
    "\xcf\x69\xec\xbb\x34\x76\x9d\xfc\xc9\xbd\xd3\xe9" \
    "\xe9\xa7\xef\xfb\xfe\xd2\x49\x12\x42\xe0\xa1\xf4" \
    "\xcd\x83\x91\x80\x6f\xef\xda\x51\x44\x2a\x53\x10" \
    "\x42\x90\x85\xc2\x44\x24\xb4\x5a\xc7\xec\xec\xec" \
    "\x72\x75\x05\xcb\xcb\x43\xd2\xf4\x04\xe7\x8e\x10" \
    "\x91\x70\x57\xa0\x7c\x5e\xb3\xba\xd9\xcf\xab\xc9" \
    "\x49\x8c\x23\x13\x91\xb0\xb5\xb5\x41\xa3\x71\xc8" \
    "\xe5\x65\x8f\xdd\xdd\x7d\xbc\x1f\x30\x18\x24\xbc" \
    "\x7a\x85\xbc\x7e\x4d\x88\x22\x10\xe9\xa2\xb5\xe6" \
    "\xf8\x38\xa0\x94\x41\xe4\x2d\x60\xc6\x03\x2e\x2d" \
    "\x19\x7a\xbd\x83\xd1\xb7\x9b\x51\x8f\x23\x5b\x74" \
    "\x54\x55\xd1\x4d\xa5\x71\x7b\xfb\x0d\xcf\x9e\x7d" \
    "\x4f\x9e\x3f\xa5\xd7\xcb\x01\x30\x06\x8c\x51\xe3" \
    "\x7e\x59\xf6\x17\xc6\x3c\x02\x40\xeb\x55\xd2\x34" \
    "\x45\x29\x85\xb5\x16\x00\xa5\x22\x4e\x4f\xf7\xea" \
    "\xd3\x58\xca\xb9\x01\x83\xc1\x0a\x9d\xce\x47\x4e" \
    "\x4e\xb6\x64\x38\x24\x44\x11\xf2\xfc\xf9\xc7\x10" \
    "\x45\x65\x16\xb6\x71\xee\x0f\xac\x5d\xa3\xdb\xed" \
    "\xd2\x6a\xfd\x40\x14\x3d\xe2\xdd\xbb\xbf\x51\x2a" \
    "\x67\xc4\x9c\xd2\x17\xd7\x59\xbf\x5f\x3c\x4f\x4f" \
    "\x9b\x58\xbb\x39\x4a\x4d\x9f\x24\x89\x69\xb7\x53" \
    "\x92\x24\x06\xe0\xe8\xe8\x77\x8c\x31\x75\xc3\xd4" \
    "\xc3\x44\x96\x89\xa2\x13\x9a\xcd\xf7\xa1\xd9\x44" \
    "\x44\x74\x38\x3c\xfc\x07\xe7\xde\x00\x60\xed\x26" \
    "\x69\xfa\x27\x1b\x1b\x09\xed\xf6\x5b\x8c\x59\x21" \
    "\x8e\x63\x3e\x7c\xf8\x0d\x6b\x13\xbc\xf7\xb3\x46" \
    "\xa6\x88\xe3\x32\x1f\xdf\x91\x65\x97\x80\xa2\x70" \
    "\x5e\x36\xee\x65\x6d\x02\x64\xe4\x79\x51\x53\xef" \
    "\x3d\x5a\xeb\x59\x61\x75\xca\x2a\x5b\x95\x2a\x60" \
    "\x75\xa0\x39\x61\x45\x5d\xac\x5d\x43\xa9\x7f\x0b" \
    "\x7c\x76\x6d\xba\xba\x14\xde\x0a\x33\xc6\xb0\xb4" \
    "\x74\x31\xd5\x6e\xed\x32\x00\xce\xf5\xc9\xf3\xcd" \
    "\x51\xdf\xbb\x2d\xd1\x5a\x58\x96\x65\x7c\xfa\xb4" \
    "\x3e\xd5\xee\x5c\xbf\xb2\x7f\x9e\x17\x6b\x52\x6b" \
    "\x3d\x8f\x41\xea\x54\x6d\xef\xb2\x66\x0b\x36\x48" \
    "\x21\x6b\xd7\x6e\xbc\x97\x91\xdd\xa6\x99\x61\x65" \
    "\xcd\xf2\xfc\xfd\xb8\x2d\xcb\xe4\xeb\xdc\x58\x67" \
    "\x90\xeb\x9a\xb5\x00\xf0\xfe\x02\x63\x56\xc6\xdf" \
    "\xe7\x72\x63\x9d\x41\xca\x9a\x79\x5f\x4c\x44\xeb" \
    "\x75\xea\xd6\xde\x9d\x61\xb3\xea\x5e\xdc\x58\xd6" \
    "\x6c\x52\xf7\xe2\xc6\x2f\xad\xb3\xdb\x34\xb3\x41" \
    "\xaa\xb6\x2b\xb8\x8e\x6c\x2e\x58\x9d\x41\x26\xb7" \
    "\x2b\xef\xff\x03\xcc\x8d\x9a\xcd\x0c\xab\xd3\x64" \
    "\x1a\xb5\x5e\x05\xb2\x1b\x35\x5b\x18\xac\x6e\xbb" \
    "\xba\xd7\x1d\x64\x52\x5f\x55\x33\x63\x0c\x83\x81" \
    "\x1b\xbd\x75\x46\xa7\xa9\x7c\x2a\x8d\xce\x9d\xf3" \
    "\x79\xcd\x66\x86\x85\x70\x45\xa7\xf3\x98\xb3\xb3" \
    "\x6d\x39\x3b\x23\x84\xe0\xe5\xe0\xe0\x47\xac\xfd" \
    "\x65\x04\xe8\xb3\xb7\xf7\x13\xe7\xe7\x5d\xf6\xf7" \
    "\x7f\x1e\x1d\x19\xe0\xc9\x93\x5f\xc9\xf3\x61\x2d" \
    "\xac\xf2\xac\xbf\xba\x5a\xfc\x79\x87\x43\x42\xf9" \
    "\x7c\xf1\xe2\x02\xe7\xae\xad\xee\xdc\x0e\x2f\x5f" \
    "\x06\xb2\xec\x12\x63\x02\xde\x5f\xa0\xd4\x5a\xd5" \
    "\x70\xf5\x30\x6b\x63\x42\x68\x10\xc7\x5b\x44\x11" \
    "\x12\x45\x45\xbb\xd6\x3a\x34\x1a\xc9\xb8\x9f\x48" \
    "\x8a\x52\x8f\x69\xb7\xdb\x18\x63\x58\x5f\x7f\x0a" \
    "\x14\xa9\x54\x2a\xaa\x84\xc9\xe4\x65\x70\x51\xc7" \
    "\xf0\xaa\x9b\xcd\x14\xec\x3e\xf5\xa0\x37\xcf\x07" \
    "\x85\xfd\x0f\x4c\xb9\x49\xd8\x7e\xa3\xcf\x88\x00" \
    "\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82"

imageTrashCan = QPixmap()
imageTrashCan.loadFromData(imageTrashCan_data,"PNG")

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


 # def canDecode(self, e):
  #  descr("BDB")
  
     # use the same mime as standard IconView
 
   # return e.provides( "application/x-qiconlist" )

  def dragObject(self):
    item = self.firstItem();
    while(item):
      if (item.isSelected()):
        ds = pyNewIconDrag(self,str(item.text()))
        ds.dragCopy()
        return
      item = item.nextItem()

#  def dragEnterEvent( self, e ):
 
#    descr("BDB")
    # Check if you want the drag...
     
#    if (canDecode(e)):
#      descr("BDB -- canDecode")
#      e.accept()

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

# Create Icon View Item that automatically puts in the Ancestor (double helix)
# image instead of the default file folder icon

class pyNewIconViewItem(QIconViewItem):

   def __init__(self, parent_view, organism):
     QIconViewItem.__init__(self, parent_view, organism, imageAncestor)

# Create Icon View Item that automatically puts in a trash can image instead 
# of the default file folder icon

class TrashIconViewItem(QIconViewItem):

  def __init__(self, parent_view):
    QIconViewItem.__init__(self, parent_view, "", imageTrashCan)
    self.setDropEnabled(True)
    self.moveBy(15,0)

  def contentsDropEvent(self, e):
    if e.source() is self:
      return
    self.QIconView.emit(PYSIGNAL("DroppedOnNewIconViewSig"),(e,))

#   def dragEnterEvent( self, e ):
# 
#     descr("BDB -- TrashIconViewItem")
#     # Check if you want the drag...
# 
#     if (canDecode(e) and not (e.source is self)):
#       descr("BDB -- TrashIconViewItem -- canDecode")
#       e.accept()

