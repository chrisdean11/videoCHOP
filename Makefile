# Makefile example from https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html#C


#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'displayimage'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -Wall -g -std=c++11

# define any directories containing header files other than /usr/include
#
INCLUDES = 	-Iinclude  \
			-I/usr/local/include/opencv4

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = 	-L../lib  \
			-L/usr/local/lib #opencv4 lib folder

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so: -lmylib -lm
LIBS = 	-lopencv_core 		\
		-lopencv_highgui	\
		-lopencv_imgproc	\
		-lopencv_imgcodecs	\
		-lopencv_videoio	\
		-lopencv_tracking	\
		-lopencv_features2d
		# locate libopencv_ to see all of these, and what folders they reside in

# define the C source files
SRCS = 	src/Main.cpp		\
		src/VideoCHOP.cpp 	\
		src/Log.cpp       	\
		src/ImageMatch.cpp

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = videoCHOP

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo  Compilation completed successfully

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it