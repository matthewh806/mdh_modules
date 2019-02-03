#include "mdh_modules.hpp"
#include "component.hpp"
#include "dsp/fft.hpp"

static const int BUFFER_SIZE = 512;

struct FrequencyScopeModule : Module {
    enum ParamIds {
        NUM_PARAMS
    };

    enum InputIds {
        INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    /*
        These values are not used for now.
    */
    float buffer[BUFFER_SIZE] = {};
    int bufferIndex = 0;

    float inputValue = 0;

    int step_count = 0;

    rack::dsp::RealFFT rfft;

    FrequencyScopeModule() : rfft(BUFFER_SIZE) {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    
    void step() override;
};

void FrequencyScopeModule::step() {
    step_count++;

    inputValue = inputs[INPUT].value;
};

struct ScopeDisplay : TransparentWidget {
    FrequencyScopeModule *module;

    std::shared_ptr<Font> font;

    ScopeDisplay() {
        font = Font::load(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
    };

    void draw(const DrawContext &ctx) override {
        if(!module) return;

        //bg
        nvgFillColor(ctx.vg, nvgRGB(20, 30, 33));
        nvgBeginPath(ctx.vg);
        nvgRect(ctx.vg, 0, 0, box.size.x, box.size.y);
        nvgFill(ctx.vg);

        nvgFontSize(ctx.vg, 40);
        nvgFontFaceId(ctx.vg, font->handle);
        nvgTextLetterSpacing(ctx.vg, 2.0);

        Vec textPos = Vec(20, 120);
        NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
        nvgFillColor(ctx.vg, textColor);
        nvgText(ctx.vg, textPos.x, textPos.y, std::to_string(module->inputValue).c_str(), NULL);
        // nvgFillColor()
    }
};

struct FrequencyScopeWidget : ModuleWidget {
    FrequencyScopeWidget(FrequencyScopeModule *module) {
        setModule(module);
        setPanel(SVG::load(asset::plugin(pluginInstance, "res/ConwaySeq.svg")));

        {
            ScopeDisplay *display = new ScopeDisplay();
            display->module = module;
            display->box.pos = Vec(0, 44);
            display->box.size = Vec(box.size.x, 240);
            addChild(display);
        }

        addInput(createInput<PJ301MPort>(Vec(20, 300), module, FrequencyScopeModule::INPUT));
    }
};

Model *modelFrequencyScope = createModel<FrequencyScopeModule, FrequencyScopeWidget>("FrequencyScope");