#include "aos/vision/blob/find_blob.h"

#include "aos/vision/blob/disjoint_set.h"

namespace aos {
namespace vision {

class BlobBuilder {
 public:
  BlobBuilder(int i) : mini(i) { img.mini = mini; }
  RangeImage img;
  int mini;
  void add(int i, ImageRange rng) {
    while ((int)img.ranges.size() <= i - mini) {
      img.ranges.emplace_back();
    }
    img.ranges[i - mini].push_back(rng);
  }

  void merge(const BlobBuilder &o) {
    const RangeImage &a = o.img;
    int diff = o.mini - mini;
    // while (img.ranges.size() < (diff + a.ranges.size())) {
    // img.ranges.emplace_back(); }
    for (int j = 0; j < (int)a.ranges.size(); j++) {
      int i = j + diff;
      while ((int)img.ranges.size() <= i) {
        img.ranges.emplace_back();
      }

      std::vector<ImageRange> cur = img.ranges[i];
      std::vector<ImageRange> prev = a.ranges[j];

      img.ranges[i].clear();  // = std::vector<ImageRange>();
      int a = 0;
      int b = 0;
      while (a < (int)cur.size() && b < (int)prev.size()) {
        if (cur[a].st < prev[b].st) {
          img.ranges[i].push_back(cur[a++]);
        } else {
          img.ranges[i].push_back(prev[b++]);
        }
      }
      while (a < (int)cur.size()) {
        img.ranges[i].push_back(cur[a++]);
      }
      while (b < (int)prev.size()) {
        img.ranges[i].push_back(prev[b++]);
      }
    }
  }
};

class BlobDisjointSet {
 public:
  DisjointSet dj_set;
  std::vector<BlobBuilder> items;
  int add_to_blob(int bid, int i, ImageRange img) {
    bid = dj_set.find(bid);
    items[bid].add(i, img);
    return bid;
  }
  int add_blob(int i, ImageRange img) {
    int bid = dj_set.add();
    items.emplace_back(i);
    items[bid].add(i, img);
    return bid;
  }
  void merge_in_blob(int cbid, int pbid) {
    cbid = dj_set.find(cbid);
    pbid = dj_set.find(pbid);
    if (cbid != pbid) {
      if (items[cbid].mini > items[pbid].mini) {
        std::swap(cbid, pbid);
      }
      items[cbid].merge(items[pbid]);
      dj_set.ufindl[pbid] = cbid;
    }
  }
  BlobList get_blobs() {
    std::vector<RangeImage> blobs;
    for (int i = 0; i < (int)items.size(); i++) {
      if (dj_set.ufindl[i] < 0) {
        blobs.emplace_back(items[i].img);
      }
    }
    return blobs;
  }
};


BlobList find_blobs(const RangeImage &rimg) {  //, AnalysisAllocator* alloc) {
  BlobDisjointSet blob_set;
  std::vector<int> pids;
  std::vector<int> cids;
  for (ImageRange rng : rimg.ranges[0]) {
    pids.push_back(blob_set.add_blob(0, rng));
  }

  for (int i = 1; i < rimg.size(); i++) {
    int mi = 0;
    int mj = 0;
    const std::vector<ImageRange> &pranges = rimg.ranges[i - 1];
    const std::vector<ImageRange> &cranges = rimg.ranges[i];
    cids.clear();

    while (mi < (int)pranges.size() && mj < (int)cranges.size()) {
      ImageRange rprev = pranges[mi];
      ImageRange rcur = cranges[mj];
      if (rcur.last() < rprev.st) {
        if ((int)cids.size() == mj) {
          cids.push_back(blob_set.add_blob(i, cranges[mj]));
        }
        mj++;
      } else if (rprev.last() < rcur.st) {
        mi++;
      } else {
        if ((int)cids.size() > mj) {
          blob_set.merge_in_blob(cids[mj], pids[mi]);
        } else {
          cids.push_back(blob_set.add_to_blob(pids[mi], i, cranges[mj]));
        }
        if (rcur.last() < rprev.last()) {
          mj++;
        } else {
          mi++;
        }
      }
    }
    while (mj < (int)cranges.size()) {
      if ((int)cids.size() == mj) {
        cids.push_back(blob_set.add_blob(i, cranges[mj]));
      }
      mj++;
    }
    std::swap(pids, cids);
  }
  return blob_set.get_blobs();
}

}  // namespace vision
}  // namespace aos
