from descr import descr
from qt import *

class pyBufferView(QWidget):
  def __init__(self,parent = None,name = None,fl = 0):
    QWidget.__init__(self,parent,name,fl)
    if not name: self.setName("pyBufferView")

    self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

    self.indicator_h = self.indicator_w = self.indicator_bit_count = 0
    self.old_bits = -1
    self.backing_store = QPixmap()

    self.setIndicatorHeight(5)
    self.setIndicatorWidth(5)
    self.setIndicatorBitCount(32)
    self.setBits(0)

  def _updateSize(self):
    size = QSize(self.getIndicatorWidth() * self.getIndicatorBitCount(), self.getIndicatorHeight())
    self.setMinimumSize(size)
    self.setMaximumSize(size)
    self.backing_store.resize(size)

  def getIndicatorHeight(self):
    return self.indicator_h
  def getIndicatorWidth(self):
    return self.indicator_w
  def getIndicatorBitCount(self):
    return self.indicator_bit_count

  def setIndicatorHeight(self, pixels):
    self.indicator_h = pixels
    self._updateSize()
  def setIndicatorWidth(self, pixels):
    self.indicator_w = pixels
    self._updateSize()
  def setIndicatorBitCount(self, count):
    self.indicator_bit_count = count
    self._updateSize()

  def setBit(self, bit_no, bit_val):
    p = QPainter(self.backing_store)
    # fill rect with first color if bit_val is high, otherwise use
    # second color
    p.setBrush(bit_val and Qt.yellow or Qt.blue)
    # alternate background colors, to help count bits visually.
    # do this by outlining odd bit number the fist color, and even the
    # second.
    p.setPen(bit_no & 1 and Qt.lightGray or Qt.darkGray)
    p.drawRect(
      (self.getIndicatorBitCount() - bit_no - 1) * self.getIndicatorWidth(),
      0,
      self.getIndicatorWidth(),
      self.getIndicatorHeight()
    )
    
  def setBits(self, bits):
    if self.old_bits != bits:
      # hi is always 2^(current bit number)
      hi = 1
      for i in xrange(32):
        self.setBit(i, hi & bits)
        hi = hi << 1
      self.old_bits = bits
      self.paint(self.backing_store.rect())
      #descr(self.name(), bits)

  def paint(self, rect):
    p = QPainter(self)
    # update dirty rectangle from backing store.
    p.drawPixmap(rect, self.backing_store)
    
  def paintEvent(self, paint_event):
    # update dirty rectangle from backing store.
    self.paint(paint_event.rect())
