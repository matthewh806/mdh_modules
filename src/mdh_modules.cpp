#include "mdh_modules.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	// Add all Models defined throughout the plugin
	p->addModel(modelNoteCalculator);
    p->addModel(modelBpmCalculator);
    p->addModel(modelGameOfLifeSequencer);
    p->addModel(modelHyphae);
	p->addModel(modelFrequencyScope);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
