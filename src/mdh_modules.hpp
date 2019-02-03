#include "rack.hpp"

using namespace rack;

// Forward-declare the Plugin
extern Plugin *pluginInstance;

// Forward-declare each Model, defined in each module source file
extern Model *modelNoteCalculator;
extern Model *modelBpmCalculator;
extern Model *modelGameOfLifeSequencer;
extern Model *modelHyphae;
extern Model *modelFrequencyScope;
