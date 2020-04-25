// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <cinder/Color.h>
#include <cinder/app/App.h>

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
};

// The window-specific data for each window
class WindowData {
 public:
  WindowData()
      : mColor(Color( 0 , 0.8, 0.8 ) ) // a random color
  {}

  Color	mColor;
  list<cinder::vec2>		mPoints; // the points drawn into this window
};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
