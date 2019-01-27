#include "mdh_modules.hpp"

Plugin *plugin;

void init(Plugin *p) {
	plugin = p;

	// Add all Models defined throughout the plugin
	p->addModel(modelNoteCalculator);
    p->addModel(modelBpmCalculator);
    p->addModel(modelGameOfLifeSequencer);
    p->addModel(modelHyphae);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
