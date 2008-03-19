//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef N_ORIG_INSTRUCTION_CPU_WIDGET_HH
#define N_ORIG_INSTRUCTION_CPU_WIDGET_HH

#include <qframe.h>
#include <qgroupbox.h>
#include <qguardedptr.h>
#include <qptrlist.h>
#include <qwidget.h>


class avd_MissionControl;
class QPushButton;
class QHBoxLayout;
class QLabel;
class QScrollView;
class QVBoxLayout;
class QWidgetStack;

class
cLabeledField : public QWidget {
  Q_OBJECT
protected:
  QHBoxLayout *m_hboxlayout;
  QLabel *m_title_label;
  QLabel *m_data1_label;
  QLabel *m_data2_label;
public:
  cLabeledField(
    QWidget *parent = 0,
    const char *name = 0,
    WFlags f = WDestructiveClose
  );
  void setTitle(const QString &title);
  void setData1Text(const QString &text);
  void setData2Text(const QString &text);
  void setData2Binary(unsigned int value);
  void setData1DecimalWidth(void);
  void setData2BinaryWidth(void);
public slots:
  void showData2Text(bool);
};

class
cSummaryWidget : public QGroupBox {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;

  cLabeledField *m_location;
  cLabeledField *m_genotype_id;
  cLabeledField *m_genotype_name;
  cLabeledField *m_faults;
  cLabeledField *m_offspring;
  cLabeledField *m_thread;
public:
  cSummaryWidget(
    QWidget *parent = 0,
    const char *name = 0
  );
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void updateState(void);
};

class
cRegistersWidget : public QGroupBox {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;
  QPtrList<cLabeledField> m_registers;
public:
  cRegistersWidget(
    QWidget *parent = 0,
    const char *name = 0
  );
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void updateState(void);
  void showData2Text(bool);
};

class
cInputsWidget : public QGroupBox {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;
  QPtrList<cLabeledField> m_inputs;
public:
  cInputsWidget(
    QWidget *parent = 0,
    const char *name = 0
  );
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void updateState(void);
  void showData2Text(bool);
};

class
cStackPopup : public QWidget {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;
  int m_stack_no;

  QVBoxLayout *m_vboxlayout;
    QScrollView *m_scrollview;
      QWidget *m_layout_widget;
        QVBoxLayout *m_sv_layout;
          QPtrList<cLabeledField> m_stacklines;
public:
  cStackPopup(
    QWidget *parent = 0,
    const char *name = 0
  );
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void setStackNumber(int stack_number);
  void updateState(void);
  void showData2Text(bool);
};

class
cStackField : public cLabeledField {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;
  int m_stack_no;

  QPushButton *m_popup_pbutton;
  cStackPopup *m_stack_popup;
public:
  cStackField(
    QWidget *parent = 0,
    const char *name = 0,
    WFlags f = WDestructiveClose
  );
  void useFrame(bool);
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void setStackNumber(int stack_number);
  void updateState(void);
  void showData2Text(bool);
protected slots:
  void displayPopupSlot(void);
};

class
cStacksWidget : public QGroupBox {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;

  cStackField *m_stackln1;
  cStackField *m_stackln2;

public:
  cStacksWidget(
    QWidget *parent = 0,
    const char *name = 0
  );
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void updateState(void);
  void showData2Text(bool);
};

class
cHideShowBinary : public QWidget {
  Q_OBJECT
protected:
  QWidgetStack *m_widgetstack;
    QPushButton *m_show_binary_pbutton;
    QPushButton *m_hide_binary_pbutton;
public:
  cHideShowBinary(
    QWidget *parent = 0,
    const char *name = 0,
    WFlags f = WDestructiveClose
  );
public slots:
  void showBinary(void);
  void hideBinary(void);
signals:
  void showBinarySig(bool);
};

class
N_Instruction_CPUWidget : public QWidget {
  Q_OBJECT
protected:
  QGuardedPtr<avd_MissionControl> m_mission_control;
  int m_cell_id;

  QVBoxLayout *m_vboxlayout;
    cSummaryWidget *m_summary;
    cRegistersWidget *m_registers;
    cStacksWidget *m_stacks;
    cInputsWidget *m_inputs;
    cHideShowBinary *m_hideshow_binary;
public:
  N_Instruction_CPUWidget(
    QWidget *parent = 0,
    const char *name = 0,
    WFlags f = WDestructiveClose
  );
  void setMissionControl(avd_MissionControl *mission_control);
public slots:
  void setPopulationCell(int cell_id);
  void updateState(void);
  void showData2Text(bool);
};

#endif /* !N_ORIG_INSTRUCTION_CPU_WIDGET_HH */
