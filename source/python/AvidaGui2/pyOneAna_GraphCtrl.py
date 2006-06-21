# -*- coding: utf-8 -*-

from Numeric import *
from pyAvidaStatsInterface import pyAvidaStatsInterface
from pyOneAna_GraphView import pyOneAna_GraphView
from pyGraphCtrl import PrintFilter
from pyButtonListDialog import pyButtonListDialog
from qt import *
from qwt import *
import os.path

# This class deals with the graphing information for a single population
# It also holds the widget for control of the graph
class pyPopulationGraph:
  def __init__(self, parent = None, label = None):
    "Add another combo box group for new population"
    if not parent:
      print "No parent for population widget"
      return None

    if not label:
      print "No label for box"
      return None

    self.label = label

    # Setup the combo boxes that control the graph
    self.layout = QHBoxLayout(None, 0, 6, "row%s_layout" %(label))

    self.layout.spacer200_1 = QSpacerItem(23, 16, QSizePolicy.Preferred,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_1)

    self.layout.m_population = QLabel(parent, "row%s_m_population" % (label))
    self.layout.m_population.setMinimumWidth(100)
    self.layout.addWidget(self.layout.m_population)
    
    self.layout.m_combo_box_1 = QComboBox(0, parent, "row%s_m_combo_box_1" % (label))
    self.layout.m_combo_box_1_font = QFont(self.layout.m_combo_box_1.font())
    self.layout.m_combo_box_1_font.setPointSize(11)
    self.layout.m_combo_box_1.setFont(self.layout.m_combo_box_1_font)
    self.layout.addWidget(self.layout.m_combo_box_1)

    self.layout.m_combo_box_1_color = QComboBox(0, parent,
                                           "row%s_m_combo_box_1_color" %(label))
    self.layout.addWidget(self.layout.m_combo_box_1_color)
    self.layout.spacer200_2 = QSpacerItem(40, 16, QSizePolicy.Expanding,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_2)

    self.layout.m_combo_box_2 = QComboBox(0, parent,
                                          "row%s_m_combo_box_2" % (label))
    self.layout.m_combo_box_2_font = QFont(self.layout.m_combo_box_2.font())
    self.layout.m_combo_box_2_font.setPointSize(11)
    self.layout.m_combo_box_2.setFont(self.layout.m_combo_box_2_font)
    self.layout.addWidget(self.layout.m_combo_box_2)
    
    self.layout.m_combo_box_2_color = QComboBox(0, parent,
                                           "row%s_m_combo_box_2_color" %(label))
    self.layout.addWidget(self.layout.m_combo_box_2_color)

    self.layout.spacer200_3 = QSpacerItem(40, 16, QSizePolicy.Expanding,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_3)

    self.layout.m_del_button = QPushButton("-", parent)
    self.layout.addWidget(self.layout.m_del_button)

    self.layout.spacer200_4 = QSpacerItem(75, 16, QSizePolicy.Expanding,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_4)

    self.layout.m_population.setText(label)

    self.parent = parent

    parent.layout().addItem(self.layout)

  def del_population_slot(self):
    "Remove this population's controls from the display"
    self.parent.layout().removeItem(self.layout)
    # hide the widgets
    self.layout.m_population.hide()
    self.layout.m_combo_box_1.hide()
    self.layout.m_combo_box_1_color.hide()
    self.layout.m_combo_box_2.hide()
    self.layout.m_combo_box_2_color.hide()
    self.layout.m_del_button.hide()
    del self.parent.m_combo_boxes[self.label]

    self.parent.modeActivatedSlot()

