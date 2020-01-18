.DEFAULT_GOAL := perft_tests
VPATH = ../

CXX = g++
CXXFLAGS = -I..

# for perft_tests
SRC_PERFT = perft_tests.cpp move_validator.cpp position.cpp ortho_position.cpp atomic_position.cpp bitboard_lookup.cpp move_rules.cpp ortho_move_rules.cpp atomic_move_rules.cpp atomic_capture_masks.cpp
# for position_tests
SRC_POST = position_tests.cpp position.cpp ortho_position.cpp bitboard_lookup.cpp
# for atomic_position_tests
SRC_ATOM_POST = atomic_position_tests.cpp atomic_position.cpp position.cpp atomic_capture_masks.cpp bitboard_lookup.cpp

SRC_ATOM_PERFT_MAKER = atomic_perft_maker.cpp move_validator.cpp position.cpp ortho_position.cpp atomic_position.cpp bitboard_lookup.cpp move_rules.cpp ortho_move_rules.cpp atomic_move_rules.cpp atomic_capture_masks.cpp
SRC_PERFTER = perfter.cpp move_validator.cpp position.cpp atomic_position.cpp bitboard_lookup.cpp move_rules.cpp ortho_move_rules.cpp atomic_move_rules.cpp atomic_capture_masks.cpp

SRCFILES = $(sort $(SRC_PERFT) $(SRC_POST) $(SRC_ATOM_POST) $(SRC_ATOM_PERFT_MAKER) $(SRC_PERFTER))
OBJFILES = $(SRCFILES:%.cpp=%.o)

perft_tests : $(SRC_PERFT:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

position_tests : $(SRC_POST:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

atomic_position_tests : $(SRC_ATOM_POST:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

atomic_perft_maker : $(SRC_ATOM_PERFT_MAKER:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^
    
perfter : $(SRC_PERFTER:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Auto-dependency generation
DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c 
%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.cpp) $<
$(DEPDIR): ; @mkdir -p $@
DEPFILES := $(SRCFILES:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):
-include $(wildcard $(DEPFILES))


.PHONY : clean
clean :
	rm -r $(wildcard %.exe) $(OBJFILES) $(DEPDIR)
