# -*- coding: utf-8 -*-

from Numeric import *
from pyAvidaStatsInterface import pyAvidaStatsInterface
from pyOneAna_GraphView import pyOneAna_GraphView
from pyGraphCtrl import PrintFilter
from pyButtonListDialog import pyButtonListDialog
from qt import *
from qwt import *
import os.path
import heapq

# holds color information
class PriorityColor:
  def __init__(self, name, qt_color, priority):
    self.name = name
    self.qt_color = qt_color
    self.priority = priority

  def __eq__(self, other):
    return (self.name == other.name) and \
           (self.qt_color == other.qt_color) and \
           (self.priority == other.priority)
  def __lt__(self, other):
    return self.priority < other.priority
  def __le__(self, other):
    return self.priority <= other.priority
  def __gt__(self, other):
    return self.priority > other.priority
  def __ge__(self, other):
    return self.priority >= other.priority

# This class deals with the graphing information for populations
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
    self.layout = QHBoxLayout(None, 0, 6, "row%s_layout" % (label))

    self.layout.spacer200_1 = QSpacerItem(23, 16, QSizePolicy.Expanding,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_1)

    self.layout.m_del_button = QPushButton("-", parent.population_box_widget)
    self.layout.addWidget(self.layout.m_del_button)

    self.layout.spacer200_2 = QSpacerItem(20, 16, QSizePolicy.Preferred,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_2)

    self.layout.m_population = QLabel(parent.population_box_widget,
                                      "row%s_m_population" % (label))
    self.layout.m_population.setMinimumWidth(100)
    self.layout.addWidget(self.layout.m_population)

    self.layout.spacer200_3 = QSpacerItem(20, 16, QSizePolicy.Preferred,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_3)

    self.layout.m_combo_box_1_color = QComboBox(0,
                                                parent.population_box_widget,
                                                "row%s_m_combo_box_1_color" % (label))
    self.layout.addWidget(self.layout.m_combo_box_1_color)

    self.layout.spacer200_4 = QSpacerItem(25, 16, QSizePolicy.Expanding,
                                     QSizePolicy.Minimum)
    self.layout.addItem(self.layout.spacer200_4)

    self.layout.m_population.setText(label)

    self.parent = parent

    self.parent.population_box_layout.addLayout(self.layout)
#    parent.population_box.addItem(self.layout)

  def del_population_slot(self):
    "Remove this population's controls from the display"
    self.parent.population_box_layout.removeItem(self.layout)
    if self.color not in self.parent.avail_colors:
      heapq.heappush(self.parent.avail_colors, self.color)
    # close and delete the widgets
    self.layout.deleteAllItems()
    self.layout.m_population.close(True)
    self.layout.m_combo_box_1_color.close(True)
    self.layout.m_del_button.close(True)
    del self.parent.m_combo_boxes[self.label]
    self.parent.modeActivatedSlot()
    self.parent.population_box_widget.resize(
      self.parent.population_box_widget.sizeHint())
    self.parent.resize(self.parent.sizeHint())

  def change_color_slot(self):
    "User selected a new color"
    color = self.parent.m_Colors[self.layout.m_combo_box_1_color.currentItem()]
    self.layout.m_population.setPaletteForegroundColor(color.qt_color)
    if color in self.parent.avail_colors:
      self.parent.avail_colors.remove(color)
      heapq.heapify(self.parent.avail_colors)
    if self.color not in self.parent.avail_colors:
      heapq.heappush(self.parent.avail_colors, self.color)
    self.color = color
    self.parent.modeActivatedSlot()

