// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"
#include "mylibrary/example.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/BSpline.h"
#include "cinder/gl/draw.h"


#include "CinderImGui.h"

#include <cinder/app/App.h>


namespace myapp {

using cinder::app::KeyEvent;
using cinder::Color;
using cinder::ColorA;
using namespace ImGui;

MyApp::MyApp() { }

void MyApp::setup() {
  cinder::gl::enableDepthWrite();
  cinder::gl::enableDepthRead();
  ui::initialize();


}

void MyApp::update() {
  ui::ScopedWindow tools_window("Tools");

  if (ui::Button("Find Picture", ImVec2(200, 150))) {
    try {
      fs::path path = getOpenFilePath( "", ImageIo::getLoadExtensions() );
      if( ! path.empty() ) {
        mTexture = gl::Texture::create( loadImage( path ) );
      }
    }
    catch( Exception &exc ) {
      CI_LOG_EXCEPTION( "failed to load image.", exc );
    }
  }
  ui::SameLine();
  if (ui::Button("Shuffle Pieces", ImVec2(200, 150))) {

  }

  if (is_jigsaw_mode) {
    ui::SameLine();
    if (ui::Button("Slide Puzzle Mode", ImVec2(200, 150))) {
      is_jigsaw_mode = false;
    }

  } else {
    ui::SameLine();
    if (ui::Button("Slide Puzzle Mode", ImVec2(300, 150))) {
      is_jigsaw_mode = true;
    }
  }
}

void MyApp::draw() {
 cinder::gl::enableAlphaBlending();
  cinder::gl::clear();
  drawPicture();
}

void MyApp::keyDown(KeyEvent event) { }

void MyApp::createToolsWindow () {

}

void MyApp::fileDrop( FileDropEvent event ) {
  try {
    mTexture = gl::Texture::create( loadImage( loadFile( event.getFile( 0 ) ) ) );
  }
  catch( Exception &exc ) {
    CI_LOG_EXCEPTION( "failed to load image: " << event.getFile( 0 ), exc );
  }
}

void MyApp::drawPicture() {
 gl::clear( Color( 0.5f, 0.5f, 0.5f ) );
  gl::enableAlphaBlending();

  if( mTexture ) {
    Rectf destRect = Rectf( mTexture->getBounds() ).getCenteredFit( getWindowBounds(), true ).scaledCentered( .85f );
    gl::draw( mTexture, destRect );
  }
}

}


// namespace myapp
