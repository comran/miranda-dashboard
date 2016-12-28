//#define NBOUNDSCHECK
#include "aos/vision/image/image_stream_legacy.h"
#include "eigen3/Eigen/Dense"

#include <limits>
#include <math.h>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>

#include "gflags/gflags.h"

#include "aos/vision/blobs.h"
#include "aos/vision/comp_geo/vector.h"
#include "aos/vision/robot.h"

namespace aos {
namespace vision {

using namespace comp_geo;

struct Overlays {
  PixelLinesOverlay* base;
  LinesOverlay* norm;
  LinesOverlay* pairs;
};

Vector<2> pt(Pt ptv) {
  return Vector<2>((double)ptv.x, (double)ptv.y);
}

Vector<2> transform_old(Vector<2> pt) {
//  double a = pt.y * 2.03148564e-03 - pt.x * 8.13350411e-04 - 7.01416298e-02;
//  double b = pt.y * 2.12167295e-03 + pt.x * 1.00524761e-03 - 9.96242030e-01;
//  double d = pt.y * 9.92279504e-05 + pt.x * 3.00513450e-06 + 5.07115373e-02;
  double a = pt.y() * 0.00118215322 + pt.x() * 0.00190612042 - 0.543996916;
  double b = pt.y() * 0.00442680419 + pt.x() * 6.14064483e-05 - 0.835188053;
  double d = pt.y() * 0.000145301432 + pt.x() * 2.52652918e-06 + 0.0806450238;

  return Vector<2>(a / d, b / d);
}


/*
-8.60624e-07 -4.50468e-06   -0.0322394
 1.35818e-05  5.38608e-07  -0.00858187
-2.52802e-06 -7.00152e-05   -0.0721976
*/

const double trans_data[] = {
//3.22123e-07, 3.40167e-06, 0.0289839, -1.32464e-05, -2.76894e-07, 0.00815036, 1.29067e-06, 6.93678e-05, 0.062592,
4.43301e-07, 3.31204e-06, 0.03013, -1.39657e-05, -7.65994e-07, 0.008365, 1.4215e-06, 7.39933e-05, 0.0669271, 
};

Vector<2> transform(Vector<2> pt) {
  double a = pt.x() * trans_data[0] + pt.y() * trans_data[1] + trans_data[2];
  double b = pt.x() * trans_data[3] + pt.y() * trans_data[4] + trans_data[5];
  double d = pt.x() * trans_data[6] + pt.y() * trans_data[7] + trans_data[8];
/*
  double a = pt.x * 8.60624e-07 + pt.y * 4.50468e-06 + 0.0322394;
  double b = -pt.x * 1.35818e-05 - pt.y * 5.38608e-07 + 0.00858187;
  double d = pt.x * 2.52802e-06 + pt.y * 7.00152e-05 + 0.0721976;
*/

  return Vector<2>(a / d, b / d);
}

Vector<2> rot90(const Vector<2>& a) { return Vector<2>(a.y(), -a.x()); }
Vector<2> norm_vector(const Segment<2>& line) { return rot90(line.AsNormed()); }

bool get_dist_cross(const Segment<2>& a, const Segment<2>& b, double* dist) {
  Vector<2> apt = a.AsNormed();
  Vector<2> bpt = b.AsNormed();

  double norm_cross = cross_pts(apt, bpt);
  
  if (std::abs(norm_cross) > 0.04) return false;
  if (apt.dot(bpt) < 0.0) { apt.x(-apt.x()); apt.y(-apt.y()); }
  Vector<2> avg = Vector<2>(apt.x() + bpt.x(), apt.y() + bpt.y()).normed();
  Vector<2> rot = rot90(avg); 
  *dist = std::abs(rot.dot(a.A()) - rot.dot(b.A()));
  return true;
}

Vector<2> bilin_interp(Vector<2> refpts[4], double a, double b) {
  return (((b * a) * refpts[0]) + ((b * (1-a)) * refpts[1])) +
             (((1 - b) * a) * (refpts[2]) + (((1 - b) * (1-a)) * refpts[3]));
}

Vector<2>rot_point(double angle, const Vector<2>& a) {
  double s = sin(angle);
  double c = cos(angle);
  return Vector<2>(a.x() * c - a.y() * s, a.x() * s + a.y() * c);
}

double reset_angle(double angle) {
  if (angle > M_PI) { angle -= 2 * M_PI; }
  if (angle < -M_PI) { angle += 2 * M_PI; }
  return angle;
}
void pair_lines(std::vector<Segment<2>> lines, Overlays ov) {
  for (int i = 0; i < (int)lines.size();) {
    double magsqr = lines[i].MagSqr();
    if (0.0254 * 0.0254 >= magsqr) {
      lines.erase(lines.begin() + i);
    } else {
      ++i;
    }
  }

  std::vector<Segment<2>> pairs;
  for (int i = 0; i < (int)lines.size(); ++i) {
    ov.base->add_line(lines[i].A(), lines[i].B());

    Vector<2> norm = norm_vector(lines[i]);

    ov.norm->add_line((0.0254 * norm + lines[i].Center()), lines[i].Center());

    for (int j = i + 1; j < (int)lines.size(); ++j) {
      double dist;
      
      if (get_dist_cross(lines[i], lines[j], &dist)) {
        if (std::abs(dist - 1.90 * 0.5 * 0.0254) < 0.2 * 0.5 * 0.0254) {
          pairs.emplace_back(Segment<2>(lines[i].A(), lines[j].B()).Center(),
                             Segment<2>(lines[i].B(), lines[j].A()).Center());
          ov.pairs->add_line(lines[i].Center(), lines[j].Center());
          //printf("%d, %d: %g\n", i, j, dist);
        }
      }
    }
  }
  if (pairs.size() != 2) {
    if (pairs.size() >= 2) {
      printf("too many %d\n", (int)pairs.size());
    }
    return;
  }
  Vector<2> a = norm_vector(pairs[0]);
  Vector<2> b = norm_vector(pairs[1]);
  if (std::abs(a.dot(b)) < 0.02) {
    Corner corner = intersect(pairs[0], pairs[1]);
    ov.pairs->add_line(corner.st, corner.pt);
    ov.pairs->add_line(corner.pt, corner.ed);
    double angle = (corner.ed - corner.pt).AngleToZero();
    double angle2 = (corner.st - corner.pt).AngleToZero();
    
    angle = reset_angle(M_PI / 2 - (angle + M_PI));
    angle2 = reset_angle(M_PI / 2 - (angle2 + M_PI));
    
    auto endpt = rot_point(angle, corner.pt);

    printf("[%g, %g, %g, %g],\n", endpt.x() / 0.0254, endpt.y() / 0.0254,
           angle * 180.0 / M_PI, angle2 * 180.0 / M_PI);
  }
}
//void HarrisShow(const ImagePtr& img, const GrayImagePtr& 

struct RobotPos {
  Vector<2> pos;
  double theta;

