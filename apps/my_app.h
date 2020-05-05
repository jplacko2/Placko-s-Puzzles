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
#include "cinder/Surface.h"
#include "cinder/Rand.h"

namespace myapp {

using namespace ci;
using namespace ci::app;
using namespace std;

//Represents a piece of the puzzle
struct PuzzlePiece {
  //default PuzzlePiece constructor
  PuzzlePiece() = default;
  //PuzzlePiece constructor taking almost every variable as an argument
  PuzzlePiece(gl::TextureRef& t, Rectf& r, int x_ind, int y_ind) {
    texture = t;
    bounds = r;
    x_index = x_ind;
    y_index = y_ind;
  }
  //holds the picture that the piece displays
  gl::TextureRef texture;
  //the base that the texture is applied to, is shifted when pieces are moved
  Rectf bounds;
  //when the piece is locked in, this becomes true
  bool is_in_frame = false;
  //holds the x index if the picture were a grid of pieces
  int x_index;
  //holds the y index if the picture were a grid of pieces
  int y_index;
  //This holds where the piece should be located when it is in the correct position
  Rectf bounds_in_frame;
};

class MyApp : public cinder::app::App {
 public:
  MyApp();

  //Overridden functions from cinder
  void setup() override;
  void update() override;
  void draw() override;
  void mouseDown(MouseEvent event) override;
  void fileDrop( FileDropEvent event ) override;

  //keep track of the components of the whole picture, the frame, and all the pieces
  gl::TextureRef mTexture;
  Rectf whole_pic_rect;
  Surface whole_picture;
  Surface pic_in_frame;
  std::vector<PuzzlePiece> pieces;

  //the variables associated with the color changing "Puzzle Completed" text displayed after winning the game
  const Font kGameOverFont = Font("Comic Sans MS", 100);
  gl::TextureRef game_over_texture;
  const std::string kGameOverText = "Puzzle \n Completed";
  std::chrono::time_point<std::chrono::system_clock> last_color_change;
  float text_r;
  float text_g;
  float text_b;

  //keeps track of the amount of pieces in each direction and the size of a piece in each direction
  int num_pieces_x;
  int piece_width;
  int num_pieces_y;
  int piece_height;

  //constants for deciding the best amount of pieces in a direction based on side length and required difficulty
  const int kHardModeMinPieces = 10;
  const int kHardModeMaxPieces = 40;
  const int kEasyModeMinPieces = 5;
  const int kEasyModeMaxPieces = 10;
  const int kDefaultNumPieces = 1;

  //variables related to game states
  bool is_hard_mode = true;
  bool should_pieces_be_scattered = false; // pieces should be scattered as long as the puzzle is not solved
  bool has_scattered_already = false; // this should only be true once, then the pieces become stationary unless moved by the user
  PuzzlePiece* selected_piece = nullptr;
  int num_pieces_locked = 0;
  bool game_over = false;

  //constants used in building the frame
  const float kPuzzleScale = .3f;
  const int kXFrameOffset = 400;
  const int kYFrameOffset = 125;

  //random number generator
  Rand random;

 private:
  //draws the whole picture in the designated location
  void drawPicture();
  //draws the space in the same spot of where the whole picture was where the user should place pieces
  void drawPuzzleFrame();
  //draws all the pieces scattered around the screen
  void drawPiecesScattered();
  //draws a miniature version of the whole picture in one corner for reference when solving the puzzle
  void drawMiniViewPicture();

  //breaks the picture into PuzzlePiece objects and fills the pieces vector
  void breakUpPicture();
  //gets the number of pieces for the given side length that should be used so that all pieces have the same size
  int getOptimalNumPieces(int length);

  //gets the texture for a specific piece based on the given bounds and a surface to copy onto
  gl::TextureRef getPieceTexture(int start_x, int start_y, int end_x, int end_y,
                                 Surface& result_surface);
  //returns true if the center of the piece is somewhere within the bounds of the rectangle that represents its correct location in the frame
  bool pieceIsInCorrectSpot(PuzzlePiece* piece);

  //resets the frame to the dark gray color with no pieces in it
  void resetFrame();
  //fills in the bounds within the frame with the pixels of the piece that goes there
  void fillInPieceInFrame(int x1, int y1, int x2, int y2);
  //renders all the specific elements (color, font, background color, location, size, etc.) of the game over text box
  void renderTextBox();

  //resets several pieces of game state and also the state of each piece
  void resetGame();
};


}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
