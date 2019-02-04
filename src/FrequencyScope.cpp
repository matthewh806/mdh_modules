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

    float buffer[BUFFER_SIZE] = {};
    int bufferIndex = 0;
    float frameIndex = 0;

    float inputValue = 0;

    rack::dsp::RealFFT rfft;

    FrequencyScopeModule() : rfft(BUFFER_SIZE) {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    
    void step() override;
};

void FrequencyScopeModule::step() {
    inputValue = inputs[INPUT].value;

    // TODO: I've just copied this from the scope example blindly...
    float deltaTime = std::pow(2.0f, -14.0f);
    int frameCount = (int)std::ceil(deltaTime * APP->engine->getSampleRate());

    // Add frame to buffer
    if(bufferIndex < BUFFER_SIZE) {
        if(++frameIndex > frameCount) {
            frameIndex = 0;
            buffer[bufferIndex] = inputs[INPUT].value;
            bufferIndex;
        }
    }
};

struct ScopeDisplay : TransparentWidget {
    FrequencyScopeModule *module;
    int frame = 0;

    float amp = 0;
    float freq = 0;

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

        nvgFontSize(ctx.vg, 10);
        nvgFontFaceId(ctx.vg, font->handle);
        nvgTextLetterSpacing(ctx.vg, 2.0);

        // TODO: Should this fft really be handled here in the draw call?
        float *in = rack::dsp::alignedNew<float>(BUFFER_SIZE);
        float *out = rack::dsp::alignedNew<float>(2 * BUFFER_SIZE);
        for(int i = 0; i < BUFFER_SIZE; i++) {
            in[i] = module->buffer[i] / 10.0f;
        }

        module->rfft.rfft(in, out);

        nvgStrokeColor(ctx.vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xc0));
        nvgSave(ctx.vg);

        Vec box_size = box.size.minus(Vec(0, 30));
        float box_width = box_size.x;
        // float slice_width = box_width * (1.0 / (BUFFER_SIZE*2));

        Rect b = Rect(Vec(0, 15), box_size);
        nvgScissor(ctx.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
        nvgBeginPath(ctx.vg);

        for(int i = 0; i < BUFFER_SIZE * 2; i++) {
            float x = (float)i / ((2*BUFFER_SIZE)- 1);
            float y = out[i];

            Vec p;
            p.x = b.pos.x + b.size.x * x;
            p.y = b.pos.y + b.size.y * (1.0f-y);
            
            if(i == 0)
                nvgMoveTo(ctx.vg, p.x, p.y);
            else 
                nvgLineTo(ctx.vg, p.x, p.y);
        }

        nvgLineCap(ctx.vg, NVG_ROUND);
        nvgMiterLimit(ctx.vg, 2.0f);
        nvgStrokeWidth(ctx.vg, 1.5f);
        nvgGlobalCompositeOperation(ctx.vg, NVG_LIGHTER);
        nvgStroke(ctx.vg);
        nvgResetScissor(ctx.vg);
        nvgRestore(ctx.vg);

        Vec textPos = Vec(10, 20);
        NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
        nvgFillColor(ctx.vg, textColor);
        nvgText(ctx.vg, textPos.x, textPos.y, "Amplitude - ", NULL);
        nvgText(ctx.vg, textPos.x, textPos.y + 20, "Frequency - ", NULL);

        // Draw text stats
        if(++frame >= 4) {
            frame = 0;
            amp = module->inputValue;
            freq = std::abs(out[0]);
        }

        nvgText(ctx.vg, textPos.x + 120, textPos.y, std::to_string(amp).c_str(), NULL);
        nvgText(ctx.vg, textPos.x + 120, textPos.y + 20, std::to_string(freq).c_str(), NULL);
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