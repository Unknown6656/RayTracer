#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <ostream>

#define EPSILON 1e-5
#define OSTREAM_OPERATOR(x) friend inline std::ostream& operator<<(std::ostream& os, const x& value) { os << value.to_string(); return os; }


struct ARGB
{
    double A, R, G, B;


    ARGB() noexcept : ARGB(0) { }

    ARGB(double gray) noexcept : ARGB(1, gray) { }

    ARGB(double a, double gray) noexcept : ARGB(gray, gray, gray) { }

    ARGB(double r, double g, double b) noexcept : ARGB(1, r, g, b) { }

    ARGB(double a, double r, double g, double b) noexcept
        : A(a), R(r), G(g), B(b)
    {
    }

    std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << '[' << A << "," << R << "," << G << "," << B << ']';

        return ss.str();
    }

    OSTREAM_OPERATOR(ARGB);

    inline ARGB operator-() const noexcept
    {
        return ARGB(-A, -R, -G, -B);
    }

    inline ARGB operator+(const ARGB& other) const noexcept
    {
        return ARGB(A + other.A, R + other.R, G + other.G, B + other.B);
    }

    inline ARGB operator-(const ARGB& other) const noexcept
    {
        return ARGB(A - other.A, R - other.R, G - other.G, B - other.B);
    }

    inline ARGB operator*(const double factor) const noexcept
    {
        return ARGB(A * factor, R * factor, G * factor, B * factor);
    }

    inline ARGB operator/(const double factor) const
    {
        return ARGB(A / factor, R / factor, G / factor, B / factor);
    }
};

struct Vec3
{
    static const Vec3 Zero, UnitX, UnitY, UnitZ;

    const double X, Y, Z;


    Vec3() noexcept : Vec3(0) { }

    Vec3(double v) noexcept : Vec3(v, v, v) { }

    Vec3(double x, double y, double z) noexcept : X(x), Y(y), Z(z) { }

    Vec3(double x, double y, double z, double w) : Vec3(x / w, y / w, z / w) { }

    // Vec3(std::initializer_list<double> list) : Vec3(process(list)) { }

    // Vec3(const double* const arr) : Vec3(arr[0], arr[1], arr[2]) { }

    inline double norm() const noexcept
    {
        return X * X + Y * Y + Z * Z;
    }

    inline double length() const noexcept
    {
        return std::sqrt(norm());
    }

    inline Vec3 normalize() const
    {
        return *this / length();
    }

    inline Vec3 add(const Vec3& other) const noexcept
    {
        return Vec3(X + other.X, Y + other.Y, Z + other.Z);
    }

    inline Vec3 sub(const Vec3& other) const noexcept
    {
        return Vec3(X - other.X, Y - other.Y, Z - other.Z);
    }

    inline Vec3 scale(const double factor) const noexcept
    {
        return Vec3(X * factor, Y * factor, Z * factor);
    }

    inline double dot(const Vec3& other) const noexcept
    {
        return X * other.X + Y * other.Y + Z * other.Z;
    }

    inline Vec3 cross(const Vec3& other) const noexcept
    {
        return Vec3(
            Y * other.Z - Z * other.Y,
            Z * other.X - X * other.Z,
            X * other.Y - Y * other.X
        );
    }

    inline Vec3 reflect(const Vec3& normal) const noexcept
    {
        const double theta = dot(normal);

        return normal.scale(2 * theta).sub(*this);
    };

    inline Vec3 refract(const Vec3& normal, const double eta, bool* const total_reflection)
    {
        const double theta = dot(normal);
        const double k = 1 - (eta * eta * (1 - theta * theta));

        if (total_reflection)
            *total_reflection = k < 0;

        if (k < 0)
            return reflect(-normal);
        else
            return scale(eta).add(normal.scale(eta * theta - std::sqrt(k)));
    }

    inline Vec3 transform(const std::vector<double>& matrix) const noexcept
    {
        return Vec3(
            matrix[0] * X + matrix[1] * Y + matrix[2] * Z,
            matrix[3] * X + matrix[4] * Y + matrix[5] * Z,
            matrix[6] * X + matrix[7] * Y + matrix[8] * Z
        );
    }

    inline Vec3 rotate(const double& euler_x, const double& euler_y, const double& euler_z) const noexcept
    {
        return transform(create_rotation_matrix(euler_x, euler_y, euler_z));
    }

    inline Vec3 rotate(const Vec3& euler_angles) noexcept
    {
        return rotate(euler_angles.X, euler_angles.Y, euler_angles.Z);
    }

    std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << '[' << X << "," << Y << "," << Z << ']';

