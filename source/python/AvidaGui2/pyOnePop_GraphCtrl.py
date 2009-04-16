# -*- coding: utf-8 -*-

from Numeric import *
from pyAvidaStatsInterface import pyAvidaStatsInterface
from pyOnePop_GraphView import pyOnePop_GraphView
from pyGraphCtrl import PrintFilter
from qt import *
from Qwt5 import *
import os.path

class pyOnePop_GraphCtrl(pyOnePop_GraphView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOnePop_GraphView.__init__(self,parent,name,fl)
    self.m_avida_stats_interface = pyAvidaStatsInterface()

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)

    self.m_graph_ctrl.construct(self.m_session_mdl)
    self.m_combo_box.clear()
    self.m_combo_box.setInsertionPolicy(QComboBox.AtBottom)
    for entry in self.m_avida_stats_interface.m_entries:
      self.m_combo_box.insertItem(entry.name)
    self.connect(
      self.m_combo_box, SIGNAL("activated(int)"), self.modeActivatedSlot)

    self.m_x_array = zeros(2, Float)
    self.m_y_array = zeros(2, Float)

    self.m_graph_ctrl.setAxisTitle(QwtPlot.xBottom, "Time (updates)")
    self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)

    # Start with second graph mode -- "Average Fitness".
    self.m_combo_box.setCurrentItem(2)
    self.modeActivatedSlot(self.m_combo_box.currentItem())

  def load(self, path, filename, colx, coly):
    if (self.m_avida is None) or (self.m_avida.m_population.GetStats().GetUpdate() == 0):
      self.m_x_array = zeros(1, Float)
      self.m_y_array = zeros(1, Float)
    else:
      self.m_x_array, self.m_y_array = self.m_avida_stats_interface.load(
        path,
        filename, colx, coly)

  def modeActivatedSlot(self, index):
#    self.m_graph_ctrl.setTitle(self.m_avida_stats_interface.m_entries[index].name)
    self.m_graph_title.setText(
      self.m_avida_stats_interface.m_entries[index].name)
    self.m_graph_ctrl.clear()
    if index:
      self.load(
        self.m_session_mdl.m_tempdir_out,
        self.m_avida_stats_interface.m_entries[index].file,
        1,
        self.m_avida_stats_interface.m_entries[index].index
      )
      # Old QWT 4.2
      # self.m_graph_ctrl.m_curve = self.m_graph_ctrl.insertCurve(self.m_avida_stats_interface.m_entries[index].name)
      # self.m_graph_ctrl.setCurveData(self.m_graph_ctrl.m_curve, self.m_x_array, self.m_y_array)
      # self.m_graph_ctrl.setCurvePen(self.m_graph_ctrl.m_curve, QPen(Qt.red))
      # self.m_graph_ctrl.m_zoomer.setZoomBase(self.m_graph_ctrl.curve(self.m_graph_ctrl.m_curve).boundingRect())
      # self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)
      # self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yLeft)

      # New QWT methods
      self.m_graph_ctrl.m_curve = QwtPlotCurve(self.m_avida_stats_interface.m_entries[index].name)
      self.m_graph_ctrl.m_curve.attach(self.m_graph_ctrl)
      self.m_graph_ctrl.m_curve.setData(self.m_x_array, self.m_y_array)
      self.m_graph_ctrl.m_curve.setPen(QPen(Qt.red))
      self.m_graph_ctrl.m_zoomer.setZoomBase(self.m_graph_ctrl.m_curve.boundingRect())
      self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)
      self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yLeft)

    self.m_graph_ctrl.replot()
      
  def setAvidaSlot(self, avida):
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      self.disconnect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if(self.m_avida):
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

  def avidaUpdatedSlot(self):
    # print "... in avidaUpdatedSlot ..."
    if self.m_combo_box.currentItem():
      self.m_x_array = concatenate(
        (self.m_x_array, [self.m_avida.m_population.GetStats().GetUpdate()]),
        1
      )
      self.m_y_array = concatenate(
        (self.m_y_array,
          [self.m_avida_stats_interface.getValue(
            self.m_combo_box.currentItem(),
            self.m_avida.m_population.GetStats()
          )]
        ),
        1
      )
      if hasattr(self.m_graph_ctrl, "m_curve"):
        # print " m_graph_crtl has attribute m_curve\n"
        # self.m_graph_ctrl.setCurveData(self.m_graph_ctrl.m_curve, self.m_x_array, self.m_y_array)
        self.m_graph_ctrl.m_curve.setData(self.m_x_array, self.m_y_array)
        # if self.m_graph_ctrl.m_curve.setData(self.m_x_array, self.m_y_array):
        #  print " setData succeeded\n"
        # else:
        #  print " setData failed\n"
        # Quick hack: Cause the zoomer to limit zooming-out to the
        # boundaries of the displayed curve.
        self.m_graph_ctrl.m_zoomer.setZoomBase(self.m_graph_ctrl.m_curve.boundingRect())
        # Quick hack: If the user has zoomed-in on or -out from the graph,
        # autoscaling will have been disabled. This reenables it.
        self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)
        self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yLeft)
        self.m_graph_ctrl.replot()
      else:
        print " m_graph_crtl DOES NOT have attribute m_curve\n"

  def printGraphSlot(self):
    printer = QPrinter()
    if printer.setup():
      filter = PrintFilter()
      if (QPrinter.GrayScale == printer.colorMode()):
        filter.setOptions(QwtPlotPrintFilter.PrintAll & ~QwtPlotPrintFilter.PrintCanvasBackground)
      self.m_graph_ctrl.printPlot(printer, filter)

  def restart(self):
    self.m_avida = None
    self.m_combo_box.clear()
    self.m_combo_box.setInsertionPolicy(QComboBox.AtBottom)
    for entry in self.m_avida_stats_interface.m_entries:
      self.m_combo_box.insertItem(entry.name)
    self.connect(
      self.m_combo_box, SIGNAL("activated(int)"), self.modeActivatedSlot)

    self.m_x_array = zeros(2, Float)
    self.m_y_array = zeros(2, Float)

    self.m_graph_ctrl.setAxisTitle(QwtPlot.xBottom, "Time (updates)")
    self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)

    # Start with second graph mode -- "Average Fitness".
    self.m_combo_box.setCurrentItem(2)
    self.modeActivatedSlot(self.m_combo_box.currentItem())

  def exportSlot(self):
    "Export stats to a file"
    if self.m_combo_box.currentItem():
      self.m_avida_stats_interface.export(
        [("", self.m_session_mdl.m_tempdir_out)],
        self.m_session_mdl)
