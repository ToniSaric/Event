#Compiler and Linker
CXX	= g++
LD	= g++

#The Directories, Source, Includes, Objects, Binary and Resources
TARGET 	 = event
BUILDDIR	= build
TARGETDIR 	= bin
SRCEXT		= cpp
OBJEXT		= o

#Flags, Libraries and Includes
INCLIST = $(shell find -name inc -type d)
INC		= $(addprefix -I, $(INCLIST))

#The Target Binary Program
CXXFLAGS = -std=c++2a -Wall -Wextra -Werror -Wl,-z,relro,-z,now -g -pthread
CXXFLAGS+= -fstack-protector-strong -pedantic-errors -fconcepts -Wno-parentheses
CXXFLAGS+=
LDFLAGS	 = $(addprefix -L, $(INCLIST))
#LDFLAGS	+= -Wl ,-rpath,$(LIBDIR)
#LDFLAGS	+= -lpthread -lboost_thread
#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

#List of all sources and objects
SOURCES = $(shell find -name *.cpp)
#SOURCES = $(shell find -path ./libs -prune -o -type f -name '*.cpp' -print)
OBJECTS = $(addprefix $(BUILDDIR)/,$(patsubst %.cpp,%.o, $(notdir $(SOURCES))))
VPATH 	= $(dir $(SOURCES))

#Default Make
all: directories $(TARGET)

#Remake
remake: clean all

#Create directories
directories:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)

#Linking
$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGETDIR)/$(TARGET) $(LIBS)

#Compiling
$(BUILDDIR)/%.o : %.cpp
	$(CXX) $(INC) $(CXXFLAGS) -c $< -o $@

#Clean all
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(TARGETDIR)

#TO-DO: Create Test rule:

#Non-File Targets
.PHONY: all remake clean directories