#include "rack.hpp"

using namespace rack;

const std::string MDH_MODULES = "MDH Modules";

// Forward-declare the Plugin
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelNoteCalculator;
extern Model *modelBpmCalculator;
extern Model *modelGameOfLifeSequencer;
