#Compiler and Linker
CXX     = g++
LD      = g++

#The Directories, Source, Includes, Objects, Binary and Resources
TARGET    = event
BUILDDIR  = build
TARGETDIR = bin

#Flags, Libraries and Includes
INCLIST  = $(shell find -name inc -type d)
INC      = $(addprefix -I, $(INCLIST))

CXXFLAGS  = -std=c++2a -Wall -Wextra -Werror -Wl,-z,relro,-z,now -g -pthread
CXXFLAGS += -fstack-protector-strong -pedantic-errors -fconcepts -Wno-parentheses
LDFLAGS   = $(addprefix -L, $(INCLIST))

#---------------------------------------------------------------------------------
#Main sources (src/ only — excludes tests)
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:.cpp=.o)))
VPATH   = src

#Test sources
TESTDIR      = tests
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS = $(addprefix $(BUILDDIR)/,$(notdir $(TEST_SOURCES:.cpp=.o)))
GTEST_LIBS   = -lgtest -lgtest_main -pthread
VPATH       += $(TESTDIR)

#---------------------------------------------------------------------------------

#Default Make
all: directories $(TARGETDIR)/$(TARGET)

#Remake
remake: clean all

#Create directories
directories:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)

#Linking
$(TARGETDIR)/$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

#Compiling
$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(INC) $(CXXFLAGS) -c $< -o $@

#Test
test: directories $(TEST_OBJECTS)
	$(LD) $(CXXFLAGS) $(TEST_OBJECTS) -o $(TARGETDIR)/test $(GTEST_LIBS)
	./$(TARGETDIR)/test

#Clean all
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(TARGETDIR)

#Non-File Targets
.PHONY: all remake clean directories test
