# -*- coding: utf-8 -*-


from AvidaCore import cHardwareDefs, cHardwareCPUDefs

from qt import *
from qtcanvas import *
import math

class pyCircle:
  def __init__(self):
    self.setNumPoints(1)
    self.setRadius(0.)
    self.setCenterX(0.)
    self.setCenterY(0.)
    self.setOTheta(0.)
  def setNumPoints(self, pts):
    self.m_num_points = pts
    self.setDTheta(2.*math.pi/self.m_num_points)
  def setRadius(self, r):
    self.m_radius = r
  def setCenterX(self, x):
    self.m_center_x = x
  def setCenterY(self, y):
    self.m_center_y = y
  def setDTheta(self, d):
    self.m_d_theta = d
  def setOTheta(self, o):
    self.m_o_theta = o
  def numPoints(self):
    return self.m_num_points
  def radius(self):
    return self.m_radius
  def centerX(self):
    return self.m_center_x
  def centerY(self):
    return self.m_center_y
  def oTheta(self):
    return self.m_o_theta
  def dTheta(self):
    return self.m_d_theta

class pyInstructionPoint:
  def __init__(self):
    self.setHidden(True)
    self.setCircle(None)
    self.setPosition(0)
  def setHidden(self, is_hidden):
    self.m_hidden = is_hidden
  def setCircle(self, circle):
    self.m_circle = circle
  def setPosition(self, position):
    self.m_position = position
  def hidden(self):
    return self.m_hidden
  def circle(self):
    return self.m_circle
  def position(self):
    return self.m_position

def checkDimensions(outer_diameter, n):
  print "checkDimensions(outer_diameter: %d, n: %d)" % (outer_diameter, n)
  sin_n = math.sin(math.pi/n)
  sin_n_3 = 3 * sin_n
  R = (outer_diameter/2.) / (1 + sin_n_3)
  r = R * sin_n

  return R, r

class pyOrganismAnimator:
  def __init__(self):
    print "pyOrganismAnimator.__init__()..."
    self.font = QFont(qApp.font())

    self.label_font = QFont(self.font)
    self.label_font_metrics = QFontMetrics(self.label_font)
    self.label_text_height = 12
    self.label_font_points_per_pixel = self.label_font.pointSizeFloat()/self.label_font_metrics.height()
    self.label_point_size_float = self.label_text_height * self.label_font_points_per_pixel
    self.label_font.setPointSizeFloat(self.label_point_size_float)

    self.default_color_saturation = 100
    self.default_color_value = 248

    self.on_brush = QBrush(Qt.red)
    self.off_brush = QBrush(Qt.black)
    self.on_pen = QPen(Qt.red)
    self.off_pen = QPen(Qt.black)

  def setOps(self, ops = None):
    print "pyOrganismAnimator.setOps()..."
    self.ops_dict = ops

  def setInstNames(self, inst_names = None):
    print "pyOrganismAnimator.setInstNames()..."
    self.inst_names_dict = inst_names

  def setCanvas(self, canvas = None):
    print "pyOrganismAnimator.setCanvas()..."
    self.canvas = canvas
  def setFrames(self, frames = None):
    """
    Preps pyOrganismAnimator for displaying organism movie frames.
    
    frames - sequence of hardware snapshots from organism analysis.
    """
    print "pyOrganismAnimator.setFrames()..."
    self.frames = frames
    if frames is None:
      self.last_copied_instruction_cache = None
    else:
      self.last_copied_instruction_cache = []
      last_copied_instruction = 0
      for i in xrange(frames.getSnapshotCount()):
        ms = frames.getMemorySnapshotAt(i)
        last_copied_instruction = max(
          [ms.GetFlagCopied(j) and j or last_copied_instruction for j in xrange(last_copied_instruction, ms.GetSize())]
        )
        self.last_copied_instruction_cache.append(last_copied_instruction)

    #self.instructions_anim.setFrames(frames)
    #self.heads_anim.setFrames(frames)
    #self.hw_anim.setFrames(frames)
  def setSize(self, width, height):
    """
    Called when canvas dimensions have changed.

    XXX

    This has become too hacky. It was supposed to manage required layout
    changes, but the code that does this has been scattered all over the
    place, and needs reconsolidation. @kgn
    """
    print "pyOrganismAnimator.setSize(width:%d, height:%d)" % (width, height)
    self.canvas_width = width
    self.canvas_height = height

    #layout_dims = self.layout_manager.checkDimensions(width, height)
    #for num in layout_dims:
    #  if num <= 0:
    #    return
    #if layout_dims != self.layout_dims:
    #  self.layout_dims = layout_dims
    #  self.layout_manager.setDimensions(layout_dims)
    #  self.c_x, self.c_y, self.R, self.r, self.rr = layout_dims
    #  text_height = 3 * self.r
    #  font_metrics = QFontMetrics(self.font)
    #  font_points_per_pixel = self.font.pointSizeFloat()/font_metrics.height()
    #  point_size_float = text_height * font_points_per_pixel
    #  self.font.setPointSizeFloat(point_size_float)

  def showFrame(self, frame_number):
    """
    Forwards call to .showFrame() of various subanimators, then updates
    canvas.
    """
    print "pyOrganismAnimator.showFrame(%d)" % frame_number
    #self.instructions_anim.showFrame(frame_number)
    #self.heads_anim.showFrame(frame_number)
    #self.hw_anim.showFrame(frame_number)
    #self.canvas.update()

