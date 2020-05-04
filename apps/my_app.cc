// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>

#include "CinderImGui.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/draw.h"
#include "cinder/gl/gl.h"
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

  //button allows you to search through files for a picture and loads it
  if (ui::Button("Find Picture", ImVec2(200, 150))) {
    //tries to load the picture chosen
    try {
      fs::path path = getOpenFilePath("", ImageIo::getLoadExtensions());
      if (!path.empty()) {
        whole_picture = Surface(loadImage(path));
        mTexture = gl::Texture::create(whole_picture);
        breakUpPicture();
        resetGame();
      }
    } catch (Exception& exc) {
      CI_LOG_EXCEPTION("failed to load image.", exc);
    }
  }
  //button to scatter pieces around the screen
  ui::SameLine();
  if (ui::Button("Shuffle Pieces", ImVec2(200, 150))) {
    resetGame();
    should_pieces_be_scattered = true;
  }

  ui::NewLine();
  //button to automatically solve the puzzle
  if (ui::Button("Solve", ImVec2(200, 150))) {
    resetGame();
  }
  //will display the opposite of the current mode, and when clicked will switch to the displayed mode
  if (is_hard_mode) {
    ui::SameLine();
    if (ui::Button("Easy Mode", ImVec2(200, 150))) {
      resetGame();
      is_hard_mode = false;
      breakUpPicture();
    }
  } else {
    ui::SameLine();
    if (ui::Button("Hard Mode", ImVec2(200, 150))) {
      resetGame();
      is_hard_mode = true;
      breakUpPicture();
    }
  }
}

void MyApp::fileDrop(FileDropEvent event) {

  try {
    resetGame();
    whole_picture = Surface(loadImage(loadFile(event.getFile(0))));
    mTexture = gl::Texture::create(whole_picture);
    breakUpPicture();
  } catch (Exception &exc) {
    CI_LOG_EXCEPTION("failed to load image: " << event.getFile(0), exc);
  }
}

