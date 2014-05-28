#include "poShape.h"

#include "Masking.h"
#include "poScene.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;

bool bUseFbo = false;
bool bShowMask = true;

MaskingRef Masking::create() {
    MaskingRef node(new Masking());
    node->setup();
    return node;
}


void Masking::setup() {
    //setPosition(200,50);
    
    ci::gl::TextureRef texture = ci::gl::Texture::create(ci::loadImage(ci::app::loadAsset(("zach.jpg"))));
    mZach = po::Shape::create(texture);
    //mZach->setTexture(texture, po::TextureFit::Type::EXACT);
//    mZach->setAlignment(po::Alignment::CENTER_CENTER);
    mZach->setPosition(ci::app::getWindowWidth()/2,
                       ci::app::getWindowHeight()/2);
    //image->setRotation(45);
    addChild(mZach);
    
    //Create mask
    ci::gl::TextureRef maskTex = gl::Texture::create(loadImage(loadAsset("mask.png")));
    mMask = po::Shape::create(maskTex);
    mZach->setMask(mMask);
    
    mMask->fillColor(ci::Color(1,0,1));
    mMask->setPosition(0,0);
    addChild(mMask);
    
    //Load the shader
//    try {
////        mShader = gl::GlslProg ( loadAsset("poMask_vert.glsl"), loadAsset( "poMask_frag.glsl"));
//        mShader = gl::GlslProg ( loadResource(RES_GLSL_PO_MASK_VERT), loadResource( RES_GLSL_PO_MASK_FRAG));
//    } catch (gl::GlslProgCompileExc e) {
//        console() << "Could not load shader: " << e.what() << std::endl;
//        exit(1);
//    }
    
    //ci::app::timeline().apply(&image->getRotationAnim(), 360.f, 3.f).loop();
    
    //setCacheToFboEnabled(true);
    
    //    po::ShapeRef mask = po::Shape::createRect(200,200);
    //    mask->setFillColor(255,0,255);
    //    mask->setAlignment(po::Alignment::CENTER_CENTER);
    //    mask->setPosition(image->getPosition().x, image->getPosition().y);
    ////    addChild(mask);

}

void Masking::_drawTree() {
    if(!bUseFbo) {
        po::Node::drawTree();
        return;
    }

    {
        //Save the window buffer
        gl::SaveFramebufferBinding binding;
        
        //Setup the FBO
        mFbo = gl::Fbo(getWidth(), getHeight());
        gl::setViewport(mFbo.getBounds());
        mFbo.bindFramebuffer();
        
        ci::CameraOrtho cam;
//        cam.setOrtho( 0,getBounds().getWidth(), getBounds().getHeight(), 0, -1, 1 );
        cam.setOrtho( 0, getBounds().getWidth(), 0, getBounds().getHeight(), -1, 1 );
        gl::setMatrices(cam);
        
        //Draw into the FBO
        gl::pushMatrices();
        gl::translate(-getFrame().getUpperLeft());
        
        ci::gl::clear(ci::Color(0,0,0));
        po::Node::drawTree();
        
        gl::popMatrices();
    }

    gl::setViewport(app::getWindowBounds());
    gl::setMatrices(getScene()->getCamera());
    
    _drawFbo();
    
}

void Masking::_drawFbo() {
    if(!bUseFbo) {
        po::NodeContainer::draw();
        return;
    }
    
    gl::Texture tex = mFbo.getTexture();
    tex.setFlipped(true);
    
    if(bShowMask) {
        tex.bind(0);
        mMask->getTexture()->bind(1);
        
        mShader.bind();
        mShader.uniform("tex", 0);
        mShader.uniform("mask", 1);
        mShader.uniform ( "contentScale", Vec2f((float)tex.getWidth() / (float)mMask->getWidth(), (float)tex.getHeight() / (float)mMask->getHeight() ) );
            
        mShader.uniform ( "maskPosition", maskPos/ci::Vec2f(mFbo.getWidth(), mFbo.getHeight()) );
    }

    
    beginDrawTree();
    
    gl::color(255,255,255);
    if(bShowMask) {
        //gl::scale(1,-1,1);
        gl::drawSolidRect(mFbo.getBounds());
    } else {
        gl::draw(tex);
        //gl::draw(mMaskTex);
    }

    finishDrawTree();

    if(bShowMask) {
        tex.unbind();
        mMask->getTexture()->unbind();
        mShader.unbind();
    }
}

void Masking::mouseMove(po::MouseEvent &event)
{
    maskPos = event.getPos() - mMask->getTexture()->getSize()/2;
    //std::cout << event.getPos() << std::endl;
    //maskPos = event.getPos() - ci::Vec2f(mMaskTex->getWidth(), mMaskTex->getHeight())/2;
    //std::cout << maskPos << std::endl;
}


void Masking::keyDown(po::KeyEvent &event)
{
    if(event.getChar() == 'm') {
        if(mZach->hasMask()) {
            mZach->removeMask();
        } else {
            mZach->setMask(mMask);
        }
    }
    
    else if(event.getChar() == 'f') {
        bUseFbo = !bUseFbo;
    }
}