class pyOrganismScopeView2(QCanvasView):
  def __init__(self,parent = None,name = None,fl = 0):
    print "pyOrganismScopeView2.__init__()..."
    QCanvasView.__init__(self,parent,name,fl)
    if not name: self.setName("pyOrganismScopeView2")

    self.setVScrollBarMode(QScrollView.AlwaysOff)
    self.setHScrollBarMode(QScrollView.AlwaysOff)
    self.m_canvas = QCanvas()
    self.setCanvas(self.m_canvas)

    font = QFont(self.font())
    font_metrics = QFontMetrics(font)
    self.m_font_points_per_pixel = font.pointSizeFloat()/font_metrics.height()

    self.m_organism_circle_margin = 10
    self.m_font_oversize_factor = 1.2

    self.m_visible_width = 1
    self.m_visible_height = 1
    self.m_parent_circle = pyCircle()
    self.m_child_circle = pyCircle()

    self.reset()

  def reset(self):
    print "pyOrganismScopeView2.reset()..."

    if hasattr(self, "m_inst_pts") and self.m_inst_pts is not None:
      for item in self.m_inst_pts:
        item.setCircle(None)
    if hasattr(self, "m_inst_items") and self.m_inst_items is not None:
      for item in self.m_inst_items:
        item.setCanvas(None)
    if hasattr(self, "m_inst_bg_items") and self.m_inst_bg_items is not None:
      for item in self.m_inst_bg_items:
        item.setCanvas(None)
    if hasattr(self, "m_ihead_move_items") and self.m_ihead_move_items is not None:
      for item in self.m_ihead_move_items:
        item.setCanvas(None)
    if hasattr(self, "m_ihead_item") and self.m_ihead_item is not None:
      self.m_ihead_item.setCanvas(None)
    if hasattr(self, "m_rhead_item") and self.m_rhead_item is not None:
      self.m_rhead_item.setCanvas(None)
    if hasattr(self, "m_whead_item") and self.m_whead_item is not None:
      self.m_whead_item.setCanvas(None)
    if hasattr(self, "m_fhead_item") and self.m_fhead_item is not None:
      self.m_fhead_item.setCanvas(None)

    self.m_inst_pts = None
    self.m_inst_items = None
    self.m_inst_bg_items = None
    self.m_ihead_move_items = None
    self.m_ihead_item = None
    self.m_rhead_item = None
    self.m_whead_item = None
    self.m_fhead_item = None
    self.m_task_items = None

    self.m_task_names = None
    self.m_inst_names = None

    self.m_frames = None
    self.m_max_genome_size = 0
    self.m_current_frame_number = 0
    self.m_current_radius = None
    self.m_current_genome = None
    self.m_current_tasks = None
    self.m_current_ihead = None
    self.m_current_rhead = None
    self.m_current_whead = None
    self.m_current_fhead = None
    self.m_current_ihead_move = None

    # Hmm; can't emit gestationTimeChangedSig(0) without causing absurd slider values. @kgn
    self.emit(PYSIGNAL("gestationTimeChangedSig"),(1,))
    self.emit(PYSIGNAL("executionStepResetSig"),(0,))

  def setTaskNames(self, task_names = None):
    print "pyOrganismScopeView2.setTaskNames()..."
    self.m_task_names = task_names

  def setInstNames(self, inst_names = None):
    print "pyOrganismScopeView2.setInstNames()..."
    self.m_inst_names = inst_names

  def setOps(self, ops = None):
    print "pyOrganismScopeView2.setOps()..."
    self.m_ops_dict = ops

  def setFrames(self, frames = None):
    print "pyOrganismScopeView2.setFrames()..."
    self.reset()
    self.m_frames = frames
    if self.m_frames is not None:
      self.last_copied_instruction_cache = []
      last_copied_instruction = 0
      for i in xrange(self.m_frames.getSnapshotCount()):
        ms = self.m_frames.getMemorySnapshotAt(i)
        last_copied_instruction = max(
          [ms.GetFlagCopied(j) and j or last_copied_instruction for j in xrange(last_copied_instruction, ms.GetSize())]
        )
        self.last_copied_instruction_cache.append(last_copied_instruction)

      self.m_parent_size = len(self.m_frames.m_genome_info[0])
      self.m_max_genome_size = max([len(genome) for genome in self.m_frames.m_genome_info])
      self.m_child_size = max(0, last_copied_instruction - self.m_parent_size + 1)
      print "parent_size %d, max_genome_size %d" % (self.m_parent_size, self.m_max_genome_size)

      self.updateParentCircle()

      text_height = 2. * self.m_instruction_spot_radius
      font = QFont(self.font())
      point_size_float = self.m_font_oversize_factor * text_height * self.m_font_points_per_pixel
      font.setPointSizeFloat(point_size_float)

      self.m_inst_pts = [pyInstructionPoint() for i in xrange(self.m_max_genome_size)]
      self.m_inst_items = [QCanvasText(self.m_canvas) for i in xrange(self.m_max_genome_size)]
      self.m_inst_bg_items = [QCanvasEllipse(self.m_canvas) for i in xrange(self.m_max_genome_size)]
      for item in self.m_inst_items:
        item.setTextFlags(Qt.AlignCenter)
        item.setZ(3.)
      for item in self.m_inst_bg_items:
        item.setSize(text_height, text_height)
        item.setZ(2.)

      if self.m_frames.m_ihead_info is not None:
        self.m_ihead_item = QCanvasEllipse(self.m_canvas)
        self.m_ihead_item.setSize(text_height + 6, text_height + 6)
        self.m_ihead_item.setZ(1.)
        self.m_ihead_item.setBrush(QBrush(Qt.blue))

        self.m_ihead_bg_item = QCanvasEllipse(self.m_canvas)
        self.m_ihead_bg_item.setSize(text_height + 6, text_height + 6)
        self.m_ihead_bg_item.setZ(4.)
        self.m_ihead_bg_item.setBrush(QBrush(Qt.white))

        self.m_ihead_text = QCanvasText(self.m_canvas)
        self.m_ihead_text.setFont(font)
        self.m_ihead_text.setColor(Qt.blue)
        self.m_ihead_text.setTextFlags(Qt.AlignCenter)
        self.m_ihead_text.setText("i")
        self.m_ihead_text.setZ(5.)

      if self.m_frames.m_rhead_info is not None:
        self.m_rhead_item = QCanvasEllipse(self.m_canvas)
        self.m_rhead_item.setSize(text_height + 6, text_height + 6)
        self.m_rhead_item.setZ(1.)
        self.m_rhead_item.setBrush(QBrush(Qt.green))

        self.m_rhead_bg_item = QCanvasEllipse(self.m_canvas)
        self.m_rhead_bg_item.setSize(text_height + 6, text_height + 6)
        self.m_rhead_bg_item.setZ(4.)
        self.m_rhead_bg_item.setBrush(QBrush(Qt.white))

        self.m_rhead_text = QCanvasText(self.m_canvas)
        self.m_rhead_text.setFont(font)
        self.m_rhead_text.setColor(Qt.green)
        self.m_rhead_text.setTextFlags(Qt.AlignCenter)
        self.m_rhead_text.setText("r")
        self.m_rhead_text.setZ(5.)

      if self.m_frames.m_whead_info is not None:
        self.m_whead_item = QCanvasEllipse(self.m_canvas)
        self.m_whead_item.setSize(text_height + 6, text_height + 6)
        self.m_whead_item.setZ(1.)
        self.m_whead_item.setBrush(QBrush(Qt.red))

        self.m_whead_bg_item = QCanvasEllipse(self.m_canvas)
        self.m_whead_bg_item.setSize(text_height + 6, text_height + 6)
        self.m_whead_bg_item.setZ(4.)
        self.m_whead_bg_item.setBrush(QBrush(Qt.white))

        self.m_whead_text = QCanvasText(self.m_canvas)
        self.m_whead_text.setFont(font)
        self.m_whead_text.setColor(Qt.red)
        self.m_whead_text.setTextFlags(Qt.AlignCenter)
        self.m_whead_text.setText("w")
        self.m_whead_text.setZ(5.)

      if self.m_frames.m_fhead_info is not None:
        self.m_fhead_item = QCanvasEllipse(self.m_canvas)
        self.m_fhead_item.setSize(text_height + 6, text_height + 6)
        self.m_fhead_item.setZ(1.)
        self.m_fhead_item.setBrush(QBrush(Qt.cyan))

        self.m_fhead_bg_item = QCanvasEllipse(self.m_canvas)
        self.m_fhead_bg_item.setSize(text_height + 6, text_height + 6)
        self.m_fhead_bg_item.setZ(4.)
        self.m_fhead_bg_item.setBrush(QBrush(Qt.white))

        self.m_fhead_text = QCanvasText(self.m_canvas)
        self.m_fhead_text.setFont(font)
        self.m_fhead_text.setColor(Qt.cyan)
        self.m_fhead_text.setTextFlags(Qt.AlignCenter)
        self.m_fhead_text.setText("f")
        self.m_fhead_text.setZ(5.)

      # XXX
      ###if self.m_frames.m_ihead_moves is not None:
      ###  #self.m_ihead_move_items = [QCanvasSpline(self.m_canvas) for i in xrange(len(self.m_frames.m_ihead_moves))]
      ###  self.m_ihead_move_items = [pyHeadPath(self.m_canvas) for i in xrange(len(self.m_frames.m_ihead_moves))]

      if self.m_frames.m_is_viable:
        self.emit(PYSIGNAL("gestationTimeChangedSig"),(self.m_frames.m_gestation_time,))
      else:
        self.emit(PYSIGNAL("gestationTimeChangedSig"),(len(self.m_frames.m_genome_info),))
      self.showFrame(0)

  def viewportResizeEvent(self, resize_event):
    print "pyOrganismScopeView2.viewportResizeEvent()..."
    QCanvasView.viewportResizeEvent(self, resize_event)
    # XXX Fragility: If the top-level Avida-ED window is allowed to change height, this is going to cause a painful slow-down
    # whenever the window height changes.  But having chosen a fixed window height, we're okay for now.
    #
    # @kgn
    if self.m_visible_height != resize_event.size().height() or self.m_visible_width != resize_event.size().width():
      self.m_canvas.resize(resize_event.size().width(), resize_event.size().height())
      self.m_visible_width = resize_event.size().width()
      self.m_visible_height = resize_event.size().height()
      self.updateParentCircle()
      self.showFrame(self.m_current_frame_number)

  def updateParentCircle(self):
    print "pyOrganismScopeView2.updateParentCircle()..."

    if self.m_frames is not None:
      extra_width_factor = float(self.m_parent_size + self.m_child_size)/self.m_parent_size
      print "parent_size %d, child_size %d, factor %d" % (self.m_parent_size, self.m_child_size, extra_width_factor)
      n = max(self.m_parent_size, self.m_child_size)
      bounded_width = self.m_visible_width - 2.*self.m_organism_circle_margin
      bounded_height = self.m_visible_height - 2.*self.m_organism_circle_margin
      outer_diameter = min(bounded_width/extra_width_factor, bounded_height)

      radius, self.m_instruction_spot_radius = checkDimensions(outer_diameter, n)
      self.m_parent_circle.setRadius(radius)
      self.m_parent_circle.setCenterX(outer_diameter/2.)
      self.m_parent_circle.setCenterY(self.m_visible_height/2.)
      self.m_parent_circle.setNumPoints(self.m_parent_size)
      self.m_parent_circle.setOTheta(0.)

      if self.m_inst_items is not None:
        text_height = 2. * self.m_instruction_spot_radius
        font = QFont(self.font())
        point_size_float = self.m_font_oversize_factor * text_height * self.m_font_points_per_pixel
        font.setPointSizeFloat(point_size_float)
        for item in self.m_inst_items:
          item.setFont(font)
        for item in self.m_inst_bg_items:
          item.setSize(point_size_float, point_size_float)

  def updateChildCircle(self, child_size, parent_size, parent_circle_radius, parent_center_x, parent_center_y, instruction_spot_radius):
    print "pyOrganismScopeView2.updateChildCircle()..."
    self.m_child_circle.setRadius(parent_circle_radius * child_size / parent_size)
    self.m_child_circle.setCenterX(parent_center_x + parent_circle_radius + 2*instruction_spot_radius + self.m_child_circle.radius())
    self.m_child_circle.setCenterY(parent_center_y)
    self.m_child_circle.setNumPoints(child_size)
    self.m_child_circle.setOTheta(math.pi)

  def debug_displayHardwareCPUSnapshot(self, frame_number):
    print "pyOrganismScopeView2.debug_displayHardwareCPUSnapshot(%d)..." % frame_number
    hardware_snapshot = self.m_frames.getHardwareSnapshotAt(frame_number)
    memory_snapshot = self.m_frames.getMemorySnapshotAt(frame_number)
    threads_snapshot = self.m_frames.getThreadsSnapshotAt(frame_number)

    print "hardware_snapshot", hardware_snapshot
    print "memory_snapshot", memory_snapshot
    print "threads_snapshot", threads_snapshot
    memory_size = memory_snapshot.GetSize()
    memory_string = str(memory_snapshot.AsString())
    print "memory_snapshot.GetSize()", memory_size
    print "memory:"
    for i in xrange(memory_size):
      print " ", memory_string[i], memory_snapshot.GetFlagCopied(i), memory_snapshot.GetFlagExecuted(i)
    threads_count = len(threads_snapshot)
    print "thread:"
    for i in xrange(threads_count):
      thread = threads_snapshot[i]
      print " .cur_head", thread.cur_head
      print " .cur_stack", thread.cur_stack
      print " .next_label", thread.next_label
      print " .read_label", thread.read_label
      #print " .stack", thread.stack
      for j in xrange(cHardwareDefs.s_STACK_SIZE):
        print " .stack[%d]" % j, thread.stack.Get(j)
      print " .reg[s_REG_AX]", thread.GetRegister(cHardwareCPUDefs.s_REG_AX)
      print " .reg[s_REG_BX]", thread.GetRegister(cHardwareCPUDefs.s_REG_BX)
      print " .reg[s_REG_CX]", thread.GetRegister(cHardwareCPUDefs.s_REG_CX)
      #print " .reg[s_REG_DX]", thread.GetRegister(cHardwareCPUDefs.s_REG_DX)
      print " .heads[s_HEAD_IP]", thread.GetHead(cHardwareDefs.s_HEAD_IP).GetPosition()
      print " .heads[s_HEAD_READ]", thread.GetHead(cHardwareDefs.s_HEAD_READ).GetPosition()
      print " .heads[s_HEAD_WRITE]", thread.GetHead(cHardwareDefs.s_HEAD_WRITE).GetPosition()
      print " .heads[s_HEAD_FLOW]", thread.GetHead(cHardwareDefs.s_HEAD_FLOW).GetPosition()


    #print "ss", hardware_snapshot
    #print "ss.GetMemory()", hardware_snapshot.GetMemory()
    #print "ss.GetMemory().AsString()", hardware_snapshot.GetMemory().AsString()

    # cCPUMemory memory : cCPUMemory & GetMemory()
    # cCPUStack global_stack : cCPUStack pyGetGlobalStack()
    # int thread_time_used : int pyGetThreadTimeUsed()
    ### assume a single thread
    # cHardwareCPU_Thread thread : tArray<cHardwareCPU_Thread> pyGetThreads()
    # bool mal_active : bool GetMalActive()
    # bool advance_ip : bool pyGetAdvanceIP()

  def showFrame(self, frame_number = 0):
    print "pyOrganismScopeView2.showFrame(%d)" % frame_number
    old_frame_number = self.m_current_frame_number
    old_genome = self.m_current_genome

    self.m_current_frame_number = 0
    self.m_current_genome = None

    if self.m_frames is not None and frame_number < self.m_frames.m_gestation_time:
      #self.debug_displayHardwareCPUSnapshot(frame_number)
      self.m_current_frame_number = frame_number
      self.m_current_genome = self.m_frames.m_genome_info[self.m_current_frame_number]
      displayed_genome_size = max(self.last_copied_instruction_cache[self.m_current_frame_number] + 1, self.m_parent_size)
      color = QColor()

      current_child_size = displayed_genome_size - self.m_parent_size
      if current_child_size > 0:
        self.updateChildCircle(
          current_child_size,
          self.m_parent_size,
          self.m_parent_circle.radius(),
          self.m_parent_circle.centerX(),
          self.m_parent_circle.centerY(),
          self.m_instruction_spot_radius
        )
        self.m_child_circle.setDTheta(-self.m_child_circle.dTheta())

      for i in xrange(self.m_parent_size):
        pt = self.m_inst_pts[i]
        pt.setHidden(False)
        pt.setCircle(self.m_parent_circle)
        pt.setPosition(i)
      for i in xrange(current_child_size):
        pt = self.m_inst_pts[i + self.m_parent_size]
        pt.setHidden(False)
        pt.setCircle(self.m_child_circle)
        pt.setPosition(i)
      for i in xrange(displayed_genome_size, self.m_max_genome_size):
        pt = self.m_inst_pts[i]
        pt.setHidden(True)

      if self.m_current_frame_number >= (self.m_frames.m_gestation_time - 1) and self.m_frames.m_is_viable:
        self.m_child_circle.setCenterX(self.m_child_circle.centerX() + 2 * self.m_instruction_spot_radius)

      ###
      for i in xrange(self.m_max_genome_size):
        pt = self.m_inst_pts[i]
        item = self.m_inst_items[i]
        bg_item = self.m_inst_bg_items[i]
        if pt.hidden():
          item.hide()
          bg_item.hide()
        else:
          theta = pt.circle().oTheta() + pt.position() * pt.circle().dTheta()
          x = pt.circle().radius()*math.cos(theta) + pt.circle().centerX()
          y = pt.circle().radius()*math.sin(theta) + pt.circle().centerY()

          item.setX(x)
          item.setY(y)
          item.setText(self.m_current_genome[i])
          item.show()

          bg_item.setX(x)
          bg_item.setY(y)
          color.setHsv((self.m_ops_dict[self.m_current_genome[i]] * 360) / len(self.m_ops_dict), 85, 248)
          bg_item.setBrush(QBrush(color))
          bg_item.show()

      for head_info, head_item, head_bg_item, head_text in (
        (self.m_frames.m_ihead_info, self.m_ihead_item, self.m_ihead_bg_item, self.m_ihead_text),
        (self.m_frames.m_rhead_info, self.m_rhead_item, self.m_rhead_bg_item, self.m_rhead_text),
        (self.m_frames.m_whead_info, self.m_whead_item, self.m_whead_bg_item, self.m_whead_text),
        (self.m_frames.m_fhead_info, self.m_fhead_item, self.m_fhead_bg_item, self.m_fhead_text),
      ):
        if head_info is not None:
          head = head_info[self.m_current_frame_number]
          if displayed_genome_size < head:
            head = -1
          pt = self.m_inst_pts[head]
          if pt.hidden():
            head_item.hide()
            head_bg_item.hide()
            head_text.hide()
          else:
            circle = pt.circle()
            theta = circle.oTheta() + pt.position() * circle.dTheta()
            radius = circle.radius()
            cx = circle.centerX()
            cy = circle.centerY()

            head_item.setX(radius*math.cos(theta) + cx)
            head_item.setY(radius*math.sin(theta) + cy)
            head_item.show()

            head_bg_item.setX((radius - head_item.width())*math.cos(theta) + cx)
            head_bg_item.setY((radius - head_item.width())*math.sin(theta) + cy)
            head_bg_item.show()

            head_text.setX((radius - head_item.width())*math.cos(theta) + cx)
            head_text.setY((radius - head_item.width())*math.sin(theta) + cy)
            head_text.show()

    self.emit(PYSIGNAL("frameShownSig"),(self.m_frames, self.m_current_frame_number))
    self.m_canvas.update()
