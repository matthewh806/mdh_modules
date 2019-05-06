#include "mdh_modules.hpp"
#include "component.hpp"
#include "dsp/fft.hpp"

static const int BUFFER_SIZE = 1024;

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
    float *in = rack::dsp::alignedNew<float>(BUFFER_SIZE);
    float *out = rack::dsp::alignedNew<float>(2 * BUFFER_SIZE);
    float *power_spectrum = rack::dsp::alignedNew<float>(BUFFER_SIZE);

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

    // TODO: Should this fft really be handled here in the draw call?       
    for(int i = 0; i < BUFFER_SIZE; i++) {
        in[i] = buffer[i] / 10.0f;
    }

    rfft.rfft(in, out);

    // Get the power spectrum of out
    /*
    * k[0] - DC offset
    * k[1] - n/2 for even n
    * k[2] = real(F(1))
    * k[3] = imag(F(1))
    * k[4] = real(F(2))
    * k[5] = imag(F(2))
    * ...
    * k[length - 2] = real(F(n/2 - 1))
    * k[length - 1] = imag(F(n/2 - 1))
    */

    power_spectrum[0] = out[0] * out[0]; // DC component of spectrum
    for(int k = 2; k < BUFFER_SIZE; k+=2) { // TODO: Check the rounding of (BUFFER_SIZE + 1)/ 2 -> it should be equivalent to BUFFER_SIZE / 2 rounded up (C rounds integers down)
        power_spectrum[k] = std::sqrt(out[k] * out[k] + out[k+1] * out[k+1]);
    }

    if(BUFFER_SIZE % 2 == 0) {
        power_spectrum[1] = out[1] * out[1];
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

    void draw(const DrawArgs &args) override {
        if(!module) return;

        //bg
        nvgFillColor(args.vg, nvgRGB(20, 30, 33));
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgFill(args.vg);

        nvgFontSize(args.vg, 10);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, 2.0);

        nvgStrokeColor(args.vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xc0));
        nvgSave(args.vg);

        Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 30)));
        nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
        nvgBeginPath(args.vg);

        for(int i = 0; i < BUFFER_SIZE; i++) {
            float x = (float)i / (BUFFER_SIZE- 1);
            float y = module->power_spectrum[i];

            Vec p;
            p.x = b.pos.x + b.size.x * x;
            p.y = b.pos.y + b.size.y * (1.0f-y);
            
            nvgRect(args.vg, p.x, 0, box.size.x, p.y);
        }

        // for(int i = 0; i < 10; i++) {
        //     float x = (float)i / 9;

        //     Vec p;
        //     p.x = box.pos.x + b.size.x * x;

        //     nvgRect(args.vg, box.pos.x, 0, p.x, box.size.y);
        // }

        nvgLineCap(args.vg, NVG_ROUND);
        nvgMiterLimit(args.vg, 2.0f);
        nvgStrokeWidth(args.vg, 1.5f);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgStroke(args.vg);
        nvgResetScissor(args.vg);
        nvgRestore(args.vg);

        Vec textPos = Vec(10, 20);
        NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
        nvgFillColor(args.vg, textColor);
        nvgText(args.vg, textPos.x, textPos.y, "Amplitude - ", NULL);
        nvgText(args.vg, textPos.x, textPos.y + 20, "Frequency - ", NULL);

        // Draw text stats
        if(++frame >= 4) {
            frame = 0;
            amp = module->inputValue;
            freq = std::abs(module->power_spectrum[0]);
        }

        nvgText(args.vg, textPos.x + 120, textPos.y, std::to_string(amp).c_str(), NULL);
        nvgText(args.vg, textPos.x + 120, textPos.y + 20, std::to_string(freq).c_str(), NULL);
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