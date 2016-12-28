#ifndef AOS_VISION_COMP_GEO_SEGMENT_H_
#define AOS_VISION_COMP_GEO_SEGMENT_H_

#include <cmath>

#include "aos/vision/comp_geo/vector.h"

namespace aos {
namespace vision {
namespace comp_geo {

// two dimensional vector.
template<int Size>
class Segment {
 public:
  Segment() : A_(), B_() { }

  Segment(Vector<Size> A, Vector<Size> B): A_(A), B_(B) { }

  Segment(double ax, double ay, double az, double bx, double by, double bz)
      : A_(ax, ay, az), B_(bx, by, bz) {}

  ~Segment() { }

  void Set(Vector<Size> a, Vector<Size> b) {
    A_ = a;
    B_ = b;
  }

  Vector<Size> A() const { return A_; }
  Vector<Size> B() const { return B_; }

  Vector<Size> AsVector() const { return B_ - A_; }
  Vector<Size> AsNormed() const { return AsVector().normed(); }

  Vector<Size> Center () const { return (A_ + B_).Scale(0.5); }

  // Fast part of length.
  double MagSqr() {
   return (AsVector()).MagSqr();
  }

  // Length of the vector.
  double Mag() { return std::sqrt(MagSqr()); }
  
  Segment<Size> Scale(const double &other) {
    return Segment<2>(A_.Scale(other), B_.Scale(other));
  }

  Vector<2> Intersect(const Segment<2> &other) {
    double x1 = A_.x();
    double y1 = A_.y();
    double x2 = B_.x();
    double y2 = B_.y();

    double x3 = other.A().x();
    double y3 = other.A().y();
    double x4 = other.B().x();
    double y4 = other.B().y();

    // check wikipedia on how to intersect two lines.
    double xn =
        (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
    double xd = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    double yn =
        (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);
    double yd = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    return Vector<2>(xn / xd, yn / yd);
  }

 private:
  // Begining point.
  Vector<Size> A_;

  // Ending point.
  Vector<Size> B_;
};

class Corner {
 public:
  Vector<2> st;
  Vector<2> pt;
  Vector<2> ed;
};

inline Corner intersect(const Segment<2>& a, const Segment<2>& b) {
	Vector<2> g = a.A() - b.A();
	Vector<2> d1 = a.B() - a.A();
	Vector<2> d2 = b.B() - b.A();
	
	double det = (d2.x() * d1.y() - d2.y() * d1.x());
	double alpha = cross_pts(g, d2) / det;
	double beta = -cross_pts(g, d1) / det;
  Corner out;
	out.pt = (alpha * d1) + a.A();

//  printf("err: %g %g\n", alpha, beta);
  
  out.st = (std::abs(alpha) < 0.5) ? a.B() : a.A();

  out.ed = (std::abs(beta) < 0.5) ? b.B() : b.A();

  if (cross_pts(out.st - out.pt, out.ed - out.pt) < 0) {
    std::swap(out.st, out.ed);
  }
  return out;
}

}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_COMP_GEO_VECTOR_H_
