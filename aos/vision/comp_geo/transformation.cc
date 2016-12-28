#include "math/transformation.h"
#include "math/matrix4x4.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Transformation Transformation::tra(Vector v) {
  Transformation result;
  for(int i = 0; i<3;i++) {
    result.m[i][3]=v[i];
    result.invm[i][3]=-v[i]; 
  }
  return result;
}

Matrix4x4 outerprod(const Vector& v) {
  Matrix4x4 m;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; ++j) {
      m[i][j] = v[i] * v[j];
    }
  }
  return m;
}

Transformation Transformation::rot(Vector v) {
  Transformation result;
  v[3] = 0;
  float angle  = v.mag();
  Vector rot = v.norm();
  Matrix4x4 lie = lie_cross(rot);
  Matrix4x4 ide;
  result.m = (outerprod(rot)).add(lie.mult(sin(angle)).add(lie.mult(lie).mult(-cos(angle))));
  result.m[3][3] = 1;
  result.invm = (outerprod(rot)).add(lie.mult(sin(-angle)).add(lie.mult(lie).mult(-cos(-angle))));
  result.invm[3][3] = 1;
  return result;
}

Matrix4x4 Transformation::lie_cross(Vector v) {
  Matrix4x4 lieCross;
  lieCross[0][0] = lieCross[1][1] = lieCross[2][2] = 0;
  lieCross[0][1]=-v[2];
  lieCross[0][2]=v[1];
  lieCross[1][0]=v[2];
  lieCross[1][2]=-v[0];
  lieCross[2][0]=-v[1];
  lieCross[2][1]=v[0];
  lieCross[3][3]=0;
  return lieCross;
}

Transformation Transformation::sca(Vector v) {
  
  Transformation result;
  for (int i=0;i<3;i++) {
    result.m[i][i]=v[i];
    result.invm[i][i]=1.0/v[i];
  }
  return result;
}

Transformation Transformation::comp(Transformation t) {
  Transformation result;
  result.m = m.mult(t.m);
  result.invm = t.invm.mult(invm);

  result.tran_invm = result.invm.transpose();

  result.tran_invm[3][0] = 0;
  result.tran_invm[3][1] = 0;
  result.tran_invm[3][2] = 0;
  
  result.tran_invm[0][3]=0;
  result.tran_invm[1][3]=0;
  result.tran_invm[2][3]=0;


  return result;
}
