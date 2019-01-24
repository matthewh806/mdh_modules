#include "mdh_modules.hpp"
#include "componentlibrary.hpp"
#include "mdh_components.hpp"
#include <dsp/digital.hpp>
#include <iostream>

using namespace rack;

struct HyphaeModule : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    
    enum InputIds {
        CLOCK_INPUT,
        NUM_INPUTS
    };
    
    enum outputIds {
        NUM_OUTPUTS
    };
    
    enum lightIds {
        NUM_LIGHTS
    };
    
    HyphaeModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
    rack::dsp::SchmittTrigger m_clockTrigger;
    
    bool clockTick;
    bool gateIn;
    
    void step() override {
        auto const& inputClock = inputs.at(CLOCK_INPUT);
        clockTick = inputClock.active && m_clockTrigger.process(inputClock.value);
        gateIn = m_clockTrigger.isHigh();
    }
};

struct HyphaeDrawingWidget : TransparentWidget {
    HyphaeModule *module;
    
    NVGcolor color;
    
    HyphaeDrawingWidget() {}
    
    void draw(NVGcontext *vg) override {
        if(module->gateIn) {
            nvgBeginPath(vg);
            nvgCircle(vg, box.size.x / 2, box.size.y / 2, 100);
            nvgFillColor(vg, color);
            nvgFill(vg);
        } else {
            color = getRandomPastelColor();
        }
        
        Widget::draw(vg);
    }
    
    NVGcolor getRandomPastelColor() {
        std::uint8_t colorBytes[3] = {};
        colorBytes[0] = (std::uint8_t)(rand() % 128 + 1 + 127);
        colorBytes[1] = (std::uint8_t)(rand() % 128 + 1 + 127);
        colorBytes[2] = (std::uint8_t)(rand() % 128 + 1 + 127);
        
        return nvgRGBA(colorBytes[0], colorBytes[1], colorBytes[2], 255);
    }
};

struct HyphaeWidget : ModuleWidget {
    HyphaeWidget(HyphaeModule *module) : ModuleWidget(module) {
        setPanel(SVG::load(asset::plugin(plugin, "res/ConwaySeq.svg")));
        
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        {
            HyphaeDrawingWidget *display = new HyphaeDrawingWidget();
            display->module = module;
            display->box.pos = Vec(50, 50);
            display->box.size = Vec(256, 256);
            
            addChild(display);
        }

        addInput(createInput<PJ301MPort>(Vec(33, 280), module, HyphaeModule::CLOCK_INPUT));
    };
};

Model *modelHyphae = createModel<HyphaeModule, HyphaeWidget>("Hyphae");

