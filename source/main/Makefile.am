CPPFLAGS = @CPPFLAGS@ -DVIEW_$(VIEW)
LDFLAGS = -lm $(VIEW_LIBS)

avida_SOURCES = merit.cc merit.hh		\
		stats.cc stats.hh		\
                inst_lib.cc inst_lib.hh		\
		phenotype.cc phenotype.hh	\
                code_array.cc code_array.hh	\
		genotype.cc genotype.hh		\
                species.cc species.hh		\
		genebank.cc genebank.hh		\
                slice.cc slice.hh		\
		population.cc population.hh	\
                avida.cc avida.hh		\
		tasks.cc tasks.hh		\
		geneology.cc geneology.hh	\
		landscape.cc landscape.hh	\
		task_buffer.cc task_buffer.hh	\
		inst.cc	inst.hh			\
                config.cc config.hh		\
		classes.hh

avida_LDADD =  ../cpu/libcpu.a   \
		../event/libevent.a          \
		../viewers/libview.a  ../tools/libtools.a


bin_PROGRAMS = avida




