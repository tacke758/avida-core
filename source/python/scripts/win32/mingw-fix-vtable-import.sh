#!/bin/sh
# Fixes errors with vtable imports when linking bindings

perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' config.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' fitness_matrix.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' genebank.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' genotype.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' genotype_control.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' inject_genebank.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' inject_genotype.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' inject_genotype_control.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' lineage.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' lineage_control.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' mx_code_array.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' my_code_array_less_than.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' phenotype.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' py_avida_driver.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' py_hardware_tracer.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' species_control.dir/build.make
perl -i -pe 's/ -shared/-enable-runtime-pseudo-reloc -shared/g' stats.dir/build.make
