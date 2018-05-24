#include "mdh_modules.hpp"
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
    
    const char *note_display = "0";

	NoteCalculatorModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
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
    
    void draw(NVGcontext *vg) override {
        const char *note_display = module->note_display;
        
        nvgFontSize(vg, 24);
        nvgTextLetterSpacing(vg, 2.5);
        
        Vec textPos = Vec(4, 20);
        NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
        nvgFillColor(vg, textColor);
        nvgText(vg, textPos.x, textPos.y, note_display, NULL);
        nvgFillColor(vg, textColor);
    }
};

struct NoteCalculatorWidget : ModuleWidget {
	NoteCalculatorWidget(NoteCalculatorModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));
        
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        BubbleScrew *speechBubble = Widget::create<BubbleScrew>(Vec(2.0f, 160.0f));
        addChild(speechBubble);
        
        FaceSVG *faceSVG = Widget::create<FaceSVG>(Vec(2.0f, 240.0f));
        addChild(faceSVG);
        
        addInput(Port::create<PJ301MPort>(Vec(33, 90), Port::INPUT, module, NoteCalculatorModule::PITCH_INPUT));

        {
            NoteDisplay *display = new NoteDisplay();
            display->module = module;
            display->box.pos = Vec(14.0f, 160.0f);
            display->box.size = Vec(60.0f, 56.0f);
            addChild(display);
        }
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelNoteCalculator = Model::create<NoteCalculatorModule, NoteCalculatorWidget>("MDH Modules", "Note Calculator", "Note Calculator", UTILITY_TAG);
