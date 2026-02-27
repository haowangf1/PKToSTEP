#ifndef __CXCHG_3DMAT_HPP__
#define __CXCHG_3DMAT_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include <cmath>
#include <iosfwd>

class Xchg_pnt;
class Xchg_dir;

//! \brief 3x3 Matrix class for 3D transformations
//! This class represents a 3x3 matrix used for rotations, scaling, and other linear transformations in 3D space
class XCHG_API Xchg_3dMat
{
protected:
    double _mat[3][3];  // 3x3 matrix storage in row-major order

public:
    // Constructors
    Xchg_3dMat();
    Xchg_3dMat(const Xchg_3dMat& other);
    Xchg_3dMat(double m00, double m01, double m02,
               double m10, double m11, double m12,
               double m20, double m21, double m22);
    ~Xchg_3dMat() {}

    // Assignment
    Xchg_3dMat& operator=(const Xchg_3dMat& other);

    // Element access
    double& operator()(Size_t row, Size_t col);
    const double& operator()(Size_t row, Size_t col) const;
    double* operator[](Size_t row);
    const double* operator[](Size_t row) const;

    // Matrix operations
    void SetIdentity();
    void SetZero();
    void SetDiagonal(double d0, double d1, double d2);
    
    // Matrix properties
    double Determinant() const;
    double Trace() const;
    bool IsIdentity(double tolerance = 1e-10) const;
    bool IsOrthogonal(double tolerance = 1e-10) const;
    
    // Matrix transformations
    void Transpose();
    Xchg_3dMat Transposed() const;
    bool Inverse();
    Xchg_3dMat Inversed() const;
    void Normalize();  // Normalize rotation matrix (Gram-Schmidt)
    
    // Matrix multiplication
    Xchg_3dMat operator*(const Xchg_3dMat& other) const;
    Xchg_3dMat& operator*=(const Xchg_3dMat& other);
    
    // Scalar operations
    Xchg_3dMat operator*(double scalar) const;
    Xchg_3dMat& operator*=(double scalar);
    Xchg_3dMat operator/(double scalar) const;
    Xchg_3dMat& operator/=(double scalar);
    
    // Matrix addition/subtraction
    Xchg_3dMat operator+(const Xchg_3dMat& other) const;
    Xchg_3dMat& operator+=(const Xchg_3dMat& other);
    Xchg_3dMat operator-(const Xchg_3dMat& other) const;
    Xchg_3dMat& operator-=(const Xchg_3dMat& other);
    
    // Comparison
    bool operator==(const Xchg_3dMat& other) const;
    bool operator!=(const Xchg_3dMat& other) const;
    bool IsEqual(const Xchg_3dMat& other, double tolerance = 1e-10) const;
    
    // Rotation matrix construction
    static Xchg_3dMat RotationX(double angle);
    static Xchg_3dMat RotationY(double angle);
    static Xchg_3dMat RotationZ(double angle);
    static Xchg_3dMat RotationAxis(const Xchg_dir& axis, double angle);
    static Xchg_3dMat RotationFromTo(const Xchg_dir& from, const Xchg_dir& to);
    
    // Scale matrix
    static Xchg_3dMat Scale(double sx, double sy, double sz);
    static Xchg_3dMat Scale(double s);
    
    // Skew-symmetric matrix from vector (for cross product)
    static Xchg_3dMat CrossProductMatrix(const Xchg_dir& v);
    
    // Extract rotation axis and angle
    bool GetAxisAngle(Xchg_dir& axis, double& angle, double tolerance = 1e-10) const;
    
    // Convert to/from basis vectors
    void SetFromBasis(const Xchg_dir& xAxis, const Xchg_dir& yAxis, const Xchg_dir& zAxis);
    void GetBasis(Xchg_dir& xAxis, Xchg_dir& yAxis, Xchg_dir& zAxis) const;
    
    // Apply transformation to vectors/points
    Xchg_dir Transform(const Xchg_dir& v) const;
    Xchg_dir TransformTranspose(const Xchg_dir& v) const;  // M^T * v
    
    // Output
    friend std::ostream& operator<<(std::ostream& os, const Xchg_3dMat& mat);
    
    // Utilities
    void Dump(FILE* file = stdout) const;
};

// Non-member operators
Xchg_3dMat operator*(double scalar, const Xchg_3dMat& mat);

#endif // __CXCHG_3DMAT_HPP__

