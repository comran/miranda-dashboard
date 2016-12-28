#ifndef _AOS_VISION_BLOB_DISJOINT_SET_H_
#define _AOS_VISION_BLOB_DISJOINT_SET_H_

#include <vector>

namespace aos {
namespace vision {

class DisjointSet {
 public:
  std::vector<int> ufindl;
  DisjointSet() {}
  DisjointSet(int l) : ufindl(l, -1) {}
  int add() {
    ufindl.push_back(-1);
    return ufindl.size() - 1;
  }
  int &operator[](int i) { return ufindl[i]; }
  int find(int bid) {
    int tid = ufindl[bid];
    if (tid <= -1) {
      return bid;
    }
    return ufindl[bid] = find(tid);
  }
};

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_BLOB_DISJOINT_SET_H_
