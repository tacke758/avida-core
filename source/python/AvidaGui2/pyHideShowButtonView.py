# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyHideShowButtonView.ui'
#
# Created: Mon Apr 3 17:15:33 2006
#      by: The PyQt User Interface Compiler (pyuic) 3.15.1
#
# WARNING! All changes made in this file will be lost!


from qt import *


class pyHideShowButtonView(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("pyHideShowButtonView")


        pyHideShowButtonViewLayout = QVBoxLayout(self,0,0,"pyHideShowButtonViewLayout")

        self.m_widget_stack = QWidgetStack(self,"m_widget_stack")

        self.m_is_hidden_page = QWidget(self.m_widget_stack,"m_is_hidden_page")
        m_is_hidden_pageLayout = QVBoxLayout(self.m_is_hidden_page,0,0,"m_is_hidden_pageLayout")

        self.m_is_hidden_button = QToolButton(self.m_is_hidden_page,"m_is_hidden_button")
        m_is_hidden_pageLayout.addWidget(self.m_is_hidden_button)
        self.m_widget_stack.addWidget(self.m_is_hidden_page,0)

        self.m_is_shown_page = QWidget(self.m_widget_stack,"m_is_shown_page")
        m_is_shown_pageLayout = QVBoxLayout(self.m_is_shown_page,0,0,"m_is_shown_pageLayout")

        self.m_is_shown_button = QToolButton(self.m_is_shown_page,"m_is_shown_button")
        m_is_shown_pageLayout.addWidget(self.m_is_shown_button)
        self.m_widget_stack.addWidget(self.m_is_shown_page,1)
        pyHideShowButtonViewLayout.addWidget(self.m_widget_stack)

        self.languageChange()

        self.resize(QSize(149,149).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)


    def languageChange(self):
        self.setCaption(self.__tr("pyHideShowButtonView"))
        self.m_is_hidden_button.setText(self.__tr("..."))
        self.m_is_shown_button.setText(self.__tr("..."))


    def __tr(self,s,c = None):
        return qApp.translate("pyHideShowButtonView",s,c)
