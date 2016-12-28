#include "aos/vision/checker_board.h"
#include "eigen3/Eigen/Dense"

#include <limits>
#include <math.h>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>

#include "aos/vision/comp_geo/vector.h"
#include "aos/vision/comp_geo/segment.h"
#include "aos/vision/blobs.h"

namespace aos {
namespace vision {

using namespace comp_geo;

inline int pxdiff(PixelRef a, PixelRef b) {
  return ((int)a.r + (int)a.g + (int)a.b) - ((int)b.r + (int)b.g + (int)b.b);
}

inline bool flipped(PixelRef a, PixelRef b) {
  int pxdiffv = pxdiff(a, b);
  return pxdiffv > 50 || pxdiffv < -50;
}

template<int dir>
ImgCursor TrackVert(const ImagePtr& img, const ImagePtr& dbg, ImgCursor seed, PixelRef start) {
  for (int i = 0; i < 150; i++) {
    seed.y += dir;
    dbg.get_px(seed.x, seed.y) = {0, 255, 0};
    if (flipped(start, img.get_px(seed.x - 4, seed.y + dir * 2)) && i >= 1) break;
    if (flipped(start, img.get_px(seed.x, seed.y))) {
      seed.x --;
    }
    if (!flipped(start, img.get_px(seed.x, seed.y))) {
      seed.x ++;
    }
  }
  return seed;
}

template<int dir, int s_side>
ImgCursor TrackHoriz(const ImagePtr& img, const ImagePtr& dbg,
                     ImgCursor seed, PixelRef start, PixelRef dbgval) {
  for (int i = 0; i < 150; i++) {
    seed.x += dir;
    dbg.get_px(seed.x, seed.y) = dbgval;
    if (flipped(start, img.get_px(seed.x + dir * 2, seed.y - s_side * 4)) && i >= 1) break;
    if (flipped(start, img.get_px(seed.x, seed.y))) {
      seed.y -= s_side;
    }
    if (!flipped(start, img.get_px(seed.x, seed.y))) {
      seed.y += s_side;
    }
  }
  return seed;
}


template <int dx, int dy>
void EdgeFind(const ImagePtr& img, int x, int y, 
              PixelRef sum_color, std::vector<Vector<2>>* pts) {
  bool a = pxdiff(img.get_px(x, y), sum_color) > 0;
  bool b = pxdiff(img.get_px(x + dx, y + dy), sum_color) > 0;
  if (a != b) {
    //printf("%d %d\n", x, y);
    pts->emplace_back(x + 0.5 * dy, y + 0.5 * dy);
  }
}

bool dbg_corner(const ImagePtr& dbg, const ImagePtr& img, ImgCursor cur, Vector<2>* pt) {
  (void) dbg;
  const int corner_size = 14;
  const int corner_size_sample = 8;
  PixelRef sum_color;
  PxSum sum;
  if (cur.x < corner_size || cur.y < corner_size ||
      cur.x >= img.fmt.w - corner_size || cur.y >= img.fmt.h - corner_size) return false;
  for (int i = -corner_size_sample; i <= corner_size_sample; ++i) {
  for (int j = -corner_size_sample; j <= corner_size_sample; ++j) {
    sum.add_px(img.get_px(cur.x + i, cur.y + j));
  }
  }

  sum_color = sum.get_avg();
  for (int k = 0; k < 3; ++k) {
  std::vector<Vector<2>> pts;
  for (int i = -corner_size; i < corner_size; ++i) {
    EdgeFind<1, 0>(img, cur.x + i, cur.y + corner_size, sum_color, &pts);
  }
  for (int i = -corner_size; i < corner_size; ++i) {
    EdgeFind<0, 1>(img, cur.x + corner_size, cur.y + i, sum_color, &pts);
  }
  for (int i = -corner_size; i < corner_size; ++i) {
    EdgeFind<-1, 0>(img, cur.x - i, cur.y - corner_size, sum_color, &pts);
  }
  for (int i = -corner_size; i < corner_size; ++i) {
    EdgeFind<0, -1>(img, cur.x - corner_size, cur.y - i, sum_color, &pts);
  }

  if (pts.size() != 4) return false;

  Segment<2> a (pts[0], pts[2]);
  Segment<2> b (pts[1], pts[3]);
  *pt = intersect(a, b).pt;
  cur.x = pt->x();
  cur.y = pt->y();
    if (cur.x < corner_size || cur.y < corner_size ||
        cur.x >= img.fmt.w - corner_size || cur.y >= img.fmt.h - corner_size) return false;
  }

  return true; 
}
void dbg_corner(const ImagePtr& dbg, const ImagePtr& img, ImgCursor cur) {
  Vector<2> pt;
  dbg_corner(dbg, img, cur, &pt);
}

class GridCellRef {
 public:
  Vector<2> img_pos;
  bool located = false;
  bool location_error = false;
};

class Range1D {
 public:
  typedef int RangeType;
  RangeType min = std::numeric_limits<RangeType>::max();
  RangeType max = std::numeric_limits<RangeType>::min();
  void add(RangeType item) {
    max = std::max(item, max);
    min = std::min(item, min);
  }
  RangeType begin() { return min; }
  RangeType end() { return max + 1; }
};

inline Vector<2> homogTrans(const Eigen::MatrixXd& mat, Vector<2> pt) {
  Eigen::VectorXd point(3);
  point(0) = pt.x();
  point(1) = pt.y();
  point(2) = 1.0;
  Eigen::VectorXd res = mat * point;
  return Vector<2>(res(0) / res(2), res(1) / res(2) );
}

class AllCornerGridFinder {
 public:
  typedef std::pair<int, int> grid_cell;

