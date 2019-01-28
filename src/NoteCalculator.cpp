#include "mdh_modules.hpp"
#include "componentlibrary.hpp"
#include "mdh_components.hpp"
#include <iostream>
#include <array>

//const float c_4 = 261.626f;
static const char *notes_sharp[12] = {"c", "c#", "d", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

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
    
    const char *note_display = "";

	NoteCalculatorModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    
	void step() override;
};


void NoteCalculatorModule::step() {
	 if(pitch == inputs[PITCH_INPUT].value)
         return;
    
    pitch = inputs[PITCH_INPUT].value;
    float o = std::floor(pitch);
    int s = std::floor((pitch - o) * 12.f);
    
//    float out = o + s / 12.f;
    
    note_display = notes_sharp[s];
}

struct NoteDisplay : TransparentWidget {
    NoteCalculatorModule *module;
    std::shared_ptr<Font> font;
    
    NoteDisplay() {
        font = Font::load(asset::plugin(plugin, "res/fonts/Segment14.ttf"));
    };
    
    void draw(NVGcontext *vg) override {
        if(!module) { return; }

        const char *note_display = module->note_display;
        
        if(!note_display)
            note_display = "";
        
        nvgFontSize(vg, 22);
        nvgFontFaceId(vg, font->handle);
        nvgTextLetterSpacing(vg, 2.0);
        
        Vec textPos = Vec(4, 20);
        NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
        nvgFillColor(vg, textColor);
        nvgText(vg, textPos.x, textPos.y, note_display, NULL);
        nvgFillColor(vg, textColor);
    }
};

struct NoteCalculatorWidget : ModuleWidget {
	NoteCalculatorWidget(NoteCalculatorModule *module) {
        setModule(module);

		setPanel(SVG::load(asset::plugin(plugin, "res/NoteCalculator.svg")));
        
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // {
        //     NoteDisplay *display = new NoteDisplay();
        //     display->module = module;
        //     display->box.pos = Vec(30.0f, 120.0f);
        //     display->box.size = Vec(40.0f, 40.0f);
        //     addChild(display);
        // }
        
        addInput(createInput<PJ301MPort>(Vec(33, 280), module, NoteCalculatorModule::PITCH_INPUT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelNoteCalculator = createModel<NoteCalculatorModule, NoteCalculatorWidget>("NoteCalculator");