class pyOneAna_GraphCtrl(pyOneAna_GraphView):

  def __init__(self, parent = None, name = None, fl = 0):
    pyOneAna_GraphView.__init__(self,parent,name,fl)
    self.m_avida_stats_interface = pyAvidaStatsInterface()
    self.m_combo_boxes = {}

  def construct_box(self, widget):
    "Initialize new combo box group with stat information"
    widget.layout.m_combo_box_1_color.clear()
    for color in self.m_Colors:
      widget.layout.m_combo_box_1_color.insertItem(color.name)

    if len(self.avail_colors) > 0:
      widget.color = heapq.heappop(self.avail_colors)
      widget.layout.m_combo_box_1_color.setCurrentItem(widget.color.priority)
    else:
      widget.layout.m_combo_box_1_color.setCurrentItem(0)

    # set the color
    widget.layout.m_population.setPaletteForegroundColor(widget.color.qt_color)

    # connect combo boxes to signal
    self.connect(widget.layout.m_combo_box_1_color, SIGNAL("activated(int)"),
                 widget.change_color_slot)
    self.connect(widget.layout.m_del_button, SIGNAL("clicked()"),
                 widget.del_population_slot)

    print widget.parent.population_box_widget.sizeHint().width()
    print widget.parent.population_box_widget.sizeHint().height()
    
    

    # Show the contents
    widget.layout.m_population.show()
    widget.layout.m_combo_box_1_color.show()
    widget.layout.m_del_button.show()
    widget.parent.population_box_widget.show()
    print widget.parent.population_box_widget.sizeHint().width()
    print widget.parent.population_box_widget.sizeHint().height()
    widget.parent.population_box_widget.resize(widget.parent.population_box_widget.sizeHint())
    self.resize(self.sizeHint())

  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None

    self.population_box.setOrientation(Qt.Vertical)
    self.population_box_widget = QWidget(self.population_box,
                                         "population_box_widget")
    self.population_box_widget.setSizePolicy(
      QSizePolicy(QSizePolicy.Preferred, QSizePolicy.MinimumExpanding))
    self.population_box_widget.setGeometry(QRect(5, 15, 310, 90))
    self.population_box_widget.setMinimumWidth(310)
    self.population_box_widget.setMinimumHeight(90)
    self.population_box_layout = QVBoxLayout(self.population_box_widget, 2, 1,
                                             "population_box_layout")

    self.m_graph_ctrl.construct(self.m_session_mdl)
    self.m_combo_box_1.clear()
    self.m_combo_box_2.clear()
    self.m_combo_box_1.setInsertionPolicy(QComboBox.AtBottom)
    self.m_combo_box_2.setInsertionPolicy(QComboBox.AtBottom)
    self.m_petri_dish_dir_path = ' '
    self.m_petri_dish_dir_exists_flag = False

    self.connect(self.m_combo_box_1, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(self.m_combo_box_2, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(self.m_combo_box_1_style, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(self.m_combo_box_2_style, SIGNAL("activated(int)"),
                 self.modeActivatedSlot)
    self.connect(self.m_session_mdl.m_session_mdtr, 
                 PYSIGNAL("freezerItemDoubleClickedOnInOneAnaSig"),
                 self.freezerItemDoubleClickedOn)

    # set up the combo boxes with plot options
    for entry in self.m_avida_stats_interface.m_entries:
      self.m_combo_box_1.insertItem(entry.name)
    for entry in self.m_avida_stats_interface.m_entries:
      self.m_combo_box_2.insertItem(entry.name)

    # line styles
    self.pen_styles = ['solid', 'thick', 'dotted']
    for style in self.pen_styles:
      self.m_combo_box_1_style.insertItem(style)
      self.m_combo_box_2_style.insertItem(style)

    self.m_combo_box_1_style.setCurrentItem(0)
    self.m_combo_box_2_style.setCurrentItem(1)

    # set up the plot line color options
    self.m_Colors = [PriorityColor('red', Qt.red, 0),
                     PriorityColor('blue', Qt.blue, 1),
                     PriorityColor('green', Qt.green, 2),
                     PriorityColor('cyan', Qt.cyan, 3),
                     PriorityColor('magenta', Qt.magenta, 4),
                     PriorityColor('yellow', Qt.yellow, 5),
                     PriorityColor('dark red', Qt.darkRed, 6),
                     PriorityColor('dark green', Qt.darkGreen, 7),
                     PriorityColor('dark blue', Qt.darkBlue, 8)]

    # available colors
    self.avail_colors = self.m_Colors[:] # shallow copy m_Colors

    self.connect(self.m_session_mdl.m_session_mdtr,
                 PYSIGNAL("freezerItemDroppedInOneAnalyzeSig"),
                 self.petriDropped)
    self.m_graph_ctrl.setAxisTitle(QwtPlot.xBottom, "Time (updates)")
    self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)

    # Start the left with second graph mode -- "Average Fitness"
    self.m_combo_box_1.setCurrentItem(2)

    # Start the right with zeroth mode -- "None"
    self.m_combo_box_2.setCurrentItem(0)

    self.population_box_widget.resize(self.population_box_widget.sizeHint())

  def check_file(self, path):
    "Check for a valid population file"
    if self.m_combo_box_1.currentItem():
      index = self.m_combo_box_1.currentItem()
      stat = self.m_avida_stats_interface.m_entries[index]
      return os.path.isfile(os.path.join(str(path), stat.file))
    else:
      return False

  def graph_row(self, row):
    layout = row.layout
    if self.m_combo_box_1.currentItem() or self.m_combo_box_2.currentItem():

      if self.m_combo_box_1.currentItem():
        index_1 = self.m_combo_box_1.currentItem()
        stat_1 = self.m_avida_stats_interface.m_entries[index_1]
        if self.check_file(row.m_petri_dish_dir_path):
          pass
        else:
          print "error: there is no data file in the directory to load from"
          self.m_graph_ctrl.setTitle(
            self.m_avida_stats_interface.m_entries[0].name)
          self.m_graph_ctrl.setAxisTitle(
            QwtPlot.yLeft, self.m_avida_stats_interface.m_entries[0].name)
          self.m_graph_ctrl.replot()
          return
        self.m_graph_ctrl.setAxisTitle(QwtPlot.yLeft, stat_1.name)
        self.m_graph_ctrl.enableYLeftAxis(True)
        self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yLeft)
        self.m_curve_1_arrays = self.m_avida_stats_interface.load(
            str(row.m_petri_dish_dir_path), stat_1.file, 1, stat_1.index
        )

        row.m_curve_1 = self.m_graph_ctrl.insertCurve(stat_1.name)
        self.m_graph_ctrl.setCurveData(row.m_curve_1, self.m_curve_1_arrays[0],
                                       self.m_curve_1_arrays[1])
        # set the pen
        if self.pen_styles[self.m_combo_box_1_style.currentItem()] is 'thick':
          self.m_graph_ctrl.setCurvePen(row.m_curve_1, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()].qt_color, 3))
        elif self.pen_styles[self.m_combo_box_1_style.currentItem()] is 'dotted':
          self.m_graph_ctrl.setCurvePen(row.m_curve_1, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()].qt_color, 0, Qt.DotLine))
        else:
          self.m_graph_ctrl.setCurvePen(row.m_curve_1, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()].qt_color))
        self.m_graph_ctrl.setCurveYAxis(row.m_curve_1, QwtPlot.yLeft)
        if not self.m_combo_box_2.currentItem():
          self.m_graph_ctrl.enableYRightAxis(False)
          self.m_graph_ctrl.setTitle(stat_1.name)
      else:
        self.m_graph_ctrl.enableYLeftAxis(False)


      if self.m_combo_box_2.currentItem():
        index_2 = self.m_combo_box_2.currentItem()
        stat_2 = self.m_avida_stats_interface.m_entries[index_2]
        self.m_graph_ctrl.setAxisTitle(QwtPlot.yRight, stat_2.name)
        self.m_graph_ctrl.enableYRightAxis(True)
        self.m_graph_ctrl.setAxisAutoScale(QwtPlot.yRight)
        self.m_curve_2_arrays = self.m_avida_stats_interface.load(
            str(row.m_petri_dish_dir_path), stat_2.file, 1, stat_2.index
        )

        row.m_curve_2 = self.m_graph_ctrl.insertCurve(stat_2.name)
        self.m_graph_ctrl.setCurveData(row.m_curve_2, self.m_curve_2_arrays[0], self.m_curve_2_arrays[1])
        if self.pen_styles[self.m_combo_box_2_style.currentItem()] is 'thick':
          self.m_graph_ctrl.setCurvePen(row.m_curve_2, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()].qt_color,3))
        elif self.pen_styles[self.m_combo_box_2_style.currentItem()] is 'dotted':
          self.m_graph_ctrl.setCurvePen(row.m_curve_2, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()].qt_color, 0, Qt.DotLine))
        else:
          self.m_graph_ctrl.setCurvePen(row.m_curve_2, QPen(self.m_Colors[row.layout.m_combo_box_1_color.currentItem()].qt_color))
        self.m_graph_ctrl.setCurveYAxis(row.m_curve_2, QwtPlot.yRight)
        if not self.m_combo_box_1.currentItem():
          self.m_graph_ctrl.setTitle(stat_2.name)


      self.m_graph_ctrl.setAxisAutoScale(QwtPlot.xBottom)

      if self.m_combo_box_1.currentItem() and self.m_combo_box_2.currentItem():
        self.m_graph_ctrl.setTitle(self.m_avida_stats_interface.m_entries[self.m_combo_box_1.currentItem()].name + ' (' + self.pen_styles[self.m_combo_box_1_style.currentItem()] \
        + ') and ' + self.m_avida_stats_interface.m_entries[self.m_combo_box_2.currentItem()].name + ' (' +  self.pen_styles[self.m_combo_box_2_style.currentItem()] +')')
        bounding_rect_1 = self.m_graph_ctrl.curve(row.m_curve_1).boundingRect()
        bounding_rect_2 = self.m_graph_ctrl.curve(row.m_curve_2).boundingRect()
        bounding_rect = bounding_rect_1.unite(bounding_rect_2)
      elif self.m_combo_box_1.currentItem():
        bounding_rect = self.m_graph_ctrl.curve(row.m_curve_1).boundingRect()
      else:
        bounding_rect = self.m_graph_ctrl.curve(row.m_curve_2).boundingRect()

      self.m_graph_ctrl.m_zoomer.setZoomBase(bounding_rect)
  
    else:   # goes with '   if self.m_combo_box_1.currentItem() or row.layout.m_combo_box_2.currentItem():'
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
        if self.check_file(freezer_item_name):
          self.m_combo_boxes[short_name] = pyPopulationGraph(self, short_name)
          if self.m_combo_boxes[short_name]:
            self.construct_box(self.m_combo_boxes[short_name])
            self.m_combo_boxes[short_name].m_petri_dish_dir_path = \
                freezer_item_name
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
