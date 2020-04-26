// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <cinder/Color.h>
#include <cinder/app/App.h>
#include "cinder/app/RendererGl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

namespace myapp {

using namespace ci;
using namespace ci::app;
using namespace std;

class MyApp : public cinder::app::App {
 public:
  MyApp();
  void setup() override;
  void update() override;
  void draw() override;
  void keyDown(cinder::app::KeyEvent) override;
  void fileDrop( FileDropEvent event ) override;

  gl::TextureRef		mTexture;
  bool is_jigsaw_mode = true;

 private:
  void createToolsWindow ();
  void drawPicture();
};



}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