  RobotPos(Vector<2> pos, double theta) : pos(pos), theta(theta) {}
  RobotPos() : pos {0, 0}, theta(0.0) {}

  void Update(double dth, double df) {
    pos += Vector<2> {df * cos(theta), df * sin(theta) };
    theta += dth;
    printf("cur_pos:%g %g %g\n", pos.x(), pos.y(), theta);
  }
};

void do_main(const ImageStreamConfig& cfg) {
  ImageStream stream(cfg);
  printf("fmt: %dx%d\n", stream.fmt.w, stream.fmt.h);
  auto view = GtkView(&stream);
  Overlays overlay = {view->add_pixel_line_overlay(), view->add_line_overlay(), view->add_line_overlay()};
  overlay.base->color = {255, 0, 0};
  overlay.norm->color = {0, 0, 255};
  overlay.pairs->color = {0, 255, 0};

  overlay.base->scale = 20 * 2 / 0.0254;
  overlay.norm->scale = 20 * 2 / 0.0254;
  overlay.pairs->scale = 20 * 2 / 0.0254;
  AnalysisAllocator alloc;
  GrayImageValue ix(stream.fmt);
  GrayImageValue iy(stream.fmt);
  GrayImageValue ix2(stream.fmt);
  GrayImageValue iy2(stream.fmt);
  GrayImageValue ixiy(stream.fmt);
  
  ImageValue guass_img(stream.fmt);

#if 0
  RobotInterface interface;
  //double goal = 1.0;
  RobotPos pos;
  //int i = 0;
  //int k = 0;
  //int p = 0;
  
  RobotPos goal {{ 0.2, 0.2 }, 0.0};

  int64_t timestamp = GetTimeStamp();
  int64_t timestamp_new = timestamp;
  auto old_enc = interface.GetEncoders();
  int k = 0;
  auto watcher = interface.start_watcher([&]() {
    auto enc = interface.GetEncoders();
    //printf("enc: %g, %g\n", enc.left_encoder, enc.right_encoder);
    double dl = enc.left_encoder - old_enc.left_encoder;
    double dr = enc.right_encoder - old_enc.right_encoder;
    double dth = (dr - dl) * 2.6838488120052775;
		double df = -(dl + dr) / 2.0;
    pos.Update(dth, df);
    //printf("pos: %g\n", pos);



    //if (pos < goal) {

    auto diff_vec = goal.pos - pos.pos;
    double errdist = diff_vec.Mag();

    if (errdist < 0.01 || k == 1) {
      k = 1;

      double errangle = goal.theta - pos.theta;
      printf("errangle: %g\n", errangle);
      double wheel_power = errangle * 2000;

      if (wheel_power > 20) { wheel_power = 20; }
      else if (wheel_power < -20) { wheel_power = -20; }

      interface.SendDrivetrain(true, 0, wheel_power); 
    } else {
      double errangle = diff_vec.AngleToZero() - pos.theta;
      double wheel_power = errangle * 2000;
      printf("errangle: %g\n", errangle);
      double fwd_power = 0;
      if (std::abs(wheel_power) < 30.0) {
        fwd_power = 2000.0 * errdist;
      }
      if (wheel_power > 20) { wheel_power = 20; }
      else if (wheel_power < -20) { wheel_power = -20; }
      else if (timestamp == timestamp_new) {
        timestamp_new = GetTimeStamp(); 
      }
      if (fwd_power > 20.0) { fwd_power = 20.0; }

      printf("timestamp_delta: %ld\n", timestamp_new - timestamp);

      interface.SendDrivetrain(true, fwd_power, wheel_power); 
    }
      /*
      int step = 127;

      if (k == 0) {
      i += step;
      if (i >= 127 - step) {
        k = 1;
      }
      } else if (k == 1) {
        ++p;
        if (p == 2) {
          k = 2;
        }
      } else if (k == 2) {
        i -= step;
        if (i == 0) {
          k = 3;
        }
      }
      */
      // : -40, 0);
    // } // else {
    //  interface.SendDrivetrain(true, 0, 0);
   // }
    old_enc = enc;
  });
//#else

  RobotInterface interface;
  double goal = 1.0;
  double pos = 0.0;
  auto old_enc = interface.GetEncoders();
  auto watcher = interface.start_watcher([&]() {
    auto enc = interface.GetEncoders();
    //printf("enc: %g, %g\n", enc.left_encoder, enc.right_encoder);
    double dl = enc.left_encoder - old_enc.left_encoder;
    double dr = enc.right_encoder - old_enc.right_encoder;
    double dth = (dr - dl) * 2.6838488120052775;
    (void)dth;
		double df = -(dl + dr) / 2.0;
    //printf("pos: %g\n", pos);
    pos += df;

    if (pos < goal) {
      interface.SendDrivetrain(true, pos < goal ? 40 : -40, 0);
    } else {
      interface.SendDrivetrain(true, 0, 0);
    }
    old_enc = enc;
  });
#endif

  stream.vision_cb = [&](const ImagePtr& img) {
    //return;
    //return;
    /*
   auto guass_ptr = guass_img.img_ptr();
   guass_ptr.CopyFrom(img);
   FindCheckers(guass_ptr, img);
   return;
   // */
   //return;

    overlay.base->reset();
    overlay.norm->reset();
    overlay.pairs->reset();

    
    /*
    for (PixelRef& px : img) {
      if (px.r + px.g + px.b > 90 * 3) {
        px = {255, 255, 255};
      } else {
        px = {0, 0, 0};
      }
    }
    */



   //img.CopyFrom(&guass_img);

   /*
    auto ita = img.begin();
    auto itb = (&guass_img).begin();
    for (; ita != img.end(); ++ita, ++itb) {
      if ((*ita).r > (*itb).r) {
        *ita = {255, 255, 255};
      } else {
        *ita = {0, 0, 0};
      }
    }
    */
    //return;

    RangeImage rimg = do_threshold(img, 
                [](PixelRef& px) {
      if (px.g > 88) { //149 >= px.r && px.r >= 1 && 198 >= px.g && px.g >= 104 && 255 >= px.b && px.b >= 139) {
        px = {255, 255, 255};
        return true;
      }
      return false;
    });

    BlobList blobl = aos::vision::find_blobs(rimg);

    int max_i = -1;
    int max_npixels = 2000;
    //printf("nblobs : %d\n", (int)blobl.size());
    for (int i = 0; i < (int)blobl.size(); i++) {
      int npixels = blobl[i].npixels();
      //printf("blob: %d : %d\n", i, npixels);
      if (max_npixels < npixels) {
        max_npixels = npixels;
        max_i = i;
      }
    }

    if (max_i >= 0) {
      auto& blob = blobl[max_i];
      int64_t x_sum = 0;
      int64_t y_sum = 0;
      int64_t count = 0;
      for (int i = 0; i < (int)blob.ranges.size(); ++i) {
        for (auto& range : blob.ranges[i]) {
          int bcount = (range.ed - range.st);
          count += bcount;
          y_sum += bcount * i;
          x_sum += ((range.ed + range.st - 1) * bcount) / 2;
        }
      }
      int64_t y = y_sum / count + blob.mini;
      int64_t x = x_sum / count;
//      printf("x: %ld, y: %ld\n", x, y);
      overlay.base->add_line(Vector<2>(x, y - 10), Vector<2>(x, y + 10));
      overlay.base->add_line(Vector<2>(x - 10, y), Vector<2>(x + 10, y));
      //overlay.base->add_line(Vector<2>(0, 0), Vector<2>(x, 200));
    }

/*
    for (int i = 0; i < (int)blobl.size(); i++) {
      int npixels = blobl[i].npixels();
      if ( npixels > 20000 ) {
        draw_range_img(blobl[i], img, {0, 125, 255});
        ContourNode* n = range_img_to_obj(blobl[i], &alloc);

        ContourNode* c = n;
        do {
          img.get_px(c->pt.x, c->pt.y) = {0, 255, 0};
          c = c->next;
        } while (c != n);

        std::vector<FittedLine> lines;
        hier_merge(n, img, &lines);

        std::vector<Segment<2>> lines_list(lines.size());
        std::transform(lines.begin(), lines.end(), lines_list.begin(), [](const FittedLine& line_fit) {
                       return Segment<2>(transform(pt(line_fit.st)), transform(pt(line_fit.ed)));
                    });
        //for (auto line : lines_list) {
        //  overlay.base->add_line(line.st, line.ed);
        //}
        pair_lines(lines_list, overlay);

        alloc.reset();
      } else {
        // printf("reject blob!: %d\n", blobl[i].npixels());
      }
    }
    */

  };
  stream.refresh_rate = 1;
  stream.run();
}

}  // namespace vision
}  // namespace aos

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  aos::vision::ImageStreamConfig config;
  auto argv_copy = (const char**) argv;
  config.ParseFromArgv(&argc, &argv_copy);
  config.LogConfig();
  aos::vision::do_main(config);
  return 0;
}
