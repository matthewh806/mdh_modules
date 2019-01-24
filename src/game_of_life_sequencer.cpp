#include "mdh_modules.hpp"
#include "componentlibrary.hpp"
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
        SEQUENCE_LENGTH_PARAM,
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
        GATE_OUTPUT,
        VOCT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };
    
    bool *cells = new bool[CELLS];
    int seqPos = 0; // TODO: These ints can easily overflow
    int lifeCounter = 0;
    
    rack::dsp::SchmittTrigger clearTrigger, randomizeTrigger;
    rack::dsp::SchmittTrigger clockTrigger;
    
    GameOfLifeSequencerModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        setRandomState();
    }
    
    ~GameOfLifeSequencerModule() {
        delete []cells;
    }
    
    void onRandomize() override {
        setRandomState();
    }
    
    void onReset() override {
        resetSeq();
        clearCells();
    }
    
    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        
        json_t *cellsJ = json_array();
        for(int i = 0; i < CELLS; i++) {
            json_t *cellJ = json_integer((int) cells[i]);
            json_array_append_new(cellsJ, cellJ);
        }
        
        json_object_set_new(rootJ, "cells", cellsJ);
        
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {
        json_t *cellsJ = json_object_get(rootJ, "cells");
        
        if(cellsJ) {
            for(int i=0; i < CELLS; i++) {
                json_t *cellJ = json_array_get(cellsJ, i);
                if(cellJ)
                    cells[i] = json_integer_value(cellJ);
            }
        }
    }
    
    void step() override {
        bool gateIn = false;
        
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
        // TODO: Reduce nesting
        if(inputs[EXTERNAL_CLOCK_INPUT].active) {
            int seqLen = int(params[GameOfLifeSequencerModule::SEQUENCE_LENGTH_PARAM].value);

            if(clockTrigger.process(inputs[EXTERNAL_CLOCK_INPUT].value)) {
                if(params[STEP_LIFE_SWITCH_PARAM].value) {
                    if( ++lifeCounter % int(params[LIFE_SPEED_KNOB_PARAM].value) == 0) {
                        stepLife();
                    }
                }
                
                seqPos = (seqPos + 1) % seqLen;
            }
            
            gateIn = clockTrigger.isHigh();
        }
        
        outputs[GATE_OUTPUT].value = (gateIn && columnActiveCellCount(seqPos % COLUMNS) > 0) ? 10.0f : 0.0f;
        outputs[VOCT_OUTPUT].value = calculateVoltageForColumn(seqPos % COLUMNS);
    };
    
    void resetSeq() {
        seqPos = 0;
    }
    
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
    
    bool getCellCurrentState(int x, int y) {
        return cells[getCellIndexFromXY(x, y)];
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
    
    bool getCellCurrentStateByDisplayPos(float x, float y) {
        return getCellCurrentState(int(x/HW), int(y/HW));
    };
    
    void setCellStateByDisplayPos(float x, float y, bool on) {
        setCellState(int(x / HW), int(y / HW), on);
    };
    
    bool isInRange(int x, int y) {
        return x >= 0 && x <= COLUMNS && y >= 0 && y <= ROWS;
    };
    
    float calculateVoltageForColumn(int columnIdx) {
        /* TODO:
         *  Voltage range should span an entire octave.
         *
         *  Therefore: consider up to 12 notes simultaneously
         *  e.g. 1 column active: voltage 1*(1 / 12) = 0.0833
            2 columns active: voltage = 2*(1/12) = 0.1667
            etc
         
            I think this is how it should work...?
         
            Don't bother quantizing it or rx1elating it to a scale for now.
            Leave that for other modules or later work.
         
            C2 = 0V
            F2# = 0.5V (halfway)
            C3 = 1V
         
            range 0 - +5V covers 5 ovtaves.
         
            Voltage ref:
            http://www.bytenoise.co.uk/Appendix%3A_pitches
         
         Note: I didn't use 0.0833 because I already saw some rounding errors creeping in affecting the output note.
         */
        
        int cellCount = columnActiveCellCount(columnIdx);
        return (1/12.0f * cellCount) < 1 ? 1/12.0f * cellCount : 1;
    }
    
    int columnActiveCellCount(int columnIdx) {
        int count = 0;
        
        for(int i=0; i < ROWS; i++) {
            count += cells[getCellIndexFromXY(columnIdx, i)] ? 1 : 0;
        }
        
        return count;
    }
};

