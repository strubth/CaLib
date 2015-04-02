#####################################################################
##                                                                 ##
## CaLib Makefile                                                  ##
##                                                                 ##
#####################################################################

# --------------------------- System and ROOT variables ---------------------------

S             = src
I             = include
O             = obj
L             = lib
B             = bin

SRC           = $(wildcard $(S)/TC*.cxx) $(S)/G__CaLib.cxx
INCD          = $(wildcard $(I)/TC*.h)
INC           = $(notdir $(INCD))
OBJD          = $(patsubst $(S)/%.cxx, $(O)/%.o, $(SRC))
OBJ           = $(notdir $(OBJD))

OSTYPE       := $(subst -,,$(shell uname))

ROOTGLIBS    := $(shell root-config --libs --glibs) -lEG -lFoam -lSpectrum
ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)

DEP_LIB      := libHist.so libGui.so libSpectrum.so

ifeq ($(shell root-config --has-mysql),yes)
    ROOTGLIBS := $(ROOTGLIBS) -lRMySQL
    DEP_LIB   := $(DEP_LIB) libRMySQL.so
endif
ifeq ($(shell root-config --has-sqlite),yes)
    ROOTGLIBS := $(ROOTGLIBS) -lSQLite
    DEP_LIB   := $(DEP_LIB) libSQLite.so
endif

BIN_INSTALL_DIR = $(HOME)/$(B)
LINKLIB         = -L$(CURDIR)/$(L) -lCaLib

vpath %.cxx $(S)
vpath %.h  $(I)
vpath %.o  $(O)

# ------------------------ Architecture dependent settings ------------------------

ifeq ($(OSTYPE),Darwin)
	LIB_CaLib = $(L)/libCaLib.dylib
	SOFLAGS = -dynamiclib -single_module -undefined dynamic_lookup -install_name $(CURDIR)/$(LIB_CaLib)
	POST_LIB_BUILD = @ln $(L)/libCaLib.dylib $(L)/libCaLib.so
endif

ifeq ($(OSTYPE),Linux)
	LIB_CaLib = $(L)/libCaLib.so
	SOFLAGS = -shared
	POST_LIB_BUILD = 
endif

# -------------------------------- Compile options --------------------------------

CCCOMP      = g++
CXXFLAGS    = -g -O2 -Wall -fPIC $(ROOTCFLAGS) -I./$(I)
LDFLAGS     = -g -O2 $(ROOTLDFLAGS)

# ------------------------------------ targets ------------------------------------

all:	begin $(LIB_CaLib) $(L)/libCaLib.rootmap $(B)/calib_manager end

begin:
	@echo
	@echo "-> Building CaLib on a $(OSTYPE) system"
	@echo

end:
	@echo
	@echo "-> Finished!"
	@echo

$(B)/calib_manager: $(LIB_CaLib) $(S)/MainCaLibManager.cxx
	@echo "Building the CaLib Manager"
	@mkdir -p $(B)
	@$(CCCOMP) $(CXXFLAGS) $(ROOTGLIBS) $(LINKLIB) -lncurses -o $(B)/calib_manager $(S)/MainCaLibManager.cxx

$(LIB_CaLib): $(OBJ)
	@echo
	@echo "Building libCaLib"
	@mkdir -p $(L)
	@rm -f $(L)/libCaLib.*
	@$(CCCOMP) $(LDFLAGS) $(SOFLAGS) $(OBJD) -o $(LIB_CaLib)
	@$(POST_LIB_BUILD)

$(L)/libCaLib.rootmap: $(LIB_CaLib)
	@echo "Creating ROOT map"
	@rlibmap -o $(L)/libCaLib.rootmap -l $(LIB_CaLib) -d $(DEP_LIB) -c $(I)/LinkDef.h

$(S)/G__CaLib.cxx: $(INC) $(I)/LinkDef.h
	@echo
	@echo "Creating CaLib dictionary"
	@rootcint -v -f $@ -c -I./$(I) -p $(INC) $(I)/LinkDef.h

%.o: %.cxx
	@echo "Compiling $(notdir $<)"
	@mkdir -p $(O)
	@$(CCCOMP) $(CXXFLAGS) -o $(O)/$@ -c $< 

docs:
	@echo "Creating HTML documentation"
	@rm -r -f htmldoc
	root -b -n -q $(S)/htmldoc.C
	@echo "Done."

install: $(B)/calib_manager
	@echo "Installing binaries in $(BIN_INSTALL_DIR)"
	@install -D $(B)/calib_manager $(BIN_INSTALL_DIR)/calib_manager
	@echo "Done."

uninstall:
	@echo "Uninstalling CaLib applications"
	@rm -f $(BIN_INSTALL_DIR)/calib_manager
	@echo "Done."
	
clean:
	@echo "Cleaning CaLib distribution"
	rm -f $(S)/G__*
	rm -r -f $(L)
	rm -f -r $(O)
	rm -r -f $(B)
	rm -r -f htmldoc
	@echo "Done."
 