  bool test_ncell(std::map<grid_cell, GridCellRef>::iterator val) {
    if (val == cells.end()) return false;
    return val->second.located;
  }

  void set_to_nexpand(Vector<2> guess, grid_cell cell, int dx, int dy) {
    grid_cell neigh {cell.first + dx, cell.second + dy};
    auto val = cells.find(neigh);
    if (val != cells.end() && val->second.located) return;
    cells[neigh].img_pos = guess;
      to_expand.emplace_back(neigh);
  }

  void test_ncell_pair(grid_cell cell, int dx, int dy) {
    auto a = cells.find(grid_cell {cell.first + dx, cell.second + dy});
    auto b = cells.find(grid_cell {cell.first - dx, cell.second - dy});
    bool ab = test_ncell(a);
    bool bb = test_ncell(b);
    //printf(" ab: %s\n", ab ? "true" : "false");
    //printf(" bb: %s\n", bb ? "true" : "false");
    if (ab && !a->second.location_error) {
      set_to_nexpand(((2 * cells[cell].img_pos) - a->second.img_pos), cell, -dx,
                     -dy);
      set_to_nexpand(((2 * a->second.img_pos) - cells[cell].img_pos), cell,
                     2 * dx, 2 * dy);
    } 
    
    if (bb && !b->second.location_error){
      set_to_nexpand(((2 * cells[cell].img_pos) - b->second.img_pos), cell, dx,
                     dy);
      set_to_nexpand(((2 * b->second.img_pos) - cells[cell].img_pos), cell,
                     -2 * dx, -2 * dy);
    }
  }

  void add_pt(grid_cell cell, Vector<2> pt) {
    auto& cellv = cells[cell];
    cellv.img_pos = pt;
    cellv.located = true;
    test_ncell_pair(cell, 1, 0);
    test_ncell_pair(cell, 0, 1);
    nfound++;
  }

  void expand_all(const ImagePtr& dbg, const ImagePtr& img) {
    printf("expand_all start:\n");
    while (not_empty()) {
      --expand_limit;
      grid_cell cell = to_expand.back();
      to_expand.pop_back();
      auto& cellv = cells[cell];
      if (cellv.located) { continue; }
      Vector<2> pt;

      ImgCursor guess { (int)cellv.img_pos.x(), (int)cellv.img_pos.y() };
      if (dbg_corner(dbg, img, guess, &pt)) {
        //printf("expand success: <%d, %d> (<%g, %g>)\n", cell.first, cell.second, cellv.img_pos.x, cellv.img_pos.y);
        add_pt(cell, pt);
      } else {
        cellv.located = true;
        cellv.location_error = true;
        //printf("expand error: <%d, %d> (<%g, %g>)\n", cell.first, cell.second, cellv.img_pos.x, cellv.img_pos.y);
      }
      /*
      printf("to_expand:\n");
      for (grid_cell cel : to_expand) {
        printf("  <%d, %d>\n", cel.first, cel.second);
      }
      */
    }
    getTransformation(dbg);
    printf("expanding done: %d\n", expand_limit);
  }