        return ss.str();
    }

    static std::vector<double> create_rotation_matrix(const Vec3& euler_angles) noexcept
    {
        return create_rotation_matrix(euler_angles.X, euler_angles.Y, euler_angles.Z);
    }

    static std::vector<double> create_rotation_matrix(const double& euler_x, const double& euler_y, const double& euler_z) noexcept
    {
        const double sx = std::sin(euler_x);
        const double cx = std::cos(euler_x);
        const double sy = std::sin(euler_y);
        const double cy = std::cos(euler_y);
        const double sz = std::sin(euler_z);
        const double cz = std::cos(euler_z);

        return std::vector<double>
        {
            cy * cz, sx * sy * cz - cx * sz, cx * sy * cz + sx * sz,
            cx * sz, sx * sy * sz + cx * cz, cx * sy * sz - sy * cz,
            sy,      sx * cy,                cx * cy
        };
    }

    OSTREAM_OPERATOR(Vec3);

    // why? because c++ is fucking retarded, that's why!
    inline Vec3& operator =(const Vec3& value)
    {
        return this == &value ? *this : *new(this)Vec3(value);
    }

    inline Vec3 operator+() const noexcept
    {
        return Vec3(X, Y, Z);
    }

    inline Vec3 operator-() const noexcept
    {
        return Vec3(-X, -Y, -Z);
    }

    inline Vec3 operator+(const Vec3& other) const noexcept
    {
        return add(other);
    }

    inline Vec3 operator-(const Vec3& other) const noexcept
    {
        return sub(other);
    }

    inline double operator*(const Vec3& other) const noexcept
    {
        return dot(other);
    }

    inline Vec3 operator*(const double factor) const noexcept
    {
        return scale(factor);
    }

    inline Vec3 operator/(const double factor) const
    {
        return scale(1 / factor);
    }

    //inline Vec3 operator+=(const Vec3 other) const noexcept
    //{
    //    return *this + other;
    //}

    //inline Vec3 operator-=(const Vec3 other) const noexcept
    //{
    //    return *this - other;
    //}

    //inline Vec3 operator*=(const double other) const noexcept
    //{
    //    return *this * other;
    //}

    //inline Vec3 operator/=(const double factor) const
    //{
    //    return *this / factor;
    //}

    inline operator ARGB() const noexcept
    {
        return ARGB(X, Y, Z);
    }
};

const Vec3 Vec3::Zero(0),
           Vec3::UnitX(1, 0, 0),
           Vec3::UnitY(0, 1, 0),
           Vec3::UnitZ(0, 0, 1);

#define OPTIONAL_EULER const Vec3& euler_angles = Vec3(0, 0, 0)

struct Ray
{
    const Vec3 Origin;
    const Vec3 Direction;
    const size_t IterationDepth;


    Ray() noexcept : Ray(Vec3(), Vec3(), 0) { }

    Ray(const Vec3& origin, const Vec3& dir) noexcept : Ray(origin, dir, 0) { }

    Ray(const Vec3& origin, const Vec3& dir, const size_t depth) noexcept
        : Origin(origin)
        , Direction(dir.normalize())
        , IterationDepth(depth)
    {
    }

    inline Vec3 evaluate(const double at) const noexcept
    {
        return Origin + Direction * at;
    }

    inline Ray create_next(const double at, const Vec3& next_dir) const noexcept
    {
        return Ray(evaluate(at), next_dir, IterationDepth + 1);
    }

    bool MöllerTrumboreIntersect(
        const Vec3& A, const Vec3& B, const Vec3& C,
        double* const __restrict t, double* const __restrict u, double* const __restrict v,
        bool* const __restrict hit_backface
    ) const noexcept
    {
        if (!t || !u || !v)
            return false;

        const Vec3 edgeAB = B - A;
        const Vec3 edgeAC = C - A;
        const Vec3 pvec = Direction.cross(edgeAC);
        const double det = edgeAB.dot(pvec);

        if (fabs(det) < EPSILON)
            return false;
        else if (hit_backface)
            *hit_backface = det < 0;

        const double inv_det = 1 / det;
        const Vec3 tvec = Origin - A;

        *u = tvec.dot(pvec) * inv_det;

        if (*u < 0 || *u > 1)
            return false;

        const Vec3 qvec = tvec.cross(edgeAB);

        *v = Direction.dot(qvec) * inv_det;

        if (*v < 0 || *u + *v > 1)
            return false;

        *t = edgeAC.dot(qvec) * inv_det;

        return true;
    }

    std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << "[O=" << Origin << ", D=" << Direction << ", I=" << IterationDepth << ']';

        return ss.str();
    }

    // why? because c++ is fucking retarded, that's why!
    inline Ray& operator=(const Ray& value)
    {
        return this == &value ? *this : *new(this)Ray(value);
    }

    OSTREAM_OPERATOR(Ray);
};

struct RayTraceIteration
{
    Ray Ray;
    double Distance;
    ARGB ComputedColor;
    Vec3 SurfaceNormal;


    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[R=" << Ray << ", D=" << Distance << ", N=" << SurfaceNormal << ", C=" << ComputedColor << "]";

        return ss.str();
    }

    OSTREAM_OPERATOR(RayTraceIteration);
};

typedef std::vector<RayTraceIteration> RayTraceResult;