class pyOneAna_GraphCtrl(pyOneAna_GraphView):

  def __init__(self, parent = None, name = None, fl = 0):
    pyOneAna_GraphView.__init__(self,parent,name,fl)
    self.m_avida_stats_interface = pyAvidaStatsInterface()
    self.m_combo_boxes = {}


  def construct_box(self, widget):
    "Initialize new combo box group with stat information"
    widget.layout.m_combo_box_1.clear()
    widget.layout.m_combo_box_2.clear()
    widget.layout.m_combo_box_1.setInsertionPolicy(QComboBox.AtBottom)
    widget.layout.m_combo_box_2.setInsertionPolicy(QComboBox.AtBottom)

    # set up the combo boxes with plot options
    for entry in self.m_avida_stats_interface.m_entries:
      widget.layout.m_combo_box_1.insertItem(entry.name)
    for entry in self.m_avida_stats_interface.m_entries:
      widget.layout.m_combo_box_2.insertItem(entry.name)

    widget.layout.m_combo_box_1_color.clear()
    widget.layout.m_combo_box_2_color.clear()
    for color in self.m_Colors:
      widget.layout.m_combo_box_1_color.insertItem(color[0])
      widget.layout.m_combo_box_2_color.insertItem(color[0])

    # connect combo boxes to signal
    self.connect(widget.layout.m_combo_box_1, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(widget.layout.m_combo_box_2, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(widget.layout.m_combo_box_1_color, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(widget.layout.m_combo_box_2_color, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(widget.layout.m_del_button, SIGNAL("clicked()"),
                 widget.del_population_slot)

    # Start the left with second graph mode -- "Average Fitness"
    widget.layout.m_combo_box_1.setCurrentItem(2)

    # Start the right with zeroth mode -- "None"
    widget.layout.m_combo_box_2.setCurrentItem(0)

    # Start the left with default of red
    widget.layout.m_combo_box_1_color.setCurrentItem(0)
    widget.layout.m_combo_box_2_color.setCurrentItem(2)

    self.resize(self.sizeHint())

    # Show the contents
    widget.layout.m_population.show()
    widget.layout.m_combo_box_1.show()
    widget.layout.m_combo_box_1_color.show()
    widget.layout.m_combo_box_2.show()
    widget.layout.m_combo_box_2_color.show()
    widget.layout.m_del_button.show()

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None

    self.m_graph_ctrl.construct(self.m_session_mdl)
    self.m_petri_dish_dir_path = ' '
    self.m_petri_dish_dir_exists_flag = False

    self.connect( self.m_session_mdl.m_session_mdtr, 
      PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"),
      self.freezerItemDoubleClickedOn)

    # set up the plot line color options
    self.m_Red = ['red', Qt.red]
    self.m_Blue = ['blue', Qt.blue]
    self.m_Green = ['green', Qt.green]
    self.m_ThickBlue = ['thick', Qt.blue]
    self.m_Colors = [self.m_Red, self.m_Blue, self.m_Green, self.m_ThickBlue]
    
    self.connect(self.m_session_mdl.m_session_mdtr,
                 PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"),
                 self.petriDropped)
    self.m_graph_ctrl.setAxisTitle(QwtPlot.xBottom, "Time (updates)")
    self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)

  def graph_row(self, row):
    layout = row.layout
    if row.layout.m_combo_box_1.currentItem() or row.layout.m_combo_box_2.currentItem():

      if row.layout.m_combo_box_1.currentItem():
        index_1 = row.layout.m_combo_box_1.currentItem()

        #check to see if the file exists
        if os.path.isfile(os.path.join(
            str(row.m_petri_dish_dir_path),
            self.m_avida_stats_interface.m_entries[index_1].file)):
          pass
        else:
          print "error: there is no data file in the directory to load from"
          self.m_graph_ctrl.setTitle(
            self.m_avida_stats_interface.m_entries[0].name)
          self.m_graph_ctrl.setAxisTitle(
            QwtPlot.yLeft, self.m_avida_stats_interface.m_entries[0].name)
          self.m_graph_ctrl.replot()
          return
        self.m_graph_ctrl.setAxisTitle(
          QwtPlot.yLeft, self.m_avida_stats_interface.m_entries[index_1].name)
        self.m_graph_ctrl.enableYLeftAxis(True)
        self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yLeft)
        self.m_curve_1_arrays = self.m_avida_stats_interface.load(
            str(row.m_petri_dish_dir_path),
            self.m_avida_stats_interface.m_entries[index_1].file,
            1,
            self.m_avida_stats_interface.m_entries[index_1].index
        )

        row.m_curve_1 = self.m_graph_ctrl.insertCurve(self.m_avida_stats_interface.m_entries[index_1].name)
        self.m_graph_ctrl.setCurveData(row.m_curve_1, self.m_curve_1_arrays[0], self.m_curve_1_arrays[1])
        self.m_graph_ctrl.setCurvePen(row.m_curve_1, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()][1]))
        self.m_graph_ctrl.setCurveYAxis(row.m_curve_1, QwtPlot.yLeft)
        if not row.layout.m_combo_box_2.currentItem():
          self.m_graph_ctrl.enableYRightAxis(False)
          self.m_graph_ctrl.setTitle(self.m_avida_stats_interface.m_entries[index_1].name)
      else:
        self.m_graph_ctrl.enableYLeftAxis(False)


      if row.layout.m_combo_box_2.currentItem():
        index_2 = row.layout.m_combo_box_2.currentItem()
        self.m_graph_ctrl.setAxisTitle(QwtPlot.yRight, self.m_avida_stats_interface.m_entries[index_2].name)
        self.m_graph_ctrl.enableYRightAxis(True)      
        self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yRight)
        self.m_curve_2_arrays = self.m_avida_stats_interface.load(
            str(row.m_petri_dish_dir_path),
            self.m_avida_stats_interface.m_entries[index_2].file,
            1,
            self.m_avida_stats_interface.m_entries[index_2].index
        )

        row.m_curve_2 = self.m_graph_ctrl.insertCurve(self.m_avida_stats_interface.m_entries[index_2].name)
        self.m_graph_ctrl.setCurveData(row.m_curve_2, self.m_curve_2_arrays[0], self.m_curve_2_arrays[1])
        if self.m_Colors[row.layout.m_combo_box_2_color.currentItem()][0] is 'thick':
          self.m_graph_ctrl.setCurvePen(row.m_curve_2, QPen(self.m_Colors[row.layout.m_combo_box_2_color.currentItem()][1],3))
        else:
          self.m_graph_ctrl.setCurvePen(row.m_curve_2, QPen(self.m_Colors[row.layout.m_combo_box_2_color.currentItem()][1]))
        self.m_graph_ctrl.setCurveYAxis(row.m_curve_2, QwtPlot.yRight)
        if not row.layout.m_combo_box_1.currentItem():
          self.m_graph_ctrl.setTitle(self.m_avida_stats_interface.m_entries[index_2].name)


      self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)

      if row.layout.m_combo_box_1.currentItem() and row.layout.m_combo_box_2.currentItem():    
        self.m_graph_ctrl.setTitle(self.m_avida_stats_interface.m_entries[row.layout.m_combo_box_1.currentItem()].name + ' (' + self.m_Colors[row.layout.m_combo_box_1_color.currentItem()][0] \
        + ') and ' + self.m_avida_stats_interface.m_entries[row.layout.m_combo_box_2.currentItem()].name+ ' (' +  self.m_Colors[row.layout.m_combo_box_2_color.currentItem()][0] +')')
        bounding_rect_1 = self.m_graph_ctrl.curve(row.m_curve_1).boundingRect()
        bounding_rect_2 = self.m_graph_ctrl.curve(row.m_curve_2).boundingRect()
        bounding_rect = bounding_rect_1.unite(bounding_rect_2)
      elif row.layout.m_combo_box_1.currentItem():
        bounding_rect = self.m_graph_ctrl.curve(row.m_curve_1).boundingRect()
      else:
        bounding_rect = self.m_graph_ctrl.curve(row.m_curve_2).boundingRect()

      self.m_graph_ctrl.m_zoomer.setZoomBase(bounding_rect)
  
    else:   # goes with '   if row.layout.m_combo_box_1.currentItem() or row.layout.m_combo_box_2.currentItem():'
       self.m_graph_ctrl.setTitle(self.m_avida_stats_interface.m_entries[0].name)
       self.m_graph_ctrl.setAxisTitle(QwtPlot.yLeft, self.m_avida_stats_interface.m_entries[0].name)


  def modeActivatedSlot(self, selected = None, index = None):
   
    #check to see if we have a valid directory path to analyze
    if self.m_petri_dish_dir_exists_flag == False:
      return

    if index:
      self.graph_row(self.m_combo_boxes[index])
    else:
      self.m_graph_ctrl.clear()
      # loop through the different populations we are analyzing
      for row in self.m_combo_boxes.keys():
        self.graph_row(self.m_combo_boxes[row])

    self.m_graph_ctrl.replot()
      
  def printSlot(self):
    printer = QPrinter()
    if printer.setup():
      filter = PrintFilter()
      if (QPrinter.GrayScale == printer.colorMode()):
        filter.setOptions(QwtPlotPrintFilter.PrintAll & ~QwtPlotPrintFilter.PrintCanvasBackground)
      self.m_graph_ctrl.printPlot(printer, filter)

  def exportSlot(self):
    "Export analysis data to a file"
    if len(self.m_combo_boxes) > 0:
      # Choose populations to export
      dialog_caption = "Export Analysis"
      dialog = pyButtonListDialog(dialog_caption,
                                  "Choose populations to export",
                                  self.m_combo_boxes.keys(), True)
      [button.setOn(True) for button in dialog.buttons]
      res = dialog.showDialog()
      if res == []:
        return
      paths = [(short_name, str(self.m_combo_boxes[short_name].m_petri_dish_dir_path)) for short_name in res]
      self.m_avida_stats_interface.export(paths)


  def petriDropped(self, e): 
      # a check in pyOneAnalyzeCtrl.py makes sure this is a valid path
      self.m_petri_dish_dir_exists_flag = True
      # Try to decode to the data you understand...
      freezer_item_name = QString()
      if ( QTextDrag.decode( e, freezer_item_name ) ) :
        freezer_item_name = str(e.encodedData("text/plain"))
        short_name = os.path.splitext(os.path.split(freezer_item_name)[1])[0]
        if short_name in self.m_combo_boxes:
          print "Already being graphed"
          return
        self.m_combo_boxes[short_name] = pyPopulationGraph(self, short_name)
        if self.m_combo_boxes[short_name]:
          self.construct_box(self.m_combo_boxes[short_name])
          self.m_combo_boxes[short_name].m_petri_dish_dir_path = freezer_item_name
          self.modeActivatedSlot(None, short_name)
        return

      pm = QPixmap()
      if ( QImageDrag.decode( e, pm ) ) :
        print "it was a pixmap"
        return

      # QStrList strings
      #strings = QStrList()
      strings = []
      if ( QUriDrag.decode( e, strings ) ) :
        print "it was a uri"
        m = QString("Full URLs:\n")
        for u in strings:
            m = m + "   " + u + '\n'
        # QStringList files
        files = []
        if ( QUriDrag.decodeLocalFiles( e, files ) ) :
            print "it was a file"
            m += "Files:\n"
            # for (QStringList.Iterator i=files.begin() i!=files.end() ++i)
            for i in files:
                m = m + "   " + i + '\n'
        return

      decode_str = decode( e ) 
      if decode_str:
        print " in if decode_str"

  def freezerItemDoubleClickedOn(self, freezer_item_name): 
    # a check in pyOneAnalyzeCtrl.py makes sure this is a valid path
    self.m_petri_dish_dir_exists_flag = True
    short_name = os.path.split(os.path.splitext(os.path.split(freezer_item_name)[0])[0])[1]

    if short_name in self.m_combo_boxes:
      print "Already being graphed"
      return

    self.m_combo_boxes[short_name] = pyPopulationGraph(self, short_name)
    if self.m_combo_boxes[short_name]:
      self.construct_box(self.m_combo_boxes[short_name])
      self.m_combo_boxes[short_name].m_petri_dish_dir_path = os.path.split(freezer_item_name)[0]
      self.modeActivatedSlot(None, short_name)

  def delPopulationSlot(self):
    print "test"
    return