  Eigen::MatrixXd getTransformation(const ImagePtr& dbg) {
    Eigen::MatrixXd A(nfound * 2, 9);
    int i = 0;
    Range1D gridx;
    Range1D gridy;
    for (const auto& cell : cells) {
      if (cell.second.location_error || !cell.second.located) continue;
      int x2 = cell.first.first;
      int y2 = cell.first.second;
      gridx.add(x2);
      gridy.add(y2);
      double x1 = cell.second.img_pos.x();
      double y1 = cell.second.img_pos.y();

      A(i, 6) = x1 * x2;
      A(i, 7) = y1 * x2;
      A(i, 8) = 1 * x2;
      A(i, 0) = -x1;
      A(i, 1) = -y1;
      A(i, 2) = -1;

      A(i, 3) = 0.0;
      A(i, 4) = 0.0;
      A(i, 5) = 0.0;
      i++;
      A(i, 6) = x1 * y2;
      A(i, 7) = y1 * y2;
      A(i, 8) = 1 * y2;
      A(i, 3) = -x1;
      A(i, 4) = -y1;
      A(i, 5) = -1;

      A(i, 0) = 0.0;
      A(i, 1) = 0.0;
      A(i, 2) = 0.0;
      i++;
    }

    Eigen::MatrixXd G = A.transpose() * A;
    //std::cout << "G: " << G << std::endl;
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(G, Eigen::ComputeFullU | Eigen::ComputeFullV);
    auto matv = svd.matrixV();
    Eigen::MatrixXd trans_out(3, 3);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        trans_out(i, j) = matv(i * 3 + j, 8);
      }
    }

    auto trans = trans_out.inverse();
   
    gridy.min = 2;
    gridx.min = 2;
    for (int x = gridx.min; x <= gridx.max; ++x) {
      auto px1 = homogTrans(trans, {(double)x, (double)gridy.max + 2});
      auto px2 = homogTrans(trans, {(double)x, (double)gridy.min - 2});
      draw_bresham_line({(int)px1.x(), (int)px1.y()},
                        {(int)px2.x(), (int)px2.y()}, dbg, {127, 0, 255});
    }
    
    for (int y = gridy.min; y <= gridy.max; ++y) {
      auto px1 = homogTrans(trans, {(double)gridx.max + 2, (double)y});
      auto px2 = homogTrans(trans, {(double)gridx.min - 2, (double)y});
      draw_bresham_line({(int)px1.x(), (int)px1.y()},
                        {(int)px2.x(), (int)px2.y()}, dbg, {127, 0, 255});
    }


    Eigen::MatrixXd transl(3, 3);

    for (int i = 0; i < 3; i++) { for (int j = 0; j < 3; j++) { transl(i, j) = 0; } }
    transl(0, 1) = 0.5 * 0.0254;
    transl(1, 0) = -0.5 * 0.0254;
    transl(2, 2) = 1.0;
    transl(0, 2) = 12.5 * 0.0254;

//    std::cout << "Transl: \n" << transl << std::endl;
    Eigen::MatrixXd matrans = transl * trans_out;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        printf("%g, ", -matrans(i, j));
      }
    }
    printf("\n");
    std::cout << "TransOut: \n" << -1 * transl * trans_out << std::endl;

    return trans_out;
  }

  bool not_empty() { return to_expand.size() > 0 && expand_limit > 0; }

  int expand_limit = 1024;
  int nfound = 0;

  std::vector<grid_cell> to_expand;
  std::map<grid_cell, GridCellRef> cells;
};

void SanitizeAndExpandCorners(const ImagePtr& dbg, const ImagePtr& img, ImgCursor corners[4]) {
  Vector<2> corpts[4];
  for (int i = 0; i < 4; i++) { if (!dbg_corner(dbg, img, corners[i], &corpts[i])) return; }

  Vector<2> corptguess;
  Vector<2> corpt;

  AllCornerGridFinder grid;
  grid.add_pt({1, 1}, corpts[0]);
  grid.add_pt({1, 0}, corpts[1]);
  grid.add_pt({0, 1}, corpts[2]);
  grid.add_pt({0, 0}, corpts[3]);
  grid.expand_all(dbg, img);
}

void FindCheckers(const ImagePtr& img, const ImagePtr& dbg) {
  int xseed = img.fmt.w / 2;
  int yseed = img.fmt.h / 2;
  //printf("= ");
  PixelRef start = img.get_px(xseed, yseed);
  ++xseed;
  for (int i = 0; i < 150; i++) {
    dbg.get_px(xseed, yseed) = {255, 0, 0};
    if (flipped(start, img.get_px(xseed, yseed))) {
      ImgCursor a = TrackVert<-1>(img, dbg, {xseed, yseed}, start);
      ImgCursor c = TrackHoriz<-1, -1>(img, dbg, a, start, {0, 0, 255});
      ImgCursor b = TrackVert<1>(img, dbg, {xseed, yseed}, start);
      ImgCursor d = TrackHoriz<-1, 1>(img, dbg, b, start, {0, 255, 255});

    //  printf("\n");
      ImgCursor corners[4] = {a, b, c, d};
      SanitizeAndExpandCorners(dbg, img, corners);

      return;
    }
  //  printf("%d, ", pxdiff(start, img.get_px(xseed, yseed)));
    ++xseed;
  }
//  printf("\n");

}

}  // namespace vision
}  // namespace aos
