TARGET = wkserial


INC_SELETED := serial.hh compatible.hh
INC_LINK := linkdef.h



TARGET_LIB = lib$(TARGET).so
TARGET_DICT := lib$(TARGET).cc

SRC_DIR := ./
INC_DIR := ./
OBJ_DIR := ./
SHA_DIR := ./


SRCS         := $(wildcard $(SRC_DIR)*.cc)
SRCS         := $(notdir $(SRCS))
SRCS         += $(TARGET_DICT)
OBJS         := $(patsubst %.cc,%.o,$(SRCS))

# DEBUG        := -g
CXX          := g++
LD           := g++
CXXFLAGS     := $(DEBUG) -Wall -fPIC -std=c++11 -DUSE_HSIO -D_REENTRANT -D_GNU_SOURCE
LDFLAGS      := $(DEBUG)
SOFLAGS      := -shared
# SYSLIBS      := -lboost_thread -lpcap



ROOTCINT     := rootcint
#ROOTCINT     := rootcling
ROOTCONFIG   := root-config
ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --libs)
ROOTGLIBS    := $(shell $(ROOTCONFIG) --glibs)
HASTHREAD    := $(shell $(ROOTCONFIG) --has-thread)

LIBS         := $(ROOTLIBS) $(SYSLIBS)
GLIBS        := $(ROOTGLIBS) $(SYSLIBS)

CXXFLAGS     += $(ROOTCFLAGS) -I$(SRC_DIR) -I$(INC_DIR)
LDFLAGS      += $(ROOTLDFLAGS)

vpath %.cpp $(SRC_DIR)
vpath %.cc $(SRC_DIR)
vpath %.hh $(INC_DIR)
vpath %.c $(SRC_DIR)
vpath %.h $(INC_DIR)
vpath %.o $(OBJ_DIR)

.PHONY: clean

$(TARGET_LIB): $(OBJS)
	$(LD) $(SOFLAGS) $(LDFLAGS) $(addprefix $(OBJ_DIR),$(notdir $^)) $(LIBS) -o $@


$(TARGET_DICT):  $(INC_SELETED) $(INC_LINK)
	$(ROOTCINT)  -f $@ -c $^


.SUFFIXES:
%.o:%.cc
	$(CXX)  $(CXXFLAGS) -c $< -o $(OBJ_DIR)$@

clean:
	@ rm -f lib$(TARGET)*
	@ rm -f ./*.o
	@ rm -f ./*~

