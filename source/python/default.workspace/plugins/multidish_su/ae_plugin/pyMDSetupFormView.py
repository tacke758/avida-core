# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyMDSetupFormView.ui'
#
# Created: Sat Aug 15 23:40:16 2009
#      by: The PyQt User Interface Compiler (pyuic) 3.17.6
#
# WARNING! All changes made in this file will be lost!


from qt import *


class md_setup_Form(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("md_setup_Form")



        self.tabWidget2 = QTabWidget(self,"tabWidget2")
        self.tabWidget2.setGeometry(QRect(10,10,790,650))

        self.tab = QWidget(self.tabWidget2,"tab")

        self.generateMulti_Dish_groupBox = QGroupBox(self.tab,"generateMulti_Dish_groupBox")
        self.generateMulti_Dish_groupBox.setGeometry(QRect(508,442,250,130))

        self.md_generate_pushButton = QPushButton(self.generateMulti_Dish_groupBox,"md_generate_pushButton")
        self.md_generate_pushButton.setGeometry(QRect(22,58,200,32))

        self.setSubDishLayout_groupBox = QGroupBox(self.tab,"setSubDishLayout_groupBox")
        self.setSubDishLayout_groupBox.setGeometry(QRect(0,200,500,410))

        LayoutWidget = QWidget(self.setSubDishLayout_groupBox,"layout20")
        LayoutWidget.setGeometry(QRect(12,28,460,50))
        layout20 = QHBoxLayout(LayoutWidget,11,6,"layout20")

        self.sd_pick_textLabel = QLabel(LayoutWidget,"sd_pick_textLabel")
        layout20.addWidget(self.sd_pick_textLabel)

        self.sd_pick_comboBox = QComboBox(0,LayoutWidget,"sd_pick_comboBox")
        layout20.addWidget(self.sd_pick_comboBox)

        self.sd_list_refresh_pushButton = QPushButton(LayoutWidget,"sd_list_refresh_pushButton")
        layout20.addWidget(self.sd_list_refresh_pushButton)

        self.sd_pos_in_md_textLabel = QLabel(self.setSubDishLayout_groupBox,"sd_pos_in_md_textLabel")
        self.sd_pos_in_md_textLabel.setGeometry(QRect(15,91,247,32))

        self.sd_pos_in_md_spinBox = QSpinBox(self.setSubDishLayout_groupBox,"sd_pos_in_md_spinBox")
        self.sd_pos_in_md_spinBox.setGeometry(QRect(268,94,59,26))
        self.sd_pos_in_md_spinBox.setMinValue(1)

        self.sd_add_pushButton = QPushButton(self.setSubDishLayout_groupBox,"sd_add_pushButton")
        self.sd_add_pushButton.setGeometry(QRect(333,91,119,32))

        self.md_layout_textEdit = QTextEdit(self.setSubDishLayout_groupBox,"md_layout_textEdit")
        self.md_layout_textEdit.setGeometry(QRect(10,130,480,270))
        md_layout_textEdit_font = QFont(self.md_layout_textEdit.font())
        md_layout_textEdit_font.setFamily("Courier New")
        md_layout_textEdit_font.setBold(1)
        self.md_layout_textEdit.setFont(md_layout_textEdit_font)
        self.md_layout_textEdit.setWordWrap(QTextEdit.NoWrap)

        self.setMDParams_groupBox = QGroupBox(self.tab,"setMDParams_groupBox")
        self.setMDParams_groupBox.setGeometry(QRect(0,0,500,200))

        self.sdy_textLabel = QLabel(self.setMDParams_groupBox,"sdy_textLabel")
        self.sdy_textLabel.setGeometry(QRect(12,66,116,26))

        self.sdy_spinBox = QSpinBox(self.setMDParams_groupBox,"sdy_spinBox")
        self.sdy_spinBox.setGeometry(QRect(134,66,116,26))
        self.sdy_spinBox.setMinValue(1)
        self.sdy_spinBox.setValue(4)

        self.sdx_textLabel = QLabel(self.setMDParams_groupBox,"sdx_textLabel")
        self.sdx_textLabel.setGeometry(QRect(12,32,116,26))

        self.sdx_spinBox = QSpinBox(self.setMDParams_groupBox,"sdx_spinBox")
        self.sdx_spinBox.setGeometry(QRect(134,32,116,26))
        self.sdx_spinBox.setMinValue(1)
        self.sdx_spinBox.setValue(4)

        self.sdnx_textLabel = QLabel(self.setMDParams_groupBox,"sdnx_textLabel")
        self.sdnx_textLabel.setGeometry(QRect(258,32,155,26))

        self.sdnx_spinBox = QSpinBox(self.setMDParams_groupBox,"sdnx_spinBox")
        self.sdnx_spinBox.setGeometry(QRect(419,32,59,26))
        self.sdnx_spinBox.setMinValue(1)
        self.sdnx_spinBox.setValue(30)

        self.sdny_textLabel = QLabel(self.setMDParams_groupBox,"sdny_textLabel")
        self.sdny_textLabel.setGeometry(QRect(258,66,155,26))

        self.sdny_spinBox = QSpinBox(self.setMDParams_groupBox,"sdny_spinBox")
        self.sdny_spinBox.setGeometry(QRect(419,66,59,26))
        self.sdny_spinBox.setMinValue(1)
        self.sdny_spinBox.setValue(30)

        self.md_values_textLabel = QLabel(self.setMDParams_groupBox,"md_values_textLabel")
        self.md_values_textLabel.setGeometry(QRect(133,160,345,32))

        self.md_set_values_pushButton = QPushButton(self.setMDParams_groupBox,"md_set_values_pushButton")
        self.md_set_values_pushButton.setGeometry(QRect(10,160,117,32))

        self.md_name_lineEdit = QLineEdit(self.setMDParams_groupBox,"md_name_lineEdit")
        self.md_name_lineEdit.setGeometry(QRect(125,130,353,22))

        self.md_name_textLabel = QLabel(self.setMDParams_groupBox,"md_name_textLabel")
        self.md_name_textLabel.setGeometry(QRect(12,130,107,22))

        self.MD_mutation_on_checkBox = QCheckBox(self.setMDParams_groupBox,"MD_mutation_on_checkBox")
        self.MD_mutation_on_checkBox.setGeometry(QRect(10,100,120,22))
        self.MD_mutation_on_checkBox.setChecked(1)
        self.tabWidget2.insertTab(self.tab,QString.fromLatin1(""))

        self.tab_2 = QWidget(self.tabWidget2,"tab_2")

        self.MDRunStatistics_groupBox = QGroupBox(self.tab_2,"MDRunStatistics_groupBox")
        self.MDRunStatistics_groupBox.setGeometry(QRect(0,0,760,150))

        self.MDRunStatistics_textEdit = QTextEdit(self.MDRunStatistics_groupBox,"MDRunStatistics_textEdit")
        self.MDRunStatistics_textEdit.setGeometry(QRect(0,20,760,130))
        MDRunStatistics_textEdit_font = QFont(self.MDRunStatistics_textEdit.font())
        MDRunStatistics_textEdit_font.setFamily("Courier New")
        MDRunStatistics_textEdit_font.setPointSize(11)
        self.MDRunStatistics_textEdit.setFont(MDRunStatistics_textEdit_font)
        self.MDRunStatistics_textEdit.setWordWrap(QTextEdit.NoWrap)

        self.MDDiversity_groupBox = QGroupBox(self.tab_2,"MDDiversity_groupBox")
        self.MDDiversity_groupBox.setGeometry(QRect(0,150,760,460))

        self.MDDiversity_textEdit = QTextEdit(self.MDDiversity_groupBox,"MDDiversity_textEdit")
        self.MDDiversity_textEdit.setGeometry(QRect(-2,22,760,430))
        MDDiversity_textEdit_font = QFont(self.MDDiversity_textEdit.font())
        MDDiversity_textEdit_font.setFamily("Courier New")
        MDDiversity_textEdit_font.setPointSize(11)
        self.MDDiversity_textEdit.setFont(MDDiversity_textEdit_font)
        self.MDDiversity_textEdit.setHScrollBarMode(QTextEdit.AlwaysOn)
        self.MDDiversity_textEdit.setTextFormat(QTextEdit.PlainText)
        self.MDDiversity_textEdit.setWordWrap(QTextEdit.NoWrap)
        self.tabWidget2.insertTab(self.tab_2,QString.fromLatin1(""))

        self.languageChange()

        self.resize(QSize(801,659).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)


    def languageChange(self):
        self.setCaption(self.__tr("Multi-Dish"))
        self.generateMulti_Dish_groupBox.setTitle(self.__tr("Step 3. Generate Multi-Dish"))
        self.md_generate_pushButton.setText(self.__tr("Generate Multi-Dish"))
        self.setSubDishLayout_groupBox.setTitle(self.__tr("Step 2. Layout SubDishes"))
        self.sd_pick_textLabel.setText(self.__tr("Choose SubDish"))
        self.sd_list_refresh_pushButton.setText(self.__tr("Refresh"))
        self.sd_pos_in_md_textLabel.setText(self.__tr("Choose SubDish Position in Multi-Dish"))
        self.sd_add_pushButton.setText(self.__tr("Add SubDish"))
        self.setMDParams_groupBox.setTitle(self.__tr("Step 1: Set Multi-Dish Parameters"))
        self.sdy_textLabel.setText(self.__tr("SubDishes /  col"))
        self.sdx_textLabel.setText(self.__tr("SubDishes / row"))
        self.sdnx_textLabel.setText(self.__tr("Grid Cells / SubDish row"))
        self.sdny_textLabel.setText(self.__tr("Grid Cells / SubDish  col"))
        self.md_values_textLabel.setText(self.__tr("MD: "))
        self.md_set_values_pushButton.setText(self.__tr("Set Values"))
        self.md_name_lineEdit.setText(self.__tr("md-"))
        self.md_name_textLabel.setText(self.__tr("Multi-Dish name"))
        self.MD_mutation_on_checkBox.setText(self.__tr("Mutation on?"))
        self.tabWidget2.changeTab(self.tab,self.__tr("Multi-Dish Setup"))
        self.MDRunStatistics_groupBox.setTitle(self.__tr("Run Statistics"))
        self.MDDiversity_groupBox.setTitle(self.__tr("Multi-Dish Diversity"))
        self.tabWidget2.changeTab(self.tab_2,self.__tr("Multi-Dish Statistics"))


    def __tr(self,s,c = None):
        return qApp.translate("md_setup_Form",s,c)
