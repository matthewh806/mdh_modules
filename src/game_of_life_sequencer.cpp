#include "mdh_modules.hpp"
#include "mdh_components.hpp"

#include <iostream>
#include <cstdlib>

#define ROWS 32
#define COLUMNS 32
#define CELLS 1024
#define HW 8

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
    
    bool *cells = new bool[CELLS];
    
    GameOfLifeSequencerModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setInitialState();
    }
    
    ~GameOfLifeSequencerModule() {
        delete []cells;
    }
    
    void step() override {
        bool *newCells = new bool[CELLS];

        for(int x=0; x < COLUMNS; x++) {
            for(int y=0; y<ROWS; y++) {
                // TODO: Get cell index

                // TODO: Get neighbours

                // TODO: Get new state
            }
        }

//        cells = newCells;
    };
    
    void setInitialState() {
        int i = 0;
        while(i < 256) {
            int index = rand() % 1024;
            setCellState(index, true);
            i++;
        }
    };
    
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