void MyApp::mouseDown(MouseEvent event) {

  //adjusts coordinates to what the puzzle is measured in to see if the mouse clicked on a piece
  float x = event.getPos().x / kPuzzleScale;
  float y = event.getPos().y / kPuzzleScale;
  ivec2 adjusted_coords(x, y);

  //if no piece has been selected but pieces exist, it goes through all the pieces to find the one being clicked and selects it
  if (selected_piece == nullptr && !pieces.empty()) {
    for (int i = 0; i < pieces.size(); i++) {
      if (pieces.at(i).bounds.contains(adjusted_coords)) {
        selected_piece = &pieces.at(i);
        return;
      }
    }

  } else { // if there is a selected piece then the piece is moved to the spot the mouse is in
    selected_piece->bounds.offsetCenterTo(adjusted_coords);

    //if the click was in the frame and the selected piece goes in that spot, it adds the piece to the frame and stops drawing it separately
    if (whole_pic_rect.contains(event.getPos())) {
      if (pieceIsInCorrectSpot(selected_piece)) {
        num_pieces_locked++;

        //when all the pieces are locked in, the game is over
        if (num_pieces_locked >= (num_pieces_x * num_pieces_y) && !game_over) {
          game_over = true;
          last_color_change = std::chrono::system_clock::now();
        }
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

void MyApp::draw() {

  cinder::gl::enableAlphaBlending();
  cinder::gl::clear();

  if (should_pieces_be_scattered) {
    drawPiecesScattered();
    drawPuzzleFrame();
    drawMiniViewPicture();
  } else {
    drawPicture();
    drawMiniViewPicture();
  }
  if (game_over) {
    renderTextBox();
    gl::draw(game_over_texture);
  }
}


void MyApp::drawPicture() {

  gl::clear(Color(0.5f, 0.5f, 0.5f));
  gl::enableAlphaBlending();

  if (mTexture) {
    whole_pic_rect = Rectf(mTexture->getBounds())
        .getOffset(ivec2(kXFrameOffset, kYFrameOffset))
        .scaled(kPuzzleScale);
    gl::draw(mTexture, whole_pic_rect);
  }
}


void MyApp::drawPiecesScattered() {

  gl::clear(Color(0.5f, 0.5f, 0.5f));
  gl::enableAlphaBlending();

  if (!pieces.empty()) {
    //goes through each piece and if it hasn't been scattered, it puts the piece's bounding rectangle in a random place then draws the piece.
    //If the piece has been scattered once, it is drawn in the same place as before
    //If the piece has been locked into the frame, then it won't be drawn in its own bounding rectangle
    for (int i = 0; i < pieces.size(); i++) {
      PuzzlePiece& piece = pieces.at(i);

      if (!has_scattered_already) {
        piece.bounds.moveULTo(ivec2(random.nextInt(5000),
            random.nextInt(2500)));
      }
      if (!piece.is_in_frame) {
        gl::draw(piece.texture, piece.bounds.scaled(kPuzzleScale));
      }
    }
    has_scattered_already = true;
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

void MyApp::drawPuzzleFrame() {

  Path2d path;
  //draws an outline around the frame
  path.moveTo(whole_pic_rect.getUpperLeft());
  path.lineTo(whole_pic_rect.getLowerLeft());
  path.lineTo(whole_pic_rect.getLowerRight());
  path.lineTo(whole_pic_rect.getUpperRight());
  path.close();

  gl::draw(path);
  gl::TextureRef texture = gl::Texture::create(pic_in_frame);
  gl::draw(texture, whole_pic_rect);
}

void MyApp::breakUpPicture() {

  pieces.clear();

  //gets the best number of pieces based on the picture dimensions and difficulty of puzzle needed
  num_pieces_x = getOptimalNumPieces(whole_picture.getWidth());
  num_pieces_y = getOptimalNumPieces(whole_picture.getHeight());
  piece_width = whole_picture.getWidth() / num_pieces_x;
  piece_height = whole_picture.getHeight() / num_pieces_y;

  //goes through the picture, incrementing by the calculated piece width and height
  for (int y = 0; y < whole_picture.getHeight(); y = y + piece_height) {
    for (int x = 0; x < whole_picture.getWidth(); x = x + piece_width) {

      //sets up a piece with its correct texture, bounds, and index within the "grid" of the puzzle
      Surface new_piece_surface(piece_width, piece_height, true);
      gl::TextureRef texture = getPieceTexture(x, y, x + piece_width,
          y + piece_height, new_piece_surface);
      Rectf piece_rect = Rectf(texture->getBounds());

      PuzzlePiece new_piece = PuzzlePiece(texture, piece_rect,
          x / piece_width, y / piece_height);
      new_piece.bounds_in_frame = Rectf(x, y, x + piece_width,
          y + piece_height);
      pieces.push_back(new_piece);
    }
  }
}

int MyApp::getOptimalNumPieces(int length) {

  if (is_hard_mode) {
    for (int num = kHardModeMinPieces; num <= kHardModeMaxPieces; num++) {
      if (length % num == 0) {
        return num;
      }
    }
  } else {
    for (int num = kEasyModeMinPieces; num <= kEasyModeMaxPieces; num++) {
      if (length % num == 0) {
        return num;
      }
    }
  }
  //if the picture's dimensions are awkward and none of the numbers of pieces within the given ranges work, the default is returned
  return kDefaultNumPieces;
}


gl::TextureRef MyApp::getPieceTexture(int start_x, int start_y, int end_x, int end_y,
                               Surface& result_surface) {
  //copies the picture, pixel by pixel, onto the given surface and creates a texture from it
  for (int x = 0; x < end_x - start_x; x++) {
    for (int y = 0; y < end_y - start_y; y++) {
      result_surface.setPixel(ivec2(x, y),
          whole_picture.getPixel(ivec2(x + start_x, y + start_y)));
    }
  }
  gl::TextureRef to_return = gl::Texture::create(result_surface);
  return to_return;
}


bool MyApp::pieceIsInCorrectSpot(PuzzlePiece* piece) {

  return (piece->bounds_in_frame.contains(ivec2(
      piece->bounds.getCenter().x - kXFrameOffset,
      piece->bounds.getCenter().y - kYFrameOffset)));
}

void MyApp::fillInPieceInFrame(int x1, int y1, int x2, int y2) {

  for (int pic_x = x1; pic_x < x2; pic_x++) {
    for (int pic_y = y1; pic_y < y2; pic_y++) {
      pic_in_frame.setPixel(ivec2(pic_x, pic_y),
          whole_picture.getPixel(ivec2(pic_x, pic_y)));
    }
  }
}

void MyApp::renderTextBox() {

  //every second, the text color is changed to a new, random color
  if (std::chrono::duration_cast <std::chrono::milliseconds>
      (std::chrono::system_clock::now() - last_color_change).count() >=
      1000) {
    text_r = random.nextFloat(1);
    text_g = random.nextFloat(1);
    text_b = random.nextFloat(1);
    last_color_change = std::chrono::system_clock::now();
  }

  TextBox tbox = TextBox().alignment( TextBox::CENTER).font(kGameOverFont).
      size( ivec2( 2900,1000 ) ).text(kGameOverText);
  tbox.setColor( Color( text_r, text_g, text_b ) );
  tbox.setBackgroundColor( ColorA( 0.5, 0.5, 0.5, 1 ) );
  ivec2 sz = tbox.measure();
  CI_LOG_I( "text size: " << sz );
  game_over_texture = gl::Texture2d::create( tbox.render() );
}


void MyApp::resetFrame() {

  if (mTexture) {
    pic_in_frame = Surface(mTexture->getWidth(), mTexture->getHeight(), true);
    for (int x = 0; x < pic_in_frame.getWidth(); x++) {
      for (int y = 0; y < pic_in_frame.getHeight(); y++) {
        pic_in_frame.setPixel(ivec2(x, y), ColorA(.3f, .3f, .3f));
      }
    }
  }
}

void MyApp::resetGame() {

  has_scattered_already = false;
  should_pieces_be_scattered = false;
  resetFrame();
  num_pieces_locked = 0;
  game_over = false;
  for (int i = 0; i < pieces.size(); i++) {
    pieces.at(i).is_in_frame = false;
  }
}

}  // namespace myapp
