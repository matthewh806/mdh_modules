#include "mdh_components.hpp"

using namespace rack;

SVGAnimation::SVGAnimation() {
    sw = new SVGWidget();
    addChild(sw);
}

void SVGAnimation::addFrame(std::shared_ptr<SVG> svg) {
    frames.push_back(svg);
    
    // Set first frame.
    if(!sw->svg) {
        sw->setSVG(svg);
        box.size = sw->box.size;
    }
}

void SVGAnimation::draw(NVGcontext *vg) {
    if(frames.size() == 0)
        return;
    
    sw->setSVG(frames[frame % frames.size()]);
    dirty = true;
}
