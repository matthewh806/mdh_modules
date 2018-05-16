#include "NoteCalculator.hpp"
#include <iostream>
#include <map>

const float c_4 = 261.626f;
std::string notes[13] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A"};

// see note / freq table here: http://pages.mtu.edu/~suits/notefreqs.html

struct NoteCalculatorModule : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		VOLTAGE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
    
    double ratio = pow(2, 1.0/12.0);
    float voltage = 0.0f;

	NoteCalculatorModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void NoteCalculatorModule::step() {
	 if(voltage == inputs[VOLTAGE_INPUT].value)
         return;
    
    voltage = inputs[VOLTAGE_INPUT].value;
    std::cout << "voltage: " + std::to_string(voltage) << std::endl;

    // TODO: convert the voltage to the a note (lookup table).
    

    // TODO: display the note on a display.
    // This class uses a display so check there for inspiration:
    // https://github.com/sebastien-bouffier/Bidoo/blob/e82a658a88e3c7a27b6fbc97bf2b3def486a6e04/src/DTROY.cpp
}


struct MyModuleWidget : ModuleWidget {
	MyModuleWidget(NoteCalculatorModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));
        
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
		addInput(Port::create<PJ301MPort>(Vec(33, 186), Port::INPUT, module, NoteCalculatorModule::VOLTAGE_INPUT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelNoteCalculator = Model::create<NoteCalculatorModule, MyModuleWidget>("MDH Modules", "Note Calculator", "Note Calculator", UTILITY_TAG);
