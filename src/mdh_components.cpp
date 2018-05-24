#include "mdh_components.hpp"

using namespace rack;
extern Plugin *plugin;

// TODO: Remove unnecessary duplication in this class...

BubbleScrew::BubbleScrew() {
    tw = new TransformWidget();
    addChild(tw);
    
    sw = new SVGWidget();
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/bubble.svg")));
    tw->addChild(sw);
    
    tw->scale(Vec(0.18f, 0.18f));
    
    tw->box.size = sw->box.size;
    box.size = sw->box.size;
}

FaceSVG::FaceSVG() {
    tw = new TransformWidget();
    addChild(tw);
    
    sw = new SVGWidget();
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/face.svg")));
    tw->addChild(sw);
    
    tw->scale(Vec(1.0f, 1.0f));
    
    tw->box.size = sw->box.size;
    box.size = sw->box.size;
}

SVGAnimation::SVGAnimation() {
    tw = new TransformWidget();
    addChild(tw);
    
    sw = new SVGWidget();
    tw->addChild(sw);
    tw->scale(Vec(0.2f, 0.2f));
    tw->box.size = sw->box.size;
    
    box.size = sw->box.size;
}

void SVGAnimation::addFrame(std::shared_ptr<SVG> svg) {
    frames.push_back(svg);
    
    // Set first frame.
    if(!sw->svg) {
        sw->setSVG(svg);
        box.size = sw->box.size;
    }
}

//void SVGAnimation::draw(NVGcontext *vg) {
////    if(frames.size() == 0)
////        return;
////
////    sw->setSVG(frames[frame % frames.size()]);
////    dirty = true;
//    
//    FramebufferWidget::draw(vg);
//}

