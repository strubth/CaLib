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

SRC           = $(wildcard $(S)/TC*.cxx) $(S)/Dict.cxx
INCD          = $(wildcard $(I)/TC*.h)
INC           = $(notdir $(INCD))
OBJD          = $(patsubst $(S)/%.cxx, $(O)/%.o, $(SRC))
OBJ           = $(notdir $(OBJD))

OSTYPE       := $(subst -,,$(shell uname))

ROOTGLIBS    := $(shell root-config --libs --glibs) -lEG -lFoam -lSpectrum
ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)

BIN_INSTALL_DIR = $(HOME)/$(B)

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

all:	begin $(LIB_CaLib) $(B)/calib_manager \
	end

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
	@$(CCCOMP) $(CXXFLAGS) $(ROOTGLIBS) $(CURDIR)/$(LIB_CaLib) -lncurses -o $(B)/calib_manager $(S)/MainCaLibManager.cxx

$(LIB_CaLib): $(OBJ)
	@echo
	@echo "Building libCaLib ..."
	@mkdir -p $(L)
	@rm -f $(L)/libCaLib.*
	@$(CCCOMP) $(LDFLAGS) $(ROOTGLIBS) $(SOFLAGS) $(OBJD) -o $(LIB_CaLib)
	@$(POST_LIB_BUILD)

$(S)/Dict.cxx: $(INC) $(I)/LinkDef.h
	@echo
	@echo "Creating CaLib dictionary ..."
	@rootcint -v -f $@ -c -I./$(I) -p $(INC) $(I)/LinkDef.h

%.o: %.cxx
	@echo "Compiling $(notdir $<) ..."
	@mkdir -p $(O)
	@$(CCCOMP) $(CXXFLAGS) -o $(O)/$@ -c $< 

docs:
	@echo "Creating HTML documentation ..."
	@rm -r -f htmldoc
	root -b -n -q $(S)/htmldoc.C
	@echo "Done."

install: $(B)/calib_manager
	@echo "Installing binaries in $(BIN_INSTALL_DIR)"
	@mkdir -p $(BIN_INSTALL_DIR)
	@cp $(B)/* $(BIN_INSTALL_DIR) 
	@echo "Done."

uninstall:
	@echo "Uninstalling CaLib applications"
	@rm -f $(BIN_INSTALL_DIR)/calib_manager
	@echo "Done."
	
clean:
	@echo "Cleaning CaLib distribution ..."
	rm -f $(S)/Dict.*
	rm -r -f $(L)
	rm -f -r $(O)
	rm -r -f $(B)
	rm -r -f htmldoc
	@echo "Done."
 
