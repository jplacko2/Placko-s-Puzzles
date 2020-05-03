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
        should_shuffle = false;
        has_shuffled_already = false;
      }
    } catch (Exception& exc) {
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
      drawPuzzleBorder();
      drawMiniViewPicture();
    } else {
      drawMiniViewPicture();
      drawPuzzleBorder();
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
    should_shuffle = false;
    has_shuffled_already = false;
  } catch (Exception &exc) {
    CI_LOG_EXCEPTION("failed to load image: " << event.getFile(0), exc);
  }
}

void MyApp::drawPicture() {
  gl::clear(Color(0.5f, 0.5f, 0.5f));
  gl::enableAlphaBlending();

  if (mTexture) {
    whole_pic_rect = Rectf(mTexture->getBounds())
                         .getCenteredFit(getWindowBounds(), false)
                         .getOffset(ivec2(300, 50))
                         .scaled(.8f);
    gl::draw(mTexture, whole_pic_rect);
  }
}


void MyApp::drawPiecesScattered() {

  gl::clear(Color(0.5f, 0.5f, 0.5f));
  gl::enableAlphaBlending();

  if (!pieces.empty()) {
    for (int i = 0; i < pieces.size(); i++) {
      PuzzlePiece& piece = pieces.at(i);

      if (!has_shuffled_already) {
        piece.bounds.moveULTo(ivec2(random.nextInt(5000),
            random.nextInt(2500)));
      }
      gl::draw(piece.texture, piece.bounds.scaled(.35f));
    }
    has_shuffled_already = true;
  }
}


void MyApp::drawMiniViewPicture() {
  if (mTexture) {
    Rectf border = Rectf(mTexture->getBounds())
        .scaled(.1f)
        .getMoveULTo(ivec2(1500, 50));
    gl::draw(mTexture, border);
  }
}


void MyApp::breakUpPicture() {
  pieces.clear();

  num_pieces_x = getOptimalNumPieces(whole_picture.getWidth());
  num_pieces_y = getOptimalNumPieces(whole_picture.getHeight());
  piece_width = whole_picture.getWidth() / num_pieces_x;
  piece_height = whole_picture.getHeight() / num_pieces_y;

  for (int y = 0; y < whole_picture.getHeight(); y = y + piece_height) {
    for (int x = 0; x < whole_picture.getWidth(); x = x + piece_width) {

      Surface new_piece_surface(piece_width, piece_height, true);


      gl::TextureRef texture = getPieceTexture(x, y, x + piece_width,
          y + piece_height, new_piece_surface);
      Rectf piece_rect = Rectf(texture->getBounds());
      PuzzlePiece new_piece = PuzzlePiece(texture, piece_rect);
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

void MyApp::mouseDown(MouseEvent event) {
  float x = event.getPos().x / .35f;
  float y = event.getPos().y / .35f;
  ivec2 adjusted_coords(x, y);
  if (selected_piece == nullptr && !pieces.empty()) {
    for (int i = 0; i < pieces.size(); i++) {
      if (pieces.at(i).bounds.contains(adjusted_coords)) {
        selected_piece = &pieces.at(i);
        return;
      }
    }
  } else {
    selected_piece->bounds.offsetCenterTo(adjusted_coords);
    selected_piece = nullptr;
  }
}

/*void MyApp::mouseDrag(MouseEvent event) {
  float x = event.getPos().x / .35f;
  float y = event.getPos().y / .35f;
  ivec2 adjusted_coords(x, y);
  if (selected_piece != nullptr) {
    selected_piece->bounds.offsetCenterTo(event.getPos());
  } else if (!pieces.empty()) {
    for (int i = 0; i < pieces.size(); i++) {
      if (pieces.at(i).bounds.contains(adjusted_coords)) {
        selected_piece = &pieces.at(i);
        selected_piece->bounds.offsetCenterTo(event.getPos());
        return;
      }
    }
  }
}*/

/*void MyApp::mouseMove(MouseEvent event) {

}*/

/*void MyApp::mouseUp(MouseEvent event) {
  selected_piece = nullptr;
}*/

gl::TextureRef MyApp::getPieceTexture(int start_x, int start_y, int end_x, int end_y,
                               Surface& result_surface) {
  for (int x = 0; x < end_x - start_x; x++) {
    for (int y = 0; y < end_y - start_y; y++) {
      result_surface.setPixel(ivec2(x, y),
          whole_picture.getPixel(ivec2(x + start_x, y + start_y)));
    }
  }
  gl::TextureRef to_return = gl::Texture::create(result_surface);
  return to_return;
}

void MyApp::drawPuzzleBorder() {
  Path2d path;
  path.moveTo(whole_pic_rect.getUpperLeft());
  path.lineTo(whole_pic_rect.getLowerLeft());
  path.lineTo(whole_pic_rect.getLowerRight());
  path.lineTo(whole_pic_rect.getUpperRight());
  path.close();
  gl::draw(path);
}
}  // namespace myapp
