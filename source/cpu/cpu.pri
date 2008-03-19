
cpu {
  HEADERS += $$CPU_HH/cpu_defs.hh \
             $$CPU_HH/cpu_memory.hh \
             $$CPU_HH/cpu_stack.hh \
             $$CPU_HH/cpu_stats.hh \
             $$CPU_HH/hardware_base.hh \
             $$CPU_HH/hardware_cpu.hh \
             $$CPU_HH/hardware_factory.hh \
             $$CPU_HH/hardware_method.hh \
             $$CPU_HH/hardware_util.hh \
             $$CPU_HH/head.hh \
             $$CPU_HH/label.hh \
             $$CPU_HH/test_cpu.hh \
             $$CPU_HH/test_util.hh

  SOURCES += $$CPU_CC/cpu_memory.cc \
             $$CPU_CC/cpu_stack.cc \
             $$CPU_CC/hardware_base.cc \
             $$CPU_CC/hardware_cpu.cc \
             $$CPU_CC/hardware_factory.cc \
             $$CPU_CC/hardware_util.cc \
             $$CPU_CC/head.cc \
             $$CPU_CC/label.cc \
             $$CPU_CC/test_cpu.cc \
             $$CPU_CC/test_util.cc
}
