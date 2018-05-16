#include "NoteCalculator.hpp"
#include <iostream>
#include <array>

const float c_4 = 261.626f;
const std::array<char, 2> notes_sharp[12] = {
    {'c', '\0'},
    {'c', '#'},
    {'d', '\0'},
    {'d', '#'},
    {'e', '\0'},
    {'f', '\0'},
    {'f', '#'},
    {'g', '\0'},
    {'g', '#'},
    {'a', '\0'},
    {'a', '#'},
    {'b', '\0'},
};


// see note / freq table here: http://pages.mtu.edu/~suits/notefreqs.html

struct NoteCalculatorModule : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
        PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
    
    double ratio = 1.0 / 12.0;
    float pitch = 0.0f;

	NoteCalculatorModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void NoteCalculatorModule::step() {
	 if(pitch == inputs[PITCH_INPUT].value)
         return;
    
    pitch = inputs[PITCH_INPUT].value;
    float o = std::floor(pitch);
    float s = std::floor((pitch - o) * 12.f);
    
    float out = o + s / 12.f;
    
    const char note = notes_sharp[(int)s][0];
    
    std::cout << "freq: " + std::to_string(out);
    std::cout << ", note: ";
    std::cout << note << std::endl;

    // TODO: display the note on a display.vco
    // This class uses a display so check there for inspiration:
    // https://github.com/luckyxxl/vcv_luckyxxl/blob/master/src/Quantize.cpp
}


struct MyModuleWidget : ModuleWidget {
	MyModuleWidget(NoteCalculatorModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));
        
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        addInput(Port::create<PJ301MPort>(Vec(33, 90), Port::INPUT, module, NoteCalculatorModule::PITCH_INPUT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelNoteCalculator = Model::create<NoteCalculatorModule, MyModuleWidget>("MDH Modules", "Note Calculator", "Note Calculator", UTILITY_TAG);
