#include "mdh_modules.hpp"
#include "componentlibrary.hpp"
#include "mdh_components.hpp"
#include <dsp/digital.hpp>

struct BpmDancerModule : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    
    enum InputIds {
        CLOCK_INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds {
        NUM_OUTPUTS
    };
    
    enum LightIds {
        NUM_LIGHTS
    };
    
    BpmDancerModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
    rack::dsp::SchmittTrigger m_clockTrigger;
    void step() override;
    
    int tick = 0;
};

void BpmDancerModule::step() {
    auto const& inputClock = inputs.at(CLOCK_INPUT);
    bool const clockTick = inputClock.active && m_clockTrigger.process(inputClock.value);
    
    if(clockTick) {
        tick++;
    }
};

struct BpmDancerAnimation : SVGAnimation {
    BpmDancerAnimation() : SVGAnimation(Vec(0.33f, 0.33f)) {
        // TODO: Randomize initial animation character...
        addFrame(SVG::load(asset::plugin(plugin, "res/jigglypuff.svg")));
        sw->wrap();
        
        tw->box.size = sw->box.size;
        box.size = sw->box.size;
    }
};

struct BpmDancerWidget : ModuleWidget {
    BpmDancerWidget(BpmDancerModule *module) : ModuleWidget(module) {
        setPanel(SVG::load(asset::plugin(plugin, "res/BpmDancer.svg")));
        
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        {
            BpmDancerAnimation *dancer = createWidget<BpmDancerAnimation>(Vec(20.0f, 160.0f));
            dancer->frame=&(module->tick);
            addChild(dancer);
        }
        
        addInput(createInput<PJ301MPort>(Vec(33, 280), module, BpmDancerModule::CLOCK_INPUT));
    };
};

Model *modelBpmCalculator = createModel<BpmDancerModule, BpmDancerWidget>("Bpm Dancer");
