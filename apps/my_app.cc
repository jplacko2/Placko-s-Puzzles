// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"
#include "mylibrary/example.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/BSpline.h"
#include "cinder/gl/draw.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"

#include <cinder/app/App.h>


namespace myapp {

using cinder::app::KeyEvent;
using cinder::Color;
using cinder::ColorA;

MyApp::MyApp() { }

void MyApp::setup() {
  cinder::gl::enableDepthWrite();
  cinder::gl::enableDepthRead();
  getWindow()->setUserData(new WindowData);
  app::WindowRef new_window = createWindow(Window::Format().size(400, 400));
  ui::initialize();

  //ui::initialize( ui::Options().window(new_window).frameRounding( 0.0f ) );
}

void MyApp::update() { }

void MyApp::draw() {
  cinder::gl::enableAlphaBlending();
  cinder::gl::clear();
  cinder::gl::clear(Color(1, 1, 1));
  ImGui::Button("shuffle", ImVec2(100, 100));
}

void MyApp::keyDown(KeyEvent event) { }

}  // namespace myapp
