# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyMDSetupForm.ui'
#
# Created: Thu Jul 23 17:49:09 2009
#      by: The PyQt User Interface Compiler (pyuic) 3.17.6
#
# WARNING! All changes made in this file will be lost!


from qt import *


class md_setup_Form(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("md_setup_Form")



        self.frame3 = QFrame(self,"frame3")
        self.frame3.setGeometry(QRect(20,20,580,30))
        self.frame3.setFrameShape(QFrame.StyledPanel)
        self.frame3.setFrameShadow(QFrame.Raised)

        self.MD_Setup_textLabel = QLabel(self.frame3,"MD_Setup_textLabel")
        self.MD_Setup_textLabel.setGeometry(QRect(21,1,108,27))

        self.MD_Setup_Switch_toolButton = QToolButton(self.frame3,"MD_Setup_Switch_toolButton")
        self.MD_Setup_Switch_toolButton.setGeometry(QRect(505,1,52,27))

        self.frame4 = QFrame(self,"frame4")
        self.frame4.setGeometry(QRect(20,400,500,170))
        self.frame4.setFrameShape(QFrame.StyledPanel)
        self.frame4.setFrameShadow(QFrame.Raised)

        LayoutWidget = QWidget(self.frame4,"layout4")
        LayoutWidget.setGeometry(QRect(20,10,460,148))
        layout4 = QGridLayout(LayoutWidget,1,1,11,6,"layout4")

        self.sd_4_pushButton = QPushButton(LayoutWidget,"sd_4_pushButton")

        layout4.addWidget(self.sd_4_pushButton,0,3)

        self.sd_2_pushButton = QPushButton(LayoutWidget,"sd_2_pushButton")

        layout4.addWidget(self.sd_2_pushButton,0,1)

        self.sd_12_pushButton = QPushButton(LayoutWidget,"sd_12_pushButton")

        layout4.addWidget(self.sd_12_pushButton,2,3)

        self.sd_14_pushButton = QPushButton(LayoutWidget,"sd_14_pushButton")

        layout4.addWidget(self.sd_14_pushButton,3,1)

        self.sd_13_pushButton = QPushButton(LayoutWidget,"sd_13_pushButton")

        layout4.addWidget(self.sd_13_pushButton,3,0)

        self.sd1_pushButton = QPushButton(LayoutWidget,"sd1_pushButton")

        layout4.addWidget(self.sd1_pushButton,0,0)

        self.sd_16_pushButton = QPushButton(LayoutWidget,"sd_16_pushButton")

        layout4.addWidget(self.sd_16_pushButton,3,3)

        self.sd_10_pushButton = QPushButton(LayoutWidget,"sd_10_pushButton")

        layout4.addWidget(self.sd_10_pushButton,2,1)

        self.sd_11_pushButton = QPushButton(LayoutWidget,"sd_11_pushButton")

        layout4.addWidget(self.sd_11_pushButton,2,2)

        self.sd_3_pushButton = QPushButton(LayoutWidget,"sd_3_pushButton")

        layout4.addWidget(self.sd_3_pushButton,0,2)

        self.sd_6_pushButton = QPushButton(LayoutWidget,"sd_6_pushButton")

        layout4.addWidget(self.sd_6_pushButton,1,1)

        self.sd_7_pushButton = QPushButton(LayoutWidget,"sd_7_pushButton")

        layout4.addWidget(self.sd_7_pushButton,1,2)

        self.sd_5_pushButton = QPushButton(LayoutWidget,"sd_5_pushButton")

        layout4.addWidget(self.sd_5_pushButton,1,0)

        self.sd_9_pushButton = QPushButton(LayoutWidget,"sd_9_pushButton")

        layout4.addWidget(self.sd_9_pushButton,2,0)

        self.sd_15_pushButton = QPushButton(LayoutWidget,"sd_15_pushButton")

        layout4.addWidget(self.sd_15_pushButton,3,2)

        self.sd_8_pushButton = QPushButton(LayoutWidget,"sd_8_pushButton")

        layout4.addWidget(self.sd_8_pushButton,1,3)

        self.frame5 = QFrame(self,"frame5")
        self.frame5.setGeometry(QRect(20,60,450,190))
        self.frame5.setFrameShape(QFrame.StyledPanel)
        self.frame5.setFrameShadow(QFrame.Raised)

        LayoutWidget_2 = QWidget(self.frame5,"layout16")
        LayoutWidget_2.setGeometry(QRect(20,10,401,170))
        layout16 = QGridLayout(LayoutWidget_2,1,1,11,6,"layout16")

        self.md_values_textLabel = QLabel(LayoutWidget_2,"md_values_textLabel")

        layout16.addMultiCellWidget(self.md_values_textLabel,3,3,1,2)

        layout8 = QHBoxLayout(None,0,6,"layout8")

        self.sdnx_textLabel = QLabel(LayoutWidget_2,"sdnx_textLabel")
        layout8.addWidget(self.sdnx_textLabel)

        self.sdnx_spinBox = QSpinBox(LayoutWidget_2,"sdnx_spinBox")
        self.sdnx_spinBox.setMinValue(1)
        self.sdnx_spinBox.setValue(40)
        layout8.addWidget(self.sdnx_spinBox)

        layout16.addLayout(layout8,0,2)

        layout5 = QHBoxLayout(None,0,6,"layout5")

        self.sdx_textLabel = QLabel(LayoutWidget_2,"sdx_textLabel")
        layout5.addWidget(self.sdx_textLabel)

        self.sdx_spinBox = QSpinBox(LayoutWidget_2,"sdx_spinBox")
        self.sdx_spinBox.setMinValue(1)
        self.sdx_spinBox.setValue(4)
        layout5.addWidget(self.sdx_spinBox)

        layout16.addMultiCellLayout(layout5,0,0,0,1)

        layout10 = QHBoxLayout(None,0,6,"layout10")

        self.md_name_textLabel = QLabel(LayoutWidget_2,"md_name_textLabel")
        layout10.addWidget(self.md_name_textLabel)

        self.md_name_lineEdit = QLineEdit(LayoutWidget_2,"md_name_lineEdit")
        layout10.addWidget(self.md_name_lineEdit)

        layout16.addMultiCellLayout(layout10,2,2,0,2)

        layout7 = QHBoxLayout(None,0,6,"layout7")

        self.sdy_textLabel = QLabel(LayoutWidget_2,"sdy_textLabel")
        layout7.addWidget(self.sdy_textLabel)

        self.sdy_spinBox = QSpinBox(LayoutWidget_2,"sdy_spinBox")
        self.sdy_spinBox.setMinValue(1)
        self.sdy_spinBox.setValue(4)
        layout7.addWidget(self.sdy_spinBox)

        layout16.addMultiCellLayout(layout7,1,1,0,1)

        self.md_set_values_pushButton = QPushButton(LayoutWidget_2,"md_set_values_pushButton")

        layout16.addWidget(self.md_set_values_pushButton,3,0)

        layout9 = QHBoxLayout(None,0,6,"layout9")

        self.sdny_textLabel = QLabel(LayoutWidget_2,"sdny_textLabel")
        layout9.addWidget(self.sdny_textLabel)

        self.sdny_spinBox = QSpinBox(LayoutWidget_2,"sdny_spinBox")
        self.sdny_spinBox.setMinValue(1)
        self.sdny_spinBox.setValue(40)
        layout9.addWidget(self.sdny_spinBox)

        layout16.addLayout(layout9,1,2)

        self.md_generate_pushButton = QPushButton(LayoutWidget_2,"md_generate_pushButton")

        layout16.addMultiCellWidget(self.md_generate_pushButton,4,4,0,1)

        self.frame6 = QFrame(self,"frame6")
        self.frame6.setGeometry(QRect(20,260,501,130))
        self.frame6.setFrameShape(QFrame.StyledPanel)
        self.frame6.setFrameShadow(QFrame.Raised)

        LayoutWidget_3 = QWidget(self.frame6,"layout15")
        LayoutWidget_3.setGeometry(QRect(20,11,265,101))
        layout15 = QGridLayout(LayoutWidget_3,1,1,11,6,"layout15")

        self.sd_pick_textLabel = QLabel(LayoutWidget_3,"sd_pick_textLabel")

        layout15.addWidget(self.sd_pick_textLabel,0,0)

        layout14 = QHBoxLayout(None,0,6,"layout14")

        self.sd_pos_in_md_textLabel = QLabel(LayoutWidget_3,"sd_pos_in_md_textLabel")
        layout14.addWidget(self.sd_pos_in_md_textLabel)

        self.sd_pos_in_md_spinBox = QSpinBox(LayoutWidget_3,"sd_pos_in_md_spinBox")
        self.sd_pos_in_md_spinBox.setMinValue(1)
        layout14.addWidget(self.sd_pos_in_md_spinBox)

        layout15.addMultiCellLayout(layout14,1,1,0,1)

        self.sd_pick_comboBox = QComboBox(0,LayoutWidget_3,"sd_pick_comboBox")

        layout15.addWidget(self.sd_pick_comboBox,0,1)

        self.sd_add_pushButton = QPushButton(LayoutWidget_3,"sd_add_pushButton")

        layout15.addMultiCellWidget(self.sd_add_pushButton,2,2,0,1)

        self.languageChange()

        self.resize(QSize(667,677).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)


    def languageChange(self):
        self.setCaption(self.__tr("Multi-Dish Setup"))
        self.MD_Setup_textLabel.setText(self.__tr("Multi-Dish Setup"))
        self.MD_Setup_Switch_toolButton.setText(self.__tr("Switch"))
        self.sd_4_pushButton.setText(self.__tr("SubDish 4"))
        self.sd_2_pushButton.setText(self.__tr("SubDish 2"))
        self.sd_12_pushButton.setText(self.__tr("SubDish 12"))
        self.sd_14_pushButton.setText(self.__tr("SubDish 14"))
        self.sd_13_pushButton.setText(self.__tr("SubDish 13"))
        self.sd1_pushButton.setText(self.__tr("SubDish 1"))
        self.sd_16_pushButton.setText(self.__tr("SubDish 16"))
        self.sd_10_pushButton.setText(self.__tr("SubDish 10"))
        self.sd_11_pushButton.setText(self.__tr("SubDish 11"))
        self.sd_3_pushButton.setText(self.__tr("SubDish 3"))
        self.sd_6_pushButton.setText(self.__tr("SubDish 6"))
        self.sd_7_pushButton.setText(self.__tr("SubDish 7"))
        self.sd_5_pushButton.setText(self.__tr("SubDish 5"))
        self.sd_9_pushButton.setText(self.__tr("SubDish 9"))
        self.sd_15_pushButton.setText(self.__tr("SubDish 15"))
        self.sd_8_pushButton.setText(self.__tr("SubDish 8"))
        self.md_values_textLabel.setText(self.__tr("MD: "))
        self.sdnx_textLabel.setText(self.__tr("Grid Cells / SubDish row"))
        self.sdx_textLabel.setText(self.__tr("SubDishes / row"))
        self.md_name_textLabel.setText(self.__tr("Multi-Dish name"))
        self.sdy_textLabel.setText(self.__tr("SubDishes /  col"))
        self.md_set_values_pushButton.setText(self.__tr("Set Values"))
        self.sdny_textLabel.setText(self.__tr("Grid Cells / SubDish  col"))
        self.md_generate_pushButton.setText(self.__tr("Generate Multi-Dish"))
        self.sd_pick_textLabel.setText(self.__tr("SubDish"))
        self.sd_pos_in_md_textLabel.setText(self.__tr("SubDish Position in Multi-Dish"))
        self.sd_add_pushButton.setText(self.__tr("Add SubDish"))


    def __tr(self,s,c = None):
        return qApp.translate("md_setup_Form",s,c)
