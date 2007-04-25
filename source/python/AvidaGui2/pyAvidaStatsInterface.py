
from AvidaCore import cInitFile, cString
from AvidaCore import cPopulation, cStats
from Numeric import *
from pyButtonListDialog import pyButtonListDialog
from qt import QFileDialog
import os.path

class pyAvidaStatsEntry:
  def __init__(self, name, file, index, func):
    self.name = name
    self.file = file
    self.index = index
    self.func = func
  def setname(self, name):
    self.name = name
  def setfile(self, file):
    self.file = file
  def setindex(self, index):
    self.index = index
  def setfunc(self, func):
    self.func = func

class pyAvidaStatsInterface:
  def __init__(self):
    self.m_e = (
      ('None',                          None,            0, None),
      ('Average Metabolic Rate',                 'average.dat',   2, lambda s: s.GetAveMerit()),
      ('Average Fitness',               'average.dat',   4, lambda s: s.GetAveFitness()),
      ('Average Gestation Time',        'average.dat',   3, lambda s: s.GetAveGestation()),
#      ('Average Genome Length',                  'average.dat',   6, lambda s: s.GetAveSize()),
#      ('Average Generation',            'average.dat',  13, lambda s: s.SumGeneration().Average()),
#      ('Average Neutral Metric',        'average.dat',  14, lambda s: s.GetAveNeutralMetric()),
#      ('Average Lineage Label',         'average.dat',  15, lambda s: s.GetAveLineageLabel()),
#      ('Dominant Merit',                'dominant.dat',  2, lambda s: s.GetDomMerit()),
#      ('Dominant Gestation Time',       'dominant.dat',  3, lambda s: s.GetDomGestation()),
#      ('Dominant Fitness',              'dominant.dat',  4, lambda s: s.GetDomFitness()),
#      ('Dominant Size',                 'dominant.dat',  6, lambda s: s.GetDomSize()),
      ('Number of Organisms',           'count.dat',     3, lambda s: s.GetNumCreatures()),
#      ('Number of Genotypes',           'count.dat',     4, lambda s: s.GetNumGenotypes()),
#      ('Number of Threshold Genotypes', 'count.dat',     5, lambda s: s.GetNumThreshold()),
#      ('Number of Births',              'count.dat',     9, lambda s: s.GetNumBirths()),
#      ('Number of Deaths',              'count.dat',    10, lambda s: s.GetNumDeaths()),
    )
    self.m_entries = []
    for entry in self.m_e:
      self.m_entries.append(pyAvidaStatsEntry(entry[0], entry[1], entry[2],
                                              entry[3]))

  def getValue(self, entry_index, stats):
    if entry_index:
      return self.m_entries[entry_index].func(stats)

  def load(self, path, filename, colx, coly):
    "Load stats from file"
    init_file = cInitFile(cString(os.path.join(path, filename)))
    init_file.Load()
    init_file.Compress()

    x_array = zeros(init_file.GetNumLines(), Float)
    y_array = zeros(init_file.GetNumLines(), Float)

    for line_id in xrange(init_file.GetNumLines()):
      line = init_file.GetLine(line_id)
      x_array[line_id] = line.GetWord(colx - 1).AsDouble()
      y_array[line_id] = line.GetWord(coly - 1).AsDouble()
    return x_array, y_array

  def export(self, paths):
    """Export stats to a file.  Can export multiple populations now.
    paths is a array of tuples containing short name and full path. """
    dialog_caption = "Export Analysis"
    fd = QFileDialog.getSaveFileName("", "Text -- Excel compatible (*.txt);;CSV  -- Excel compatible (*.csv)", None,
                                     "export as", dialog_caption)
    filename = str(fd)
    if (filename[-4:].lower() != ".txt"):
      filename += ".txt"

    checks = []
    # dictionary indexed by stat name so we can lookup stats to export
    stats = {}
    stat_cnt = 0
    for stat in self.m_entries:
      # Note: this relies on labeling dummy stats with None
      if stat.name != "None":
        stats[stat.name] = stat_cnt
        checks.append(stat.name)
      stat_cnt += 1

    dialog = pyButtonListDialog(dialog_caption, "Choose stats to export",
                                checks, True)
    # enable checkboxes
    for button in dialog.buttons:
      button.setOn(True)

    res = dialog.showDialog()
    if res == []:
      return

    data = {}

    # Load stats for selected exports
    for population, path in paths:
      data[population] = {}
      for item in res:
        idx = stats[item]
        label1 = self.m_entries[idx].name
        data[population][item] = self.load(path, self.m_entries[idx].file, 1,
                                           self.m_entries[idx].index)

    out_file = open(filename, 'w')

    # write out the header
    out_file.write("Update,")
    header = ""
    for population, path in paths:
      if header != "":
        header += ","
      header += ",".join(["%s %s" % (population, item) for item in res])
    header += "\n"
    out_file.write(header)

    max_updates = 0
    # find the number of updates in each population
    for population, path in paths:
      data[population]["num_updates"] = len(data[population][res[0]][0])
      if data[population]["num_updates"] > max_updates:
        max_updates = data[population]["num_updates"]

    # write out the data
    for i in range(max_updates):
      tmp = "%d" % (i)
      for population, path in paths:
        tmp += ","
        if data[population]["num_updates"] > i:
          tmp += ",".join([str(data[population][elem][1][i]) for elem in res])
        else:
          tmp += ",".join(["" for elem in res])
      tmp += "\n"
      out_file.write(tmp)

    out_file.close()
