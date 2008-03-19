### Compile and Link Command & Options #######################################

CXX	= g++
#FLAGS	= -g -Wall -pedantic -DDEBUG_ASSERTIONS -DDEBUG_OUTPUT
#FLAGS	= -pg -Wall -pedantic
FLAGS	= -Wall -pedantic -O3 -ffast-math #-Winline
#         -pedantic -W -Wtraditional -Wshadow -Wbad-function-cast
#         -Wpointer-arith -Wcast-align -Winline 
#         -funroll-loops -ffast-math

# Include directories [May need to adjust.  See your system administrator]
#INCDIR	=  -I/usr/include/g++/
INCDIR   = 

# Libraries to link
LDLIBS   = 

### Shouldn't need to change below ###########################################

# Compile flags
CXXFLAGS= $(FLAGS)

# Flags durring linking
LDFLAGS	= $(CXXFLAGS) #-pg


### Different Builds #########################################################

# executable make
TARGET		= task_event_gen

default:  $(TARGET)

### List Source Files in the Project #########################################
SRC		= ../../tools/file.cc  ../../tools/string.cc  ../../tools/string_list.cc  ../../tools/tools.cc  ../../tools/random.cc  task_event_gen.cc

### List Object Files (for each source file there is a object file) ###########
OBJ	 	= $(SRC:.cc=.o)


### General Make Commands #####################################################

CCC		= $(CXX)		# Compatibility

$(TARGET):	$(OBJ)
		### LINKING ###
		$(CXX) $(LDFLAGS) $(INCDIR) $(OBJ)  $(LDLIBS) -o $(TARGET)
		### FINISHED MAKING "$(TARGET)" ###

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $*.o

clean:
		### REMOVING OBJECT FILES AND EXECUTEABLE ###
		rm -f core $(TARGET) $(OBJ)

new:		clean $(TARGET)

install:
	configure
	$(MAKE) new


### Dependency Generation #####################################################

#depend: $(SRC)
#	makedepend -- $(CXXFLAGS) -- $(INCDIR) $(SRC)

depend:  local_depend

local_depend: $(SRC)
	makedepend -Y -- $(CXXFLAGS) -- $(INCDIR) $(SRC)

### Release Packageing ########################################################
# DO NOT DELETE
