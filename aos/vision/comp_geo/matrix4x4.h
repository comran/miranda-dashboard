#ifndef AOS_VISION_COMP_GEO_MATRIX4X4_H_
#define AOS_VISION_COMP_GEO_MATRIX4X4_H_

#include "aos/vision/comp_geo/vector.h"

namespace aos {
namespace vision {
namespace comp_geo {

typedef double MatrixRow[4];

// Matirx of the form 4x4,  mat[row][column]
class Matrix4x4{
 public:
  double data[4][4];
  Matrix4x4() {
    const double data2[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        data[i][j] = data2[i][j];
      }
    }
  }

  const MatrixRow& operator[](int i) const{return data[i];}

  MatrixRow& operator[](int i) {return data[i];}
  Vector<4> mult(const Vector<4>& v) const;
    Matrix4x4 add(const Matrix4x4& m) const {
    Matrix4x4 result;
    for(int i=0;i<4;i++) {
      for(int j=0; j<4;j++) {
        result[i][j] = m[i][j]+data[i][j];
      }
    }
    return result;
  }
  
  Matrix4x4 mult(const Matrix4x4& m) const {
    Matrix4x4 result;
    for(int i=0;i<4;i++) {
      for(int j=0;j<4;j++) {
        float tot = 0;
        for(int k=0;k<4;k++) {
          tot += data[i][k] * m[k][j];
        }
        result.data[i][j] = tot;
      }
    }
    return result;
  }

  Matrix4x4 mult(const double& c) const {
  Matrix4x4 result;
    for(int i=0;i<4;i++) {
      for(int j=0; j<4;j++) {
        result[i][j] = data[i][j]*c;
      }
    }
    return result;
  }

  Matrix4x4 transpose() const {
    Matrix4x4 result;
    for (int i=0; i<4;i++){
      for(int j =0; j<4;j++) {
        result[j][i]=data[i][j];
      }
    }
    return result;
  }

  // print as string
  void ToString() const {
    for (int i = 0; i < 4; ++i) {
      printf("[");
      for (int j = 0; j < 4; ++j) {
        printf("%g, ", data[i][j]);
      } 
      printf("],\n");
    } 
  }
};

}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_COMP_GEO_MATRIX4X4_H_

