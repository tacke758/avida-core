from AvidaCore import cEnvironment, cHardwareDefs, cHardwareCPUDefs, cHardwareCPU
from descr import descr

#class pyHardwareCPUFrame:
#  def __init__(self):
#    pass
#    # cCPUMemory memory : cCPUMemory & GetMemory()
#    # cCPUStack global_stack : cCPUStack pyGetGlobalStack()
#    # int thread_time_used : int pyGetThreadTimeUsed()
#    ### assume a single thread
#    # cHardwareCPU_Thread thread : tArray<cHardwareCPU_Thread> pyGetThreads()
#    # bool mal_active : bool GetMalActive()
#    # bool advance_ip : bool pyGetAdvanceIP()

class pyHardwareCPUTrace:
  def __init__(self):
    self.clearFrames()

  def clearFrames(self):
    self.m_hardware_snapshots = []

    self.m_ibuf_0_info = []
    self.m_ibuf_1_info = []
    self.m_ibuf_2_info = []
    self.m_obuf_0_info = []
    self.m_genome_info = []
    self.m_ihead_info = []
    self.m_rhead_info = []
    self.m_whead_info = []
    self.m_fhead_info = []
    self.m_tasks_info = []
    self.m_task_names = []
    self.m_register_ax_info = []
    self.m_register_bx_info = []
    self.m_register_cx_info = []
    self.m_read_label_info = []
    self.m_last_copy_info = []
    self.m_ihead_moves = []
    self.m_ihead_moves_info = []
    self.m_ihead_moves_snapshot = []
    self.m_ihead_moves_counts = {}

    self.m_last_copied_instruction = 0

    self.m_is_viable = None
    self.m_gestation_time = None
    self.m_fitness = None
    self.m_size = None

  def getSnapshotCount(self):
    return len(self.m_hardware_snapshots)
  def getHardwareSnapshotAt(self, frame_number):
    return self.m_hardware_snapshots[frame_number]
  def getMemorySnapshotAt(self, frame_number):
    return self.getHardwareSnapshotAt(frame_number).GetMemory()
  def getThreadsSnapshotAt(self, frame_number):
    return self.getHardwareSnapshotAt(frame_number).pyGetThreads()

  def recordFrame(self, hardware):
    self.m_hardware_snapshots.append(cHardwareCPU(hardware))

    self.m_ibuf_0_info.append(hardware.GetOrganism().GetInputAt(0))
    self.m_ibuf_1_info.append(hardware.GetOrganism().GetInputAt(1))
    self.m_ibuf_2_info.append(hardware.GetOrganism().GetInputAt(2))
    self.m_obuf_0_info.append(hardware.GetOrganism().GetOutputAt(0))

    self.m_genome_info.append(hardware.GetMemory().AsString().GetData())
    self.m_ihead_info.append(hardware.GetHead(cHardwareDefs.s_HEAD_IP).GetPosition())
    self.m_rhead_info.append(hardware.GetHead(cHardwareDefs.s_HEAD_READ).GetPosition())
    self.m_whead_info.append(hardware.GetHead(cHardwareDefs.s_HEAD_WRITE).GetPosition())
    self.m_fhead_info.append(hardware.GetHead(cHardwareDefs.s_HEAD_FLOW).GetPosition())
    self.m_tasks_info.append(hardware.GetOrganism().GetPhenotype().GetCurTaskCount())
    self.m_register_ax_info.append(hardware.GetRegister(cHardwareCPUDefs.s_REG_AX))
    self.m_register_bx_info.append(hardware.GetRegister(cHardwareCPUDefs.s_REG_BX))
    self.m_register_cx_info.append(hardware.GetRegister(cHardwareCPUDefs.s_REG_CX))
    self.m_read_label_info.append(hardware.GetReadLabel().AsString().GetData())

    #descr("self.m_last_copied_instruction", self.m_last_copied_instruction)
    #descr("hardware.GetMemory().GetSize()", hardware.GetMemory().GetSize())
    self.m_last_copied_instruction = max(
      [hardware.GetMemory().GetFlagCopied(i) and i or self.m_last_copied_instruction \
      for i in xrange(self.m_last_copied_instruction, hardware.GetMemory().GetSize())]
    )
    self.m_last_copy_info.append(self.m_last_copied_instruction)

    if 1 < len(self.m_ihead_info):
      if self.m_ihead_info[-2] != self.m_ihead_info[-1]:
        move_count = 1
        if self.m_ihead_moves_counts.has_key((self.m_ihead_info[-2], self.m_ihead_info[-1])):
          move_count = self.m_ihead_moves_counts[(self.m_ihead_info[-2], self.m_ihead_info[-1])] + 1
        self.m_ihead_moves_counts[(self.m_ihead_info[-2], self.m_ihead_info[-1])] = move_count
        self.m_ihead_moves.append((self.m_ihead_info[-2], self.m_ihead_info[-1], move_count))
    self.m_ihead_moves_snapshot.append(self.m_ihead_moves_counts.copy())
    self.m_ihead_moves_info.append(len(self.m_ihead_moves))

  def recordTaskNames(self, environment):
    #for i in range(phenotype.GetEnvironment().GetTaskLib().GetSize()):
    #  self.m_task_names.append(phenotype.GetEnvironment().GetTaskLib().GetTask(i).GetName())
    for i in range(environment.GetTaskLib().GetSize()):
      self.m_task_names.append(environment.GetTaskLib().GetTask(i).GetName())

  def recordGenotypeSummary(self, analyze_genotype):
    self.m_is_viable = analyze_genotype.GetViable()
    self.m_gestation_time = analyze_genotype.GetGestTime()
    self.m_fitness = analyze_genotype.GetFitness()
    self.m_size = analyze_genotype.GetLength()
    if not self.m_is_viable:
      self.m_gestation_time = len(self.m_genome_info)

