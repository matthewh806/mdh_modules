#include "mdh_modules.hpp"
#include "mdh_components.hpp"

struct GameOfLifeSequencerModule : Module {
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
    
    GameOfLifeSequencerModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
    void step() override;
};

void GameOfLifeSequencerModule::step() {
    
};

struct GameOfLifeSequencerWidget : ModuleWidget {
    GameOfLifeSequencerWidget(GameOfLifeSequencerModule *module): ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/ConwaySeq.svg")));
    }
};

Model *modelGameOfLifeSequencer = Model::create<GameOfLifeSequencerModule, GameOfLifeSequencerWidget>(MDH_MODULES, "Game Of Life Sequencer", "Game Of Life Sequencer", UTILITY_TAG);

