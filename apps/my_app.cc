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

MyApp::MyApp() {}

void MyApp::setup() {
  cinder::gl::enableDepthWrite();
  cinder::gl::enableDepthRead();
  ui::initialize();
}

void MyApp::update() {
  ui::ScopedWindow tools_window("Tools");

  if (ui::Button("Find Picture", ImVec2(200, 150))) {
    try {
      fs::path path = getOpenFilePath("", ImageIo::getLoadExtensions());
      if (!path.empty()) {
        whole_picture = Surface(loadImage(path));
        mTexture = gl::Texture::create(whole_picture);
        breakUpPicture();
      }
    } catch (Exception &exc) {
      CI_LOG_EXCEPTION("failed to load image.", exc);
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
    if (ui::Button("Jigsaw Puzzle Mode", ImVec2(200, 150))) {
      is_jigsaw_mode = true;
    }
  }
}

void MyApp::draw() {
  cinder::gl::enableAlphaBlending();
  cinder::gl::clear();
  drawPicture();
}

void MyApp::keyDown(KeyEvent event) {}

void MyApp::fileDrop(FileDropEvent event) {
  try {
    whole_picture = Surface(loadImage(loadFile(event.getFile(0))));
    mTexture = gl::Texture::create(whole_picture);
    breakUpPicture();
  } catch (Exception &exc) {
    CI_LOG_EXCEPTION("failed to load image: " << event.getFile(0), exc);
  }
}

void MyApp::drawPicture() {
  gl::clear(Color(0.5f, 0.5f, 0.5f));
  gl::enableAlphaBlending();

 /* if (mTexture) {
    Rectf destRect = Rectf(mTexture->getBounds())
                         .getCenteredFit(getWindowBounds(), true)
                         .scaledCentered(.85f);
    gl::draw(mTexture, destRect);
  }*/
  if (!piece_textures.empty()) {
    Rectf pieceRect = Rectf(piece_textures.at(0)->getBounds())
                      .getCenteredFit(getWindowBounds(), true)
                      .scaledCentered(.85f);
    gl::draw(piece_textures.at(0), pieceRect);
  }
}

  void MyApp::breakUpPicture() {
    piece_textures.clear();

    numPiecesX = getOptimalNumPieces(whole_picture.getWidth());
    numPiecesY = getOptimalNumPieces(whole_picture.getHeight());
    int piece_width = whole_picture.getWidth() / numPiecesX;
    int piece_height = whole_picture.getHeight() / numPiecesY;

    for (int i = 0; i < whole_picture.getHeight(); i = i + piece_height) {
      for (int j = 0; j < whole_picture.getWidth(); j = j + piece_width) {
        Area piece_bounds(i, j, i + piece_width, j + piece_height);
        Surface new_piece;
        new_piece.copyFrom(whole_picture, piece_bounds, ImVec2(0, 0));
        piece_textures.push_back(gl::Texture::create(new_piece));
      }
  }
}

int MyApp::getOptimalNumPieces(int length) {
  if (is_jigsaw_mode) {
    for (int num = 10; num <= 40; num++) {
      if (length % num == 0) {
        return num;
      }
    }
  } else {
    for (int num = 3; num <= 10; num++) {
      if (length % num == 0) {
        return num;
      }
    }
  }
}

}// namespace myapp
