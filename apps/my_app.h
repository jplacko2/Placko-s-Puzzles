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

struct PuzzlePiece {

  PuzzlePiece() = default;
  PuzzlePiece(gl::TextureRef& t, Rectf& r) {
    texture = t;
    bounds = r;
  }
  gl::TextureRef texture;
  Rectf bounds;
  bool is_in_frame = false;
};

class MyApp : public cinder::app::App {
 public:
  MyApp();
  void setup() override;
  void update() override;
  void draw() override;
  void mouseDown(MouseEvent event) override;
  void fileDrop( FileDropEvent event ) override;


  gl::TextureRef mTexture;
  Rectf whole_pic_rect;
  Surface whole_picture;
  std::vector<PuzzlePiece> pieces;
  bool is_jigsaw_mode = true;
  int num_pieces_x;
  int piece_width;
  int num_pieces_y;
  int piece_height;
  bool should_shuffle = false;
  bool has_shuffled_already = false;
  PuzzlePiece* selected_piece = nullptr;
  Rand random;
  const float kPieceScale = .35f;
  const float kPicScale = .8f;

 private:
  void drawPicture();
  void breakUpPicture();
  int getOptimalNumPieces(int length);
  void drawPiecesScattered();
  void drawMiniViewPicture();
  gl::TextureRef getPieceTexture(int start_x, int start_y, int end_x, int end_y,
                                 Surface& result_surface);
  void drawPuzzleBorder();
  void drawPieceAdjustedToBoard(PuzzlePiece* piece);
};


}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
