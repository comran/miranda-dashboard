#ifndef AOS_VISION_COMP_GEO_VECTOR_H_
#define AOS_VISION_COMP_GEO_VECTOR_H_

#include <cmath>
#include "Eigen/Dense"

namespace aos {
namespace vision {
namespace comp_geo {

// two dimensional vector.
template<int Size>
class Vector {
 public:
  // setup memory.
  Vector(): data_(Size) {
	  for (int i = 0; i < Size; i++) {
      Set(i, 0.0);
    }
  }
  
  Vector(double x, double y) :
  	Vector() {
		Set(x, y);
	}
  
  Vector(double x, double y, double z) :
  	Vector() {
		Set(x, y, z);
	}
  
  ~Vector() { }

  double Get(int index) const {
  	return data_(index);
  }

  void Set(int index, double value) {
  	  data_(index) = value;
  }

  void Set(double x, double y) {
    data_(0) = x;
    data_(1) = y;
  }

  void Set(double x, double y, double z) {
    Set(x, y);
    data_(2) = z;
  }
  void Set(double x, double y, double z, double w) {
    Set(x, y, z);
    data_(3) = w;
  }

  // I like x.
  double x() const { 
  	if (Size < 1) return 0.0;
	  return data_(0);
  }

  void x(double xX) {
    if (Size >= 1) data_(0) = xX;
  }

  // I like y.
  double y() const {
  	if (Size < 2) return 0.0;
    return data_(1);
  }

  void y(double yY) {
    if (Size >= 2) data_(1) = yY;
  }

  // I like z.
  double z() const {
  	if (Size < 3) return 0.0;
    return data_(2);
  }

  void z(double zZ) {
    if (Size >= 3) data_(2) = zZ;
  }

  // I like w.
  double w() const {
  	if (Size < 4) return 0.0;
    return data_(3);
  }

  void w(double wW) {
    if (Size >= 4) data_(3) = wW;
  }

  // Fast part of length.
  double MagSqr() const {
  	  double sum = 0;
  	  for (int i = 0; i < Size; i++) {
	  	  sum += data_(i) * data_(i);
	  }
  	  return sum;
  }
  
  // Length of the vector.
  double Mag() const { return std::sqrt(MagSqr()); }

  // Get underlying data structure
  Eigen::RowVectorXd GetData() const { return data_; }

  // Set underlying data structure
  void SetData(Eigen::RowVectorXd& other) { data_ = other; }

  // add
  Vector<Size> operator +(const Vector<Size>& other) const {
  	  Vector<Size> nv;
	  for (int i = 0; i < Size; i++) {
	  	  nv.Set(i, other.Get(i) + Get(i));
	  }
	  return nv;
  }

  // add
  Vector<Size> operator +=(const Vector<Size>& other) {
	  for (int i = 0; i < Size; i++) {
	  	  Set(i, other.Get(i) + Get(i));
	  }
	  return *this;
  }
  
  // subtract
  Vector<Size> operator -(const Vector<Size>& other) const {
  	  Vector<Size> nv;
	  for (int i = 0; i < Size; i++) {
	  	  nv.Set(i, Get(i) - other.Get(i));
	  }
	  return nv;
  }
  
  // subtract
  Vector<Size> operator -=(const Vector<Size>& other) {
	  for (int i = 0; i < Size; i++) {
	  	  Set(i, Get(i) - other.Get(i));
	  }
	  return *this;
  }
  
  // scalar multiply
  Vector<Size> Scale (
      const double& other) const {
    Vector<Size> nv;
    for (int i = 0; i < Size; i++) {
      nv.Set(i, other * Get(i));
    }
    return nv;
  }
  
  // scalar multiply
  Vector<Size> operator *=(const double& other) {
	  for (int i = 0; i < Size; i++) {
	  	  this->Set(i, other * this->Get(i));
	  }
	  return *this;
  }
  
  // pairwise multiply
  Vector<Size> operator *(const Vector<Size>& other) const {
  	  Vector<Size> nv;
	  for (int i = 0; i < Size; i++) {
	  	  nv.Set(i, other.Get(i) * Get(i));
	  }
	  return nv;
  }
  
  // pairwise multiply
  Vector<Size> operator *=(const Vector<Size>& other) {
	  for (int i = 0; i < Size; i++) {
	  	  Set(i, other.Get(i) * Get(i));
	  }
	  return *this;
  }
  
  // Equality.
  bool operator ==(const Vector<Size>& other) const {
	  for (int i = 0; i < Size; i++) {
	  	  if (Get(i) != other.Get(i)) {
          return false;
        }
	  }
	  return true;
  }

  double dot(const Vector<Size>& other) const {
	  return data_.dot(other.GetData());
  }

  Vector<Size> cross(const Vector<Size>& other) const{
  	Vector<Size> nv;
	  nv.SetData(data_.cross(other.GetData()));
	  return nv;
  }

  Vector<Size> normed() const{
    double mag = Mag();
  	Vector<Size> nv;
	  for (int i = 0; i < Size; i++) {
	  	  nv.Set(i, Get(i) / mag);
	  }
	  return nv;
  }

  double AngleToZero() const {
    return std::atan2(y(),  x());
  }


  // Return angle between two vectors.
  double AngleTo(const Vector<Size> other) const {
    //cosθ = (u⃗ · v⃗) / (||u⃗|| ||v ⃗||)
    return std::acos(dot(other) / (Mag() * other.Mag()));
  }

  double SqDistTo(const Vector<Size> other) const {
    Vector<Size> tmp = *this - other;
    return tmp.MagSqr();
  }

  // Projection of this vector onto the other.
  Vector<Size> ProjectTo(Vector<Size> other) {
    return other.Scale((dot(other)/other.dot(other)));
  }

  // The opposite leg of the triangle.
  Vector<Size> RejectFrom(Vector<Size> other) const {
    return *this - ProjectTo(other);
  }

  // Show vector
  void Print() {
	  for (int i = 0; i < Size; i++) {
  	  	printf("%f, ", data_(i));
	  }
	  printf("\n");
  }

 private:
  // Data stored in eigen vector.
  Eigen::RowVectorXd data_;
};

inline double cross_pts(const Vector<2>& apt, const Vector<2>& bpt) {
  return apt.x() * bpt.y() - apt.y() * bpt.x();
}

}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

// scalar multiply
template<int Size>
inline aos::vision::comp_geo::Vector<Size> operator*(
    const double& lhs, aos::vision::comp_geo::Vector<Size>& rhs) {
  aos::vision::comp_geo::Vector<Size> nv;
  for (int i = 0; i < Size; i++) {
    nv.Set(i, lhs * rhs.Get(i));
  }
  return nv;
}

template <int Size>
std::ostream& operator<<(std::ostream& os,
                         const aos::vision::comp_geo::Vector<Size>& obj) {
  os << '[';
  const Eigen::Vector3d o = obj.GetData();
  for (int i = 0; i < Size; i++) {
    os << o(i);
    if (i != Size - 1) {
      os << ',' << ' ';
    }
  }
  os << ']';
  return os;
}

#endif  // AOS_VISION_COMP_GEO_VECTOR_H_
