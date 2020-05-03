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
        setUpFrame();
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
  if (is_hard_mode) {
    ui::SameLine();
    if (ui::Button("Easy Mode", ImVec2(200, 150))) {
      is_hard_mode = false;
      has_shuffled_already = false;
      should_shuffle = false;
      breakUpPicture();
      kPieceScale = .25f;
    }
  } else {
    ui::SameLine();
    if (ui::Button("Hard Mode", ImVec2(200, 150))) {
      is_hard_mode = true;
      has_shuffled_already = false;
      should_shuffle = false;
      breakUpPicture();
      kPieceScale = .35f;
    }
  }
}

void MyApp::draw() {
  cinder::gl::enableAlphaBlending();
  cinder::gl::clear();
  if (should_shuffle) {
    drawPiecesScattered();
    drawPuzzleFrame();
    drawMiniViewPicture();
  } else {
    drawPicture();
  }
}

void MyApp::fileDrop(FileDropEvent event) {
  try {
    whole_picture = Surface(loadImage(loadFile(event.getFile(0))));
    mTexture = gl::Texture::create(whole_picture);
    breakUpPicture();
    setUpFrame();
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
                         .scaled(kPicScale);
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
      if (piece.is_in_frame) {
      } else {
        gl::draw(piece.texture, piece.bounds.scaled(kPieceScale));
      }
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
      PuzzlePiece new_piece = PuzzlePiece(texture, piece_rect,
          x / piece_width, y / piece_height);
      pieces.push_back(new_piece);
    }
  }
}

int MyApp::getOptimalNumPieces(int length) {
  if (is_hard_mode) {
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
  float x = event.getPos().x / kPieceScale;
  float y = event.getPos().y / kPieceScale;
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
    if (whole_pic_rect.contains(event.getPos())) {
//TODO Fix piece locking in, add end of game feature, add tests
      if (pieceIsInCorrectSpot(selected_piece, ivec2(event.getPos().x / kPicScale,
          event.getPos().y / kPicScale))) {

        fillInPieceInFrame(selected_piece->x_index * piece_width,
            selected_piece->y_index * piece_height,
            (selected_piece->x_index + 1) * piece_width,
            (selected_piece->y_index + 1) * piece_height);
        selected_piece->is_in_frame = true;
      }
    }
    selected_piece = nullptr;
  }
}


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

void MyApp::drawPuzzleFrame() {
  Path2d path;
  path.moveTo(whole_pic_rect.getUpperLeft());
  path.lineTo(whole_pic_rect.getLowerLeft());
  path.lineTo(whole_pic_rect.getLowerRight());
  path.lineTo(whole_pic_rect.getUpperRight());
  path.close();
  gl::draw(path);
  gl::TextureRef texture = gl::Texture::create(pic_in_frame);
  gl::draw(texture, whole_pic_rect);
}

bool MyApp::pieceIsInCorrectSpot(PuzzlePiece* piece, ivec2 center) {
  Rectf pieceRect((piece->x_index * piece_width) / kPicScale,
                  (piece->y_index * piece_height) / kPicScale,
                  ((piece->x_index + 1) * piece_width) / kPicScale,
                  ((piece->y_index + 1) * piece_height) / kPicScale);
  return pieceRect.contains(center);
}

void MyApp::setUpFrame() {
  pic_in_frame = Surface(mTexture->getWidth(), mTexture->getHeight(), true);
  for (int x = 0; x < pic_in_frame.getWidth(); x++) {
    for (int y = 0; y < pic_in_frame.getHeight(); y++) {
      pic_in_frame.setPixel(ivec2(x, y), ColorA(.3f, .3f, .3f));
    }
  }
}

void MyApp::fillInPieceInFrame(int x1, int y1, int x2, int y2) {
  for (int pic_x = x1; pic_x < x2; pic_x++) {
    for (int pic_y = y1; pic_y < y2; pic_y++) {
      pic_in_frame.setPixel(ivec2(pic_x, pic_y),
          whole_picture.getPixel(ivec2(pic_x, pic_y)));
    }
  }
}
}  // namespace myapp
