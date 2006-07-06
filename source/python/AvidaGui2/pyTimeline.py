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

class pyTimeline(QwtThermo):
    """pyTimeline is a generic timeline control.
    It is derived from the QwtThermo control.  It offers the ability to plot
    flags on the timeline."""
    def __init__(self, *args):
        QwtThermo.__init__(self, *args)
        self.setOrientation(Qt.Horizontal, QwtThermo.Bottom)
        self.setPipeWidth(16)
        self.flags = []

    def addFlag(self, flag):
        "Add new flag to timeline"
        self.flags.append(flag)
#        flag.widget = TimelineFlagWidget(self, flag.pixmap, flag.info)
        flag.widget = TimelineFlagLabel(self, flag.filename, flag.info)
        self.move_flag(flag)
        if self.maxValue() == 0.0:
            flag.widget.hide()
        else:
            flag.widget.show()

    def move_flag(self, flag):
        "Draw the flag in the proper location"
        if self.maxValue() == 0.0:
            # Uninitialized range
            return
        # center widget
	adj = flag.widget.width() / 2
	frame = self.frameSize().width() - self.width()
	# default margin size is 10 (QwtThermo docs)
	borders = self.borderWidth() + 10
	a = self.width() - frame - (borders * 2)
        mult = a / (self.maxValue() - self.minValue())
        flag.widget.move((flag.pos * mult) + (frame / 2) + borders - adj, 2)

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
        self.draw_flags()

    def draw_flags(self):
        "Draw flags on timeline"
        for flag in self.flags:
            self.move_flag(flag)

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
