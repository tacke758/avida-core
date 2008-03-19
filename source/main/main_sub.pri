
main {
  HEADERS += $$MAIN_HH/analyze.hh \
             $$MAIN_HH/analyze_util.hh \
             $$MAIN_HH/avida.hh \
             $$MAIN_HH/birth_chamber.hh \
             $$MAIN_HH/callback_util.hh \
             $$MAIN_HH/config.hh \
             $$MAIN_HH/environment.hh \
             $$MAIN_HH/fitness_matrix.hh \
             $$MAIN_HH/genebank.hh \
             $$MAIN_HH/genome.hh \
             $$MAIN_HH/genome_util.hh \
             $$MAIN_HH/genotype.hh \
	     $$MAIN_HH/inject_genotype.hh \
	     $$MAIN_HH/inject_genebank.hh \
             $$MAIN_HH/inst.hh \
             $$MAIN_HH/inst_lib.hh \
             $$MAIN_HH/inst_set.hh \
             $$MAIN_HH/inst_util.hh \
             $$MAIN_HH/landscape.hh \
             $$MAIN_HH/lineage.hh \
             $$MAIN_HH/lineage_control.hh \
             $$MAIN_HH/mutations.hh \
             $$MAIN_HH/organism.hh \
             $$MAIN_HH/phenotype.hh \
             $$MAIN_HH/pop_interface.hh \
             $$MAIN_HH/population.hh \
             $$MAIN_HH/population_cell.hh \
             $$MAIN_HH/reaction.hh \
             $$MAIN_HH/reaction_result.hh \
             $$MAIN_HH/resource.hh \
             $$MAIN_HH/resource_count.hh \
             $$MAIN_HH/species.hh \
             $$MAIN_HH/stats.hh \
             $$MAIN_HH/tasks.hh

  SOURCES += $$MAIN_CC/analyze.cc \
             $$MAIN_CC/analyze_util.cc \
             $$MAIN_CC/avida.cc \
             $$MAIN_CC/birth_chamber.cc \
             $$MAIN_CC/callback_util.cc \
             $$MAIN_CC/config.cc \
             $$MAIN_CC/environment.cc \
             $$MAIN_CC/fitness_matrix.cc \
             $$MAIN_CC/genebank.cc \
             $$MAIN_CC/genome.cc \
             $$MAIN_CC/genome_util.cc \
             $$MAIN_CC/genotype.cc \
             $$MAIN_CC/inst.cc \
             $$MAIN_CC/inst_set.cc \
             $$MAIN_CC/inst_util.cc \
	     $$MAIN_CC/inject_genebank.cc \
	     $$MAIN_CC/inject_genotype.cc \
             $$MAIN_CC/landscape.cc \
             $$MAIN_CC/lineage.cc \
             $$MAIN_CC/lineage_control.cc \
             $$MAIN_CC/mutations.cc \
             $$MAIN_CC/organism.cc \
             $$MAIN_CC/phenotype.cc \
             $$MAIN_CC/pop_interface.cc \
             $$MAIN_CC/population.cc \
             $$MAIN_CC/population_cell.cc \
             $$MAIN_CC/reaction.cc \
             $$MAIN_CC/reaction_result.cc \
             $$MAIN_CC/resource.cc \
             $$MAIN_CC/resource_count.cc \
             $$MAIN_CC/species.cc \
             $$MAIN_CC/stats.cc \
             $$MAIN_CC/tasks.cc
}
