#####################################################################
##                                                                 ##
##                                                                 ##
##                                                                 ##
#####################################################################

# --------------------------- System and ROOT variables ---------------------------

S             = src
I             = include
O             = obj
L             = lib
B             = bin

SRC           = $(wildcard $(S)/i*.cxx) $(S)/Dict.cxx
INCD          = $(wildcard $(I)/i*.hh)
INC           = $(notdir $(INCD))
OBJD          = $(patsubst $(S)/%.cxx, $(O)/%.o, $(SRC))
OBJ           = $(notdir $(OBJD))

OSTYPE       := $(subst -,,$(shell uname))

ROOTGLIBS    := $(shell root-config --libs --glibs) -lEG -lFoam
ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)

BIN_INSTALL_DIR = $(HOME)/$(B)

vpath %.cxx $(S)
vpath %.hh  $(I)
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
CXXFLAGS    = -g -O3 -Wall -fPIC $(ROOTCFLAGS) -I./$(I)
LDFLAGS     = -g -O3 $(ROOTLDFLAGS)

# ------------------------------------ targets ------------------------------------

all:	begin $(LIB_CaLib) \
	end

begin:
	@echo
	@echo "-> Building CaLib on a $(OSTYPE) system"
	@echo

end:
	@echo
	@echo "-> Finished!"
	@echo

$(LIB_CaLib): $(OBJ)
	@echo
	@echo "Building libCaLib ..."
	@mkdir -p $(L)
	@rm -f $(L)/libCaLib.*
	@$(CCCOMP) $(LDFLAGS) $(ROOTGLIBS) $(SOFLAGS) $(OBJD) -o $(LIB_CaLib)
	@$(POST_LIB_BUILD)

$(S)/Dict.cxx: $(INC) $(I)/LinkDef.hh 
	@echo
	@echo "Creating CaLib dictionary ..."
	@rootcint -v -f $@ -c -I./$(I) -p $(INC) $(I)/LinkDef.hh

%.o: %.cxx
	@echo "Compiling $(notdir $<) ..."
	@mkdir -p $(O)
	@$(CCCOMP) $(CXXFLAGS) -o $(O)/$@ -c $< 

docs:
	@echo "Creating HTML documentation ..."
	@rm -r -f htmldoc
	root -b -n -q $(S)/htmldoc.C
	@echo "Done."

clean:
	@echo "Cleaning CaLib distribution ..."
	rm -f $(S)/Dict.*
	rm -r -f $(L)
	rm -f -r $(O)
	rm -r -f $(B)
	rm -r -f htmldoc
	@echo "Done."
 
