#include "rack.hpp"

#pragma once

using namespace rack;

struct BubbleScrew : FramebufferWidget {
    TransformWidget *tw;
    SVGWidget *sw;
    
    BubbleScrew();
};

struct FaceSVG : FramebufferWidget {
    TransformWidget *tw;
    SVGWidget *sw;
    
    FaceSVG();
};

struct SVGAnimation : FramebufferWidget {
    SVGWidget *sw;
    int frame = 0;
    std::vector<std::shared_ptr<SVG>> frames;
    
    SVGAnimation();
    
    void addFrame(std::shared_ptr<SVG> svg);
    
    void draw(NVGcontext *vg) override;
};
