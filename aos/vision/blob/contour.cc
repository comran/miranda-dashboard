#include "aos/vision/blob/contour.h"

namespace aos {
namespace vision {

class HalfContour {
 public:
  ContourNode* st;
  ContourNode* ed;
};

HalfContour fwd_range(int i, int sti, int edi, AnalysisAllocator *alloc) {
  ContourNode *st = alloc->cons_obj<ContourNode>(sti, i);
  ContourNode *ed = st;
  st->next = st;
  for (int x = sti + 1; x < edi; x++) {
    //   printf("x: %d\n", x);
    ed = alloc->cons_obj<ContourNode>(x, i, ed);
    //    ed->next = n;
    //    ed = n;
  }
  return HalfContour{st, ed};
}
// inline int event(bool rt, const ImageRange& rng) {
//  return rt ? rng.last() : rng.st;
//}

void close_range(HalfContour hc, AnalysisAllocator *alloc) {
  auto p_end = hc.ed;
  auto p_cur = hc.st;
  while (p_cur->pt.x < p_end->pt.x - 1) {
    p_cur = p_cur->append(p_cur->pt.x + 1, p_cur->pt.y, alloc);
  }
  if (p_end->pt.x == p_cur->pt.x) {
    p_cur->next = p_end->next;
  } else {
    p_cur->next = p_end;
  }
}

ContourNode *extend_start(int /*i*/, ContourNode *pst, int cst,
                          AnalysisAllocator *alloc) {
  while (pst->pt.x < cst) {
    pst = pst->append(pst->pt.x + 1, pst->pt.y, alloc);
  }
  pst = pst->append(pst->pt.x, pst->pt.y + 1, alloc);
  while (pst->pt.x > cst) {
    pst = pst->append(pst->pt.x - 1, pst->pt.y, alloc);
  }
  return pst;
}

ContourNode *extend_end(int /*i*/, ContourNode *pst, int cst,
                        AnalysisAllocator *alloc) {
  while (pst->pt.x > cst) {
    pst = pst->pappend(pst->pt.x - 1, pst->pt.y, alloc);
  }
  pst = pst->pappend(pst->pt.x, pst->pt.y + 1, alloc);
  while (pst->pt.x < cst) {
    pst = pst->pappend(pst->pt.x + 1, pst->pt.y, alloc);
  }
  return pst;
}

void cap_range(int /*i*/, ContourNode *pst, ContourNode *est,
               AnalysisAllocator *alloc) {
  est = est->append(est->pt.x, est->pt.y + 1, alloc);
  while (est->pt.x > pst->pt.x) {
    est = est->append(est->pt.x - 1, est->pt.y, alloc);
  }
  est->next = pst;
}

HalfContour make_cavity(int i, int sti, int edi, AnalysisAllocator *alloc) {
  ContourNode *st = alloc->cons_obj<ContourNode>(sti, i);
  ContourNode *ed = st;
  for (int x = sti + 1; x < edi; x++) {
    ed = ed->append(x, i, alloc);
  }
  return HalfContour{st, ed};
}

ContourNode *range_img_to_obj(const RangeImage &rimg,
                              AnalysisAllocator *alloc) {
  alloc->reset();
  std::vector<HalfContour> clst;
  std::vector<HalfContour> plst;

  for (int j = 0; j < (int)rimg.ranges[0].size(); j++) {
    ImageRange rng = rimg.ranges[0][j];
    plst.emplace_back(fwd_range(rimg.mini, rng.st, rng.ed, alloc));
  }

  for (int j = 1; j < (int)rimg.size(); j++) {
    const std::vector<ImageRange> &pranges = rimg.ranges[j - 1];
    const std::vector<ImageRange> &cranges = rimg.ranges[j];
    int i = j + rimg.mini;
    clst.clear();
    int mp = 0;
    int mc = 0;

    while (mp < (int)pranges.size() && mc < (int)cranges.size()) {
      ImageRange rprev = pranges[mp];
      ImageRange rcur = cranges[mc];
      if (rcur.last() < rprev.st) {
        clst.emplace_back(fwd_range(i, rcur.st, rcur.ed, alloc));
        mc++;
      } else if (rprev.last() < rcur.st) {
        close_range(plst[mp], alloc);
        mp++;
      } else {
        ContourNode *within_pb = plst[mp].ed;
        ContourNode *within_ca = extend_start(i, plst[mp].st, rcur.st, alloc);

        while (true) {
          if (mp + 1 < (int)pranges.size() and
              rcur.last() >= pranges[mp + 1].st) {
            mp++;
            cap_range(i, within_pb, plst[mp].st, alloc);
            within_pb = plst[mp].ed;
            rprev = pranges[mp];
          } else if (mc + 1 < (int)cranges.size() and
                     rprev.last() >= cranges[mc + 1].st) {
            auto cav_t = make_cavity(i, rcur.last(), cranges[mc + 1].st, alloc);
            clst.emplace_back(HalfContour{within_ca, cav_t.st});
            within_ca = cav_t.ed;
            mc++;
            rcur = cranges[mc];
          } else {
            within_pb = extend_end(i, within_pb, rcur.last(), alloc);
            clst.emplace_back(HalfContour{within_ca, within_pb});
            mc++;
            mp++;
            break;
          }
        }
      }
    }
    while (mc < (int)cranges.size()) {
      ImageRange rcur = cranges[mc];
      clst.emplace_back(fwd_range(i, rcur.st, rcur.ed, alloc));
      mc++;
    }

    while (mp < (int)pranges.size()) {
      close_range(plst[mp], alloc);
      mp++;
    }
    std::swap(clst, plst);
  }

  for (int mp = 0; mp < (int)plst.size(); mp++) {
    close_range(plst[mp], alloc);
  }
  return plst[0].st;

  /*
  bool rt_p = false;
  bool rt_c = false;
  ContourNode* p_cur = NULL;
  ContourNode* c_cur = NULL;

  ContourNode* c_start = NULL;

  while (mp < pranges.size() || mc < cranges.size()) {
    if (mc < cranges.size() && event(rt_c, cranges[mc]) < event(rt_p,
  pranges[mp])) {
      if (!rt_c) {
        c_cur = c_start = alloc->cons_obj<ContourNode>(cranges[mc].st, i);

        if (rt_p) {
          while (p_cur->pt.x < cranges[mc].st) {
            p_cur = alloc->cons_obj<ContourNode>(p_cur->pt.x + 1, p_cur->pt.y,
  p_cur);
          }
          p_cur->next = c_cur;
        }
      } else {
        auto n = alloc->cons_obj<ContourNode>(cranges[mc].last(), i);
        if (rt_p) {

        } else {


        }
        clst.emplace_back(HalfContour {c_start, n} );
        mc++;
      } rt_c = !rt_c;
    } else if ((mp < cranges.size()) &&
                event(rt_c, cranges[mc]) == event(rt_p, pranges[mp]) &&
                rt_p == rt_c) {

    } else {
      if (!rt_p) { p_cur = plst.st; }


      if (rt_p) {
        mp++;
      } rt_p = !rt_p;
    }
    */
}

}  // namespace vision
}  // namespace aos
