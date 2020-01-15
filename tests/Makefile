.DEFAULT_GOAL := perft_tests
VPATH = ../

CXX = g++
CXXFLAGS = -I..

# for perft_tests
SRCPERFT = perft_tests.cpp move_validator.cpp position.cpp bitboard_lookup.cpp move_rules.cpp ortho_move_rules.cpp
# for position_tests
SRCPOST = position_tests.cpp position.cpp bitboard_lookup.cpp
SRCATOMPOST = atomic_position_tests.cpp atomic_position.cpp position.cpp atomic_capture_masks.cpp bitboard_lookup.cpp

SRCFILES = $(sort $(SRCPERFT) $(SRCPOST) $(SRCATOMPOST))
OBJFILES = $(SRCFILES:%.cpp=%.o)

perft_tests : $(SRCPERFT:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

position_tests : $(SRCPOST:%.cpp=%.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

atomic_position_tests : $(SRCATOMPOST:%.cpp=%.o)
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
