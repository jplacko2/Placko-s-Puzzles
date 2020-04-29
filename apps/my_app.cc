// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>

#include "CinderImGui.h"
#include "cinder/BSpline.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/draw.h"
#include "cinder/gl/gl.h"
#include "mylibrary/example.h"
#include "cinder/Rand.h"

namespace myapp {

using cinder::Color;
using cinder::ColorA;
using cinder::app::KeyEvent;
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
    should_shuffle = true;
    has_shuffled_already = false;
  }
  ui::NewLine();
  if (ui::Button("Solve", ImVec2(200, 150))) {
    should_shuffle = false;
  }
  if (is_jigsaw_mode) {
    ui::SameLine();
    if (ui::Button("Slide Puzzle Mode", ImVec2(200, 150))) {
      is_jigsaw_mode = false;
      has_shuffled_already = false;
      should_shuffle = false;
      breakUpPicture();
    }

  } else {
    ui::SameLine();
    if (ui::Button("Jigsaw Puzzle Mode", ImVec2(200, 150))) {
      is_jigsaw_mode = true;
      has_shuffled_already = false;
      should_shuffle = false;
      breakUpPicture();
    }
  }
}

void MyApp::draw() {
  cinder::gl::enableAlphaBlending();
  cinder::gl::clear();
  if (should_shuffle) {
    if (is_jigsaw_mode) {
      drawPiecesScattered();
    } else {
      drawPiecesSlidePuzzle();
    }
  } else {
    drawPicture();
  }
}

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

  if (mTexture) {
    Rectf destRect = Rectf(mTexture->getBounds())
                         .getCenteredFit(getWindowBounds(), true)
                         .scaledCentered(.85f);
    gl::draw(mTexture, destRect);
  }
}


void MyApp::drawPiecesScattered() {

  gl::clear(Color(0.5f, 0.5f, 0.5f));
  gl::enableAlphaBlending();

  if (!pieces.empty() && !has_shuffled_already) {
    for (int i = 0; i < pieces.size(); i++) {

      pieces.at(i).bounds.moveULTo(ivec2(random.nextInt(0, 1000),
          random.nextInt(0, 1000)));
      gl::draw(pieces.at(i).texture, pieces.at(i).bounds
      .scaledCentered(.15f));
    }
    has_shuffled_already = true;
  }
}
void MyApp::drawPiecesSlidePuzzle() {

}


void MyApp::breakUpPicture() {
  pieces.clear();

  numPiecesX = getOptimalNumPieces(whole_picture.getWidth());
  numPiecesY = getOptimalNumPieces(whole_picture.getHeight());
  int piece_width = whole_picture.getWidth() / numPiecesX;
  int piece_height = whole_picture.getHeight() / numPiecesY;

  for (int i = 0; i < whole_picture.getHeight(); i = i + piece_height) {
    for (int j = 0; j < whole_picture.getWidth(); j = j + piece_width) {
      Area piece_bounds(ivec2(j, i),
          ivec2(j + piece_width, i + piece_height));
      Surface new_piece_surface(piece_width, piece_height, true);
      new_piece_surface.copyFrom(whole_picture, piece_bounds);
      gl::TextureRef texture = gl::Texture::create(new_piece_surface);
      PuzzlePiece new_piece(texture, Rectf(piece_bounds));
      pieces.push_back(new_piece);
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
  return 2;
}

void MyApp::mouseDrag(MouseEvent event) {
  //TODO fix this for dragging pieces
  if (selected_piece.bounds.contains(event.getPos())) {
    selected_piece.bounds = selected_piece.bounds.getOffset(event.getPos());
    return;
  }
  if (!pieces.empty()) {
    for (int i = 0; i < pieces.size(); i++) {
      if (pieces.at(i).bounds.contains(event.getPos())) {
        selected_piece = pieces.at(i);
        selected_piece.bounds = selected_piece.bounds.getOffset(event.getPos());
        return;
      }
    }
  }
}

}  // namespace myapp
