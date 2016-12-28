#include "aos/vision/blob/hierarchical_contour_merge.h"

#include <queue>

#include "aos/vision/blob/disjoint_set.h"

namespace aos {
namespace vision {

template <typename T>
class IntegralArray {
 public:
  std::vector<T> items;
  IntegralArray() {}
  IntegralArray(int size) { items.reserve(size); }
  static int mod(int a, int n) { return ((a % n) + n) % n; }

  T get(int a, int b) {
    a = mod(a, items.size());
    b = mod(b, items.size());
    if (b < a) {
      if (b == 0) {
        return items[items.size() - 1] - items[a - 1];
      }
      return items[items.size() - 1] + items[b - 1] - items[a - 1];
    }
    if (a == 0) {
      return items[b - 1];
    } else {
      return items[b - 1] - items[a - 1];
    }
  }
  void add(T t) {
    if (items.size() == 0) {
      items.push_back(t);
    } else {
      items.push_back(t + items[items.size() - 1]);
    }
  }
};

class IntegralLineFit {
 public:
  IntegralLineFit(int number_of_points, int min_line_length)
      : xx(number_of_points),
        xy(number_of_points),
        yy(number_of_points),
        x(number_of_points),
        y(number_of_points),
        n(number_of_points),
        min_len(min_line_length) {}

  void add_pt(Pt pt) {
    xx.add(pt.x * pt.x);
    xy.add(pt.x * pt.y);
    yy.add(pt.y * pt.y);
    x.add(pt.x);
    y.add(pt.y);
  }

  float get_line_error_rate(int st, int ed) {
    int64_t nv = (ed - st + n) % n;

    int64_t px = x.get(st, ed);
    int64_t py = y.get(st, ed);
    int64_t pxx = xx.get(st, ed);
    int64_t pxy = xy.get(st, ed);
    int64_t pyy = yy.get(st, ed);

    double nvsq = nv * nv;
    double m_xx = (pxx * nv - px * px) / nvsq;
    double m_xy = (pxy * nv - px * py) / nvsq;
    double m_yy = (pyy * nv - py * py) / nvsq;

    double b = m_xx + m_yy;
    double c = m_xx * m_yy - m_xy * m_xy;
    return ((b - sqrt(b * b - 4 * c)) / 2.0);
  }

  float get_err_line_range(int st, int ed) {
    int nv = (ed + 1) - st;
    if (ed < st) {
      nv += n;
    }
    int j = std::max(min_len - nv, 0) / 2;
    return get_line_error_rate((st - j + n) % n, (ed + 1 + j + n) % n);
  }

  FittedLine fit_line(int st, int ed, Pt pst, Pt ped) {
    int nv = (ed + 1) - st;
    if (ed < st) {
      nv += n;
    }
    int j = std::max(min_len - nv, 0) / 2;

    st = (st - j + n) % n;
    ed = (ed + 1 + j + n) % n;
    if (nv <= min_len) {
      return FittedLine{pst, pst};
    }

    int64_t px = x.get(st, ed);
    int64_t py = y.get(st, ed);
    int64_t pxx = xx.get(st, ed);
    int64_t pxy = xy.get(st, ed);
    int64_t pyy = yy.get(st, ed);

    double nvsq = nv * nv;
    double m_xx = (pxx * nv - px * px) / nvsq;
    double m_xy = (pxy * nv - px * py) / nvsq;
    double m_yy = (pyy * nv - py * py) / nvsq;
    double m_x = px / ((double)nv);
    double m_y = py / ((double)nv);

    double b = (m_xx + m_yy) / 2.0;
    double c = m_xx * m_yy - m_xy * m_xy;

    double eiggen = sqrt(b * b - c);
    double eigv = b - eiggen;

    double vx = m_xx - eigv;
    double vy = m_xy;
    double mag = sqrt(vx * vx + vy * vy);
    vx /= mag;
    vy /= mag;

    double av = vx * (pst.x - m_x) + vy * (pst.y - m_y);
    double bv = vx * (ped.x - m_x) + vy * (ped.y - m_y);

    Pt apt = {(int)(m_x + vx * av), (int)(m_y + vy * av)};
    Pt bpt = {(int)(m_x + vx * bv), (int)(m_y + vy * bv)};

    // printf("%g, %g, %g %g %d %g\n", m_x, m_y, vx, vy, nv, eiggen);
    return FittedLine{apt, bpt};
    // return FittedLine {pst, ped};
  }
  
  IntegralArray<int> xx;
  IntegralArray<int> xy;
  IntegralArray<int> yy;
  IntegralArray<int> x;
  IntegralArray<int> y;

  // number of points in contour
  int n;

