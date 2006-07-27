# -*- coding: utf-8 -*-

from qt import Qt, QPainter, QPixmap, QWidget, QToolTip, QSize, QLabel
from qwt import QwtThermo

class TimelineFlagWidget(QWidget):
    "TimelineFlag widget"
    def __init__(self, parent, pixmap, info):
        QWidget.__init__(self, parent)
        self.pixmap = pixmap
        QToolTip.add(self, info)
        self.pixmap_size = QSize(self.pixmap.width(), self.pixmap.height())
        self.setFixedSize(self.pixmap_size)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.drawPixmap(0, 0, self.pixmap)

    def minimumSizeHint(self):
        "Set our minimum size hint to the size of the pixmap"
        return self.pixmap_size

    def sizeHint(self):
        return self.pixmap_size

class TimelineFlagLabel(QLabel):
    "Timeline flag based on QLabel"
    def __init__(self, parent, label, info):
        QLabel.__init__(self, parent)
        self.setText(label)
        QToolTip.add(self, info)
        self.setFixedSize(self.sizeHint())

class TimelineFlag:
    "Flag contains information about event flags."
    def __init__(self, filename, pos, info):
        self.filename = filename
        self.pos = pos
        self.info = info
#        self.pixmap = QPixmap(filename)

    # Overload comparison operators so we can sort by position in timeline
    def __eq__(self, other):
        return (self.filename == other.filename) and \
            (self.pos == other.pos) and \
            (self.info == other.info)
    def __lt__(self, other):
        return self.pos < other.pos
    def __le__(self, other):
        return self.pos <= other.pos
    def __gt__(self, other):
        return self.pos > other.pos
    def __ge__(self, other):
        return self.pos >= other.pos

class pyTimeline(QwtThermo):
    """pyTimeline is a generic timeline control.
    It is derived from the QwtThermo control.  It offers the ability to plot
    flags on the timeline."""
    def __init__(self, *args):
        QwtThermo.__init__(self, *args)
        self.setOrientation(Qt.Horizontal, QwtThermo.Bottom)
        self.setPipeWidth(32)
        self.flags = []         # holds the events for this timeline
        self.clusters = []      # holds event clusters for overlapping events

    def addFlag(self, flag):
        "Add new flag to timeline"
        self.flags.append(flag)
        self.clusters.append([flag])
        flag.cluster = self.clusters[-1]
        self.flags.sort()
#        flag.widget = TimelineFlagWidget(self, flag.pixmap, flag.info)
        flag.widget = TimelineFlagLabel(self, flag.filename, flag.info)
        if self.maxValue() == 0.0:
            # Uninitialized range
            return
	flag.calc_pos = self.get_flag_pos(flag)
        flag.adj_pos = flag.calc_pos
        self.layout_events()
	self.draw_event(flag)
        if self.maxValue() == 0.0:
            flag.widget.hide()
        else:
            flag.widget.show()


    def get_border_width(self):
        "Get full border width"
	# default margin size is 10 (QwtThermo docs)
	return self.borderWidth() + 10

    def get_flag_pos(self, flag):
	"Calculate where in timeline flag should appear"
        # center widget
	adj = flag.widget.width() / 2
	frame = self.frameSize().width() - self.width()
	borders = self.get_border_width()
	a = self.width() - frame - (borders * 2)
        mult = a / (self.maxValue() - self.minValue())
	final_pos = (flag.pos * mult) + (frame / 2) + borders - adj
	return final_pos

    def layout_events(self):
        """Layout flags so there is no overlapping.
        We cluster flags together that are overlapping, and then layout the
        cluster"""
        last_flag = None
        redraw = False
        for flag in self.flags:
            flag_width = flag.widget.width() / 2
            if last_flag:
                if (flag.calc_pos - last_flag.adj_pos) < flag.widget.width() + 2:
                    # This flag overlaps on the last flag
                    flag.cluster.remove(flag)
                    flag.cluster = last_flag.cluster
                    last_flag.cluster.append(flag)
                    flag.cluster.sort()
                    # Clean up empty clusters
                    self.clusters = [c for c in self.clusters if c]
                    self.recalc_cluster(flag.cluster)
                    redraw = True
                else:
                    # flag doesn't overlap previous flag
                    flag.adj_pos = flag.calc_pos
                    if len(flag.cluster) > 1:
                        flag.cluster.remove(flag)
                        if flag.cluster:
                            self.recalc_cluster(flag.cluster)
                        # Clean up empty clusters
                        self.clusters = [c for c in self.clusters if c]
                        self.clusters.append([flag])
                        flag.cluster = self.clusters[-1]

            last_flag = flag

        if redraw:
            self.repaint()

    def recalc_cluster(self, cluster):
        "Recalculate event cluster location"
        # Cluster position is the average of the events in the
        # cluster
        sum = 0.0
        widget_sum = 0
        for flag in cluster:
            sum += flag.calc_pos
            widget_sum += flag.widget.width()
        avg = sum / len(cluster)
        start = widget_sum / 2
        start = avg - start + (cluster[0].widget.width() / 2)
        if start < self.get_border_width() + 1:
            start = self.get_border_width() + 1
        for flag in cluster:
            flag.adj_pos = start
            start += flag.widget.width()

    def draw_event(self, flag):
	"Draw both flag and line"
	self.draw_flag(flag)
	self.draw_line(flag)

    def draw_flag(self, flag):
        "Draw the flag in the proper location"
        flag.widget.move(flag.adj_pos, 2)

    def draw_line(self, flag):
	"Draw line connecting flag and event in timeline"
        flag_center = flag.widget.width() / 2
	flag_bottom = flag.widget.height()
        painter = QPainter(self)
        orig = flag.calc_pos + flag_center
	final = flag.adj_pos + flag_center
	line_height = self.pipeWidth() - flag_bottom - 6
	painter.drawLine(orig, self.pipeWidth(), orig,
                         self.pipeWidth() - 4)
	painter.drawLine(orig, self.pipeWidth() - 4, final,
			 self.pipeWidth() - line_height)
	painter.drawLine(final, self.pipeWidth() - line_height,
                         final, flag_bottom)

    def removeFlag(self, pos):
        "Remove flag from timeline position pos"
        tmp_flag = None
        for flag in self.flags:
            if flag.pos == pos:
                tmp_flag = flag
        if tmp_flag:
            self.flags.remove(tmp_flag)
            tmp_flag.widget.hide()

    def paintEvent(self, event):
        "Paint event handler"
        QwtThermo.paintEvent(self, event)
	# draw the flags
        self.draw_events()

    def draw_events(self):
        "Draw flags on timeline"
        for flag in self.flags:
            self.draw_event(flag)

    def setMaxValue(self, value):
        "Set the range max value"
        old_value = self.maxValue()
        QwtThermo.setMaxValue(self, value)
        if old_value == 0.0:
            for flag in self.flags:
                flag.widget.show()

    def setIncrementalShow(flag):
        "Hide flags until the meter hits their position"
        self.surprise = flag

    def reset(self):
        "Reset the timeline, hiding all flags"
        for flag in self.flags:
            flag.widget.close(True)
        self.flags = []

    def resizeEvent(self, evt):
        "Resize event"
        QwtThermo.resizeEvent(self, evt)
        for flag in self.flags:
            flag.calc_pos = self.get_flag_pos(flag)
        self.layout_events()
