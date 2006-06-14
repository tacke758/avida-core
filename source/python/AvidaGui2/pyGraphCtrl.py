# -*- coding: utf-8 -*-

from qt import *
from qwt import *

class pyGraphCtrl(QwtPlot):

  def __init__(self, *args):
    QwtPlot.__init__(self, *args)
    QToolTip.add(self, "<p><b>To zoom in</b> -- pause run and use mouse to select an area on graph to magnify<br><b>To zoom out</b> -- press ESC key</p>")

  def construct(self, session_mdl):
    self.setCanvasBackground(Qt.white)
    self.m_zoomer = QwtPlotZoomer(
      QwtPlot.xBottom,
      QwtPlot.yLeft,
      QwtPicker.DragSelection,
      QwtPicker.AlwaysOff,
      self.canvas())
    self.m_zoomer.setZoomBase()
    self.m_zoomer.setRubberBandPen(QPen(Qt.black))
    # Configure zoomer for use with one-button mouse -- for OS X.
    # - Click and drag to create a zoom rectangle;
    # - Option-click to zoom-out to full view.
    self.m_zoomer.initMousePattern(1)

  def saveImage(self, filename, type):
    "Save image of graph to file"
    p = QPixmap.grabWidget(self, 0, 0, self.width(), self.height())
    p.save(filename, type, 100)


class PrintFilter(QwtPlotPrintFilter):
  def __init__(self):
    QwtPlotPrintFilter.__init__(self)
  def color(self, c, item, i):
    if not (self.options() & QwtPlotPrintFilter.PrintCanvasBackground):
      if item == QwtPlotPrintFilter.MajorGrid:
        return Qt.darkGray
      elif item == QwtPlotPrintFilter.MinorGrid:
        return Qt.gray
    if item == QwtPlotPrintFilter.Title:
      return Qt.black
    elif item == QwtPlotPrintFilter.AxisScale:
      return Qt.black
    elif item == QwtPlotPrintFilter.AxisTitle:
      return Qt.black
    return Qt.black
  def font(self, f, item, i):
    result = QFont(f)
    result.setPointSize(int(f.pointSize()*1.25))
    return result
