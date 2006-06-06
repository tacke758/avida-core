
from AvidaCore import cInitFile, cString
from AvidaCore import cPopulation, cStats
from Numeric import *
from pyButtonListDialog import pyButtonListDialog
from qt import QFileDialog
import os.path

class pyAvidaStatsInterface:
  def __init__(self):
    self.m_entries = (
      ('None',                          None,            0, None),
      ('Average Metabolic Rate',                 'average.dat',   2, lambda s: s.GetAveMerit()),
      ('Average Fitness',               'average.dat',   4, lambda s: s.GetAveFitness()),
      ('Average Gestation Time',        'average.dat',   3, lambda s: s.GetAveGestation()),
      ('Average Genome Length',                  'average.dat',   6, lambda s: s.GetAveSize()),
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

  def getValue(self, entry_index, stats):
    if entry_index:
      return self.m_entries[entry_index][3](stats)

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

  def export(self, path):
    "Export stats to a file"
    dialog_caption = "Export Analysis"
    fd = QFileDialog.getSaveFileName("", "CSV (Excel compatible) (*.csv)", None,
                                     "export as", dialog_caption)
    filename = str(fd)
    if (filename[-4:].lower() != ".csv"):
      filename += ".csv"

    checks = []
    # dictionary indexed by stat name so we can lookup stats to export
    stats = {}
    stat_cnt = 0
    for stat in self.m_entries:
      # Note: this relies on labeling dummy stats with None
      if stat[0] != "None":
        stats[stat[0]] = stat_cnt
        checks.append(stat[0])
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
    # TODO: more efficient loading
    for item in res:
      idx = stats[item]
      label1 = self.m_entries[idx][0]
      data[item] = self.load(path, self.m_entries[idx][1], 1,
                             self.m_entries[idx][2])

    out_file = open(filename, 'w')
    out_file.write("Update,%s\n" % (",".join(res)))
    # TODO: get it working with zip
    #print zip([data[elem][1][i] for elem in res])        
    num_updates = len(data[res[0]][0])
    for i in range(num_updates):
      out_file.write("%d,%s\n"
                     % (i, ",".join([str(data[elem][1][i]) for elem in res])))

    out_file.close()
