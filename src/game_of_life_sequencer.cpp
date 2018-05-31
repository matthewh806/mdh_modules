#include "mdh_modules.hpp"
#include "mdh_components.hpp"

#include "dsp/digital.hpp"

#include <iostream>
#include <cstdlib>

#define ROWS 32
#define COLUMNS 32
#define CELLS 1024
#define HW 8

struct GameOfLifeSequencerModule : Module {
    enum ParamIds {
        CLEAR_PARAM,
        RANDOMIZE_PARAM,
        STEP_LIFE_SWITCH_PARAM,
        LIFE_SPEED_KNOB_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        EXTERNAL_CLOCK_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };
    
    bool *cells = new bool[CELLS];
    SchmittTrigger clearTrigger, randomizeTrigger;
    SchmittTrigger clockTrigger;
    
    GameOfLifeSequencerModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setRandomState();
    }
    
    ~GameOfLifeSequencerModule() {
        delete []cells;
    }
    
    void step() override {
        //reset
        if(clearTrigger.process(params[CLEAR_PARAM].value)) {
            clearCells();
        }
        
        //randomize
        if(randomizeTrigger.process(params[RANDOMIZE_PARAM].value)){
            setRandomState();
        }
        
        /* TODO
            Currently life is being stepped at the same rate as the external clock input.
         It would be nice to have control over this to allow it to be adjusted to be faster / slower than the clock :)
         */
        if(inputs[EXTERNAL_CLOCK_INPUT].active && clockTrigger.process(inputs[EXTERNAL_CLOCK_INPUT].value)) {
            if(params[STEP_LIFE_SWITCH_PARAM].value) {
                stepLife();
            }
        }
    };
    
    void stepLife() {
        bool *newCells = new bool[CELLS];
        
        for(int x=0; x < COLUMNS; x++) {
            for(int y=0; y<ROWS; y++) {
                int idx = getCellIndexFromXY(x, y);
                int neighborsCount = getNeighborCount(x, y);
                newCells[idx] = getNewState(cells[idx], neighborsCount);
            }
        }
        
        cells = newCells;
    }
    
    void setRandomState() {
        clearCells();
        
        int i = 0;
        while(i < 256) {
            int index = rand() % 1024;
            setCellState(index, true);
            i++;
        }
    };
    
    void clearCells() {
        for(int i=0; i<CELLS; i++)
            cells[i] = false;
    };
    
    int getNeighborCount(int x, int y) {
        int count = 0;
        
        int startX = (x - 1 >= 0) ? x-1 : 0;
        int startY = (y - 1 >= 0) ? y-1 : 0;
        int endX = (x + 1 < COLUMNS) ? x+1 : x;
        int endY = (y + 1 < ROWS) ? y + 1: y;
        
        for (int i=startX; i<=endX; i++) {
            for(int j=startY; j<=endY; j++) {
                if(i == x && j == y) { continue; }
                count += int(cells[getCellIndexFromXY(i, j)]);
            }
        }
        
        return count;
    }
    
    int getNewState(bool alive, int n) {
        if(alive && (n < 2 || n > 3)) { return false; }
        if(!alive && n==3) { return true; }
        
        return alive;
    }
    
    int getCellIndexFromXY(int x, int y) {
        return x + y * ROWS;
    };
    
    // TODO: Make this private
    void setCellState(int index, bool on) {
        cells[index] = on;
    }
    
    void setCellState(int x, int y, bool on) {
        // TODO: Invert this conditional and add a log if it is out of range
        if(isInRange(x, y)) {
            cells[getCellIndexFromXY(x, y)] = on;
        }
    };
    
    void setCellStateByDisplayPos(float x, float y, bool on) {
        setCellState(int(x / HW), int(y / HW), on);
    };
    
    bool isInRange(int x, int y) {
        return x >= 0 && x <= COLUMNS && y >= 0 && y <= ROWS;
    };
};

struct ConwaySeqDisplay : VirtualWidget {
    GameOfLifeSequencerModule * module;
    
    ConwaySeqDisplay() {};
    
    void draw(NVGcontext *vg) override {
        // bg
        nvgFillColor(vg, nvgRGB(20, 30, 33));
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFill(vg);
        
        // grid
        nvgStrokeColor(vg, nvgRGB(60, 70, 73));
        for(int i=0; i < COLUMNS; i++) {
            nvgStrokeWidth(vg, (i % 4 == 0) ? 2 : 1);
            nvgBeginPath(vg);
            nvgMoveTo(vg, i * HW, 0);
            nvgLineTo(vg, i * HW, box.size.y);
            nvgStroke(vg);
        }
        for(int i=0; i < ROWS; i++) {
            nvgStrokeWidth(vg, (i % 4 == 0) ? 2 : 1);
            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, i * HW);
            nvgLineTo(vg, box.size.x, i*HW);
            nvgStroke(vg);
        }
        
        // cells
        nvgFillColor(vg, nvgRGB(25, 150, 252)); // blue
        for(int i=0; i<CELLS; i++) {
            // TODO: Learn how to invert and do it...
            if(module->cells[i]) {
                int y = i / ROWS;
                int x = i % COLUMNS;
                
                nvgBeginPath(vg);
                nvgRect(vg, x * HW, y * HW, HW, HW);
                nvgFill(vg);
            }
        }
    };
};

struct GameOfLifeSequencerWidget : ModuleWidget {
    GameOfLifeSequencerWidget(GameOfLifeSequencerModule *module): ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/ConwaySeq.svg")));
        
        addInput(Port::create<PJ301MPort>(Vec(50, 10), Port::INPUT, module, GameOfLifeSequencerModule::EXTERNAL_CLOCK_INPUT));
        addParam(ParamWidget::create<LEDButton>(Vec(50, 320), module, GameOfLifeSequencerModule::CLEAR_PARAM, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<LEDButton>(Vec(100, 320), module, GameOfLifeSequencerModule::RANDOMIZE_PARAM, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<CKSS>(Vec(150, 320), module, GameOfLifeSequencerModule::STEP_LIFE_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(200, 320), module, GameOfLifeSequencerModule::LIFE_SPEED_KNOB_PARAM, 1.0f, 1000.0f, 100.0f));
        
        {
            ConwaySeqDisplay *display = new ConwaySeqDisplay();
            display->module = module;
            display->box.pos = Vec(50, 50);
            display->box.size = Vec(256, 256);
            
            addChild(display);
        }
    }
};

Model *modelGameOfLifeSequencer = Model::create<GameOfLifeSequencerModule, GameOfLifeSequencerWidget>(MDH_MODULES, "Game Of Life Sequencer", "Game Of Life Sequencer", UTILITY_TAG);

