#include "mdh_modules.hpp"
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
    
    rack::SchmittTrigger m_clockTrigger;
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
        addFrame(SVG::load(assetPlugin(plugin, "res/jigglypuff.svg")));
        sw->wrap();
        
        tw->box.size = sw->box.size;
        box.size = sw->box.size;
    }
};

struct BpmDancerWidget : ModuleWidget {
    BpmDancerWidget(BpmDancerModule *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/BpmDancer.svg")));
        
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        {
            BpmDancerAnimation *dancer = Widget::create<BpmDancerAnimation>(Vec(20.0f, 160.0f));
            dancer->frame=&(module->tick);
            addChild(dancer);
        }
        
        addInput(Port::create<PJ301MPort>(Vec(33, 280), Port::INPUT, module, BpmDancerModule::CLOCK_INPUT));
    };
};

Model *modelBpmCalculator = Model::create<BpmDancerModule,
    BpmDancerWidget>("MDH_MODULES", "Bpm Dancer", "Bpm Dancer", UTILITY_TAG);