struct ConwaySeqDisplay : Widget {
    GameOfLifeSequencerModule * module;
    bool newState = false;
    float dragX = 0;
    float dragY = 0;
    float initX = 0;
    float initY = 0;
    
    ConwaySeqDisplay() {};
    
    void onButton(const event::Button &e) override {
        if (e.button != 0) {
            return;
        }
        
        // e.consume(true);
        // e.target = this;
        
        initX = e.pos.x;
        initY = e.pos.y;
        
        newState = !(module->getCellCurrentStateByDisplayPos(initX, initY));
        module->setCellStateByDisplayPos(e.pos.x, e.pos.y, newState);
    }
    
    void onDragStart(const event::DragStart &e) override {
        dragX = app()->scene->rackWidget->mousePos.x;
        dragY = app()->scene->rackWidget->mousePos.y;
    }
    
    void onDragMove(const event::DragMove &e) override {
        float deltaX = app()->scene->rackWidget->mousePos.x - dragX;
        float deltaY = app()->scene->rackWidget->mousePos.y - dragY;
        
        module->setCellStateByDisplayPos(initX + deltaX, initY + deltaY, newState);
    }
    
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
        
        //seq pos
        nvgStrokeColor(vg, nvgRGB(255, 255, 255));
        nvgBeginPath(vg);
        nvgRect(vg, module->seqPos*HW, 0, HW, box.size.y);
        nvgStroke(vg);
        
        //seq len line
        float seqLen = module->params[GameOfLifeSequencerModule::SEQUENCE_LENGTH_PARAM].value * HW;
        nvgStrokeColor(vg, nvgRGB(144, 26, 252));
        nvgBeginPath(vg);
        nvgMoveTo(vg, seqLen, 0);
        nvgLineTo(vg, seqLen, box.size.y);
        nvgStroke(vg);
    };
};

struct GameOfLifeSequencerWidget : ModuleWidget {
    GameOfLifeSequencerWidget(GameOfLifeSequencerModule *module): ModuleWidget(module) {
        setPanel(SVG::load(asset::plugin(plugin, "res/ConwaySeq.svg")));
        
        addInput(createInput<PJ301MPort>(Vec(50, 10), module, GameOfLifeSequencerModule::EXTERNAL_CLOCK_INPUT));
        
        addOutput(createOutput<PJ301MPort>(Vec(326, 10), module, GameOfLifeSequencerModule::GATE_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(326, 40), module, GameOfLifeSequencerModule::VOCT_OUTPUT));
        
        addParam(createParam<LEDButton>(Vec(50, 320), module, GameOfLifeSequencerModule::CLEAR_PARAM));
        addParam(createParam<LEDButton>(Vec(100, 320), module, GameOfLifeSequencerModule::RANDOMIZE_PARAM));
        addParam(createParam<CKSS>(Vec(150, 320), module, GameOfLifeSequencerModule::STEP_LIFE_SWITCH_PARAM));
        
        addParam(createParam<RoundBlackSnapKnob>(Vec(200, 320), module, GameOfLifeSequencerModule::LIFE_SPEED_KNOB_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(250, 320), module, GameOfLifeSequencerModule::SEQUENCE_LENGTH_PARAM));
        
        {
            ConwaySeqDisplay *display = new ConwaySeqDisplay();
            display->module = module;
            display->box.pos = Vec(50, 50);
            display->box.size = Vec(256, 256);
            
            addChild(display);
        }
    }
};

Model *modelGameOfLifeSequencer = createModel<GameOfLifeSequencerModule, GameOfLifeSequencerWidget>("Game Of Life Sequencer");

