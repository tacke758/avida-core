CPPFLAGS = @CPPFLAGS@

#
# following two lines added by kaben;
#
# these lines point the compiler to the build directory
#   $(build directory)/../event
# in addition to the source directory
#   $(srcdir)/../event
# so that
#   $(build directory)/../event/cPopulation_enums_auto.ci
# included by
#   $(srcdir)/../event/population_event_factory.hh
# will be found.
#
# don't know why it was needed.
#
AVIDA_EVENT_DIR = ../event
AVIDA_CPU_DIR = $(srcdir)/../cpu
INCLUDES = -I$(AVIDA_EVENT_DIR) -I$(AVIDA_CPU_DIR)
#

libmain_a_SOURCES = analyze.hh		analyze.cc		\
    analyze_util.hh		analyze_util.cc		\
    avida.hh		avida.cc		\
    birth_chamber.hh	birth_chamber.cc	\
    callback_util.hh	callback_util.cc	\
    config.hh		config.cc		\
    environment.hh		environment.cc		\
    fitness_matrix.hh	fitness_matrix.cc	\
    genebank.hh		genebank.cc		\
    genome.hh		genome.cc		\
    genome_util.hh		genome_util.cc		\
    genotype.hh		genotype.cc		\
    inst.hh			inst.cc			\
    inst_set.hh		inst_set.cc		\
    inst_lib.hh \
    inst_util.hh		inst_util.cc		\
    landscape.hh		landscape.cc		\
    lineage.hh		lineage.cc		\
    lineage_control.hh	lineage_control.cc 	\
    mutations.hh		mutations.cc		\
    organism.hh		organism.cc		\
    phenotype.hh		phenotype.cc		\
    population.hh		population.cc		\
    population_cell.hh	population_cell.cc	\
    pop_interface.hh	pop_interface.cc	\
    reaction.hh		reaction.cc		\
    reaction_result.hh	reaction_result.cc	\
    resource.hh		resource.cc		\
    resource_count.hh	resource_count.cc	\
    species.hh		species.cc		\
    stats.hh		stats.cc		\
    tasks.hh		tasks.cc		

noinst_LIBRARIES = libmain.a


primitive_SOURCES = primitive.cc 


primitive_LDADD = ./libmain.a			\
    ../cpu/libcpu.a   		\
    ../event/libevent.a          	\
    ./libmain.a			\
    ../tools/libtools.a \
    ../third-party/trio-1.9/libtrio.a

if BUILD_PRIMITIVE
bin_PROGRAMS = primitive
endif

EXTRA_DIST = main.pro main_sub.pri