  // minimum line length we will look for
  int min_len;
};

class JoinEvent {
 public:
  int st;
  int ed;
  bool operator<(const JoinEvent & /*o*/) const { return false; }
};

void hier_merge(ContourNode *stval, std::vector<FittedLine> *fit_lines,
                float merge_rate, int min_len) {
  ContourNode *c = stval;
  // count the number of points in the contour.
  int n = 0;
  do {
    n++;
    c = c->next;
  } while (c != stval);
  IntegralLineFit fit(n, min_len);
  c = stval;
  std::vector<Pt> pts;
  do {
    fit.add_pt(c->pt);
    pts.push_back(c->pt);
    c = c->next;
  } while (c != stval);

  DisjointSet ids(n);

  std::vector<int> sts;
  sts.reserve(n);
  std::vector<int> eds;
  eds.reserve(n);
  for (int i = 0; i < n; i++) {
    sts.push_back(i);
    eds.push_back(i);
  }

  std::priority_queue<std::pair<float, JoinEvent>> evnts;
  int plim = 0;
  int nlim = 0;
  for (int i = plim; i < n - nlim; i++) {
    float err = fit.get_err_line_range(i - 1, i);
    evnts.push(std::pair<float, JoinEvent>(err, JoinEvent{(i - 1 + n) % n, i}));
  }

  while (evnts.size() > 0) {
    auto evnt = evnts.top().second;
    evnts.pop();
    int pi1 = ids.find(evnt.st);
    int pi2 = ids.find(evnt.ed);
    int st = sts[pi1];
    int ed = eds[pi2];
    if (st == evnt.st && ed == evnt.ed && pi1 != pi2) {
      ids[pi2] = pi1;
      int pi = sts[ids.find((st - 1 + n) % n)];
      int ni = eds[ids.find((ed + 1 + n) % n)];
      eds[pi1] = ed;
      if (pi >= plim && pi != st) {
        float err = fit.get_err_line_range(pi, ed);
        if (err < merge_rate) {
          evnts.push(std::pair<float, JoinEvent>(err, JoinEvent{pi, ed}));
        }
      }
      if (ni <= n - nlim && ni != ed) {
        float err = fit.get_err_line_range(st, ni);
        if (err < merge_rate) {
          evnts.push(std::pair<float, JoinEvent>(err, JoinEvent{st, ni}));
        }
      }
    }
  }
  for (int i = plim; i < n - nlim; i++) {
    if (ids[i] == -1) {
      int sti = sts[i];
      int edi = eds[i];
      if ((edi - sti + n) % n > min_len) {
        auto line_fit = fit.fit_line(sti, edi, pts[sti], pts[edi]);
        fit_lines->emplace_back(line_fit);
      }
    }
  }
}

// discrite integral for xx.
inline int d_int_xx(int a) { return ((a * 2 + 1) * (a) * (a + 1) / 6); }
// discrite integral for x.
inline int d_int_x(int a) { return ((a + 1) * a) / 2; }

/*
void line_fit_blob(const RangeImage& rimg, ImagePtr outbuf, double* cx, double* cy) {
  int64_t pxx = 0;
  int64_t pxy = 0;
  int64_t pyy = 0;
  int64_t px = 0;
  int64_t py = 0;
  int64_t nv = 0;
  for (int i = 0; i < rimg.ranges.size(); ++i) {
    int yv = rimg.mini + i;
    for (ImageRange rng : rimg.ranges[i]) {
      int ndiff = rng.ed - rng.st;
      int xdiff = d_int_x(rng.ed - 1) - d_int_x(rng.st - 1);
      int xxdiff = d_int_xx(rng.ed - 1) - d_int_xx(rng.st - 1);
      px += xdiff;
      py += ndiff * yv;
      nv += ndiff;
      pxy += xdiff * yv;
      pyy += yv * yv * ndiff;
      pxx += xxdiff;
    }
  }
  int mx = px / nv;
  int my = py / nv;
//  printf("%d, %d\n", mx, my);
  
  draw_bresham_line(Pt {mx - 10, my}, Pt {mx + 10, my}, outbuf, 125, 255, 0);
  draw_bresham_line(Pt {mx, my - 10}, Pt {mx, my + 10}, outbuf, 125, 255, 0);

    double nvsq = nv * nv;
    double m_xx = (pxx * nv - px * px) / nvsq;
    double m_xy = (pxy * nv - px * py) / nvsq;
    double m_yy = (pyy * nv - py * py) / nvsq;
    double m_x = px / ((double)nv);
    double m_y = py / ((double)nv);

    // printf("----- %g %g %g\n", m_xx, m_yy, m_xy);
    double b = (m_xx + m_yy) / 2.0;
    double c = m_xx * m_yy - m_xy * m_xy;

    double eiggen = sqrt(b * b - c);
    double eigv = b - eiggen;

    double vx = m_xx - eigv;
    double vy = m_xy;
    double mag = sqrt(vx * vx + vy * vy);
    vx /= mag;
    vy /= mag;

  double mind = 0.0;
  double maxd = 0.0;

  for (int i = 0; i < rimg.ranges.size(); ++i) {
    double ydvv = (rimg.mini + i - m_y) * vy;
    for (ImageRange rng : rimg.ranges[i]) {
      double xdvv = (rng.st - m_x) * vx;
      mind = std::min(ydvv + xdvv, mind);
      maxd = std::max(ydvv + xdvv, maxd);
      xdvv = (rng.ed - 1 - m_x) * vx;
      mind = std::min(ydvv + xdvv, mind);
      maxd = std::max(ydvv + xdvv, maxd);
    }
  }
  double ptdir;
  if (maxd > -mind) { mind = 0; ptdir = maxd; } else {maxd = 0; ptdir = mind; }
  ptdir *= 1.1;
  mind *= 1.1;
  maxd *= 1.1;
  *cx = m_x + vx * ptdir;
  *cy = m_y + vy * ptdir;


  Pt apt = {(int)(m_x + vx * mind), (int)(m_y + vy * mind)};
  Pt bpt = {(int)(m_x + vx * maxd), (int)(m_y + vy * maxd)};

  draw_bresham_line(apt, bpt, outbuf, 125, 255, 0); 
}
*/

}  // namespace vision
}  // namespace aos
