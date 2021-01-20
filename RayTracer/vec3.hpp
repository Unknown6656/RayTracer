#pragma once

#include "argb.hpp"

#define EULER_ARG const Vec3& euler_angles
#define EULER_OPTARG EULER_ARG = Vec3(0, 0, 0)
#define RAD2DEG(x) ((x) * 57.2957795131)
#define DEG2RAD(x) ((x) * 0.01745329251)
#define ROT_45 DEG2RAD(45)
#define ROT_90 DEG2RAD(90)
#define ROT_135 DEG2RAD(135)
#define ROT_180 DEG2RAD(180)
#define ROT_270 DEG2RAD(270)


struct Vec3
{
    static const Vec3 Zero, UnitX, UnitY, UnitZ;

    const double X, Y, Z;


    Vec3() noexcept : Vec3(0) {}

    Vec3(double v) noexcept : Vec3(v, v, v) {}

    Vec3(double x, double y, double z) noexcept : X(x), Y(y), Z(z) {}

    Vec3(double x, double y, double z, double w) : Vec3(x / w, y / w, z / w) {}

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

    inline double distance_to(const Vec3& other) const noexcept
    {
        return sub(other).length();
    }

    inline double angle_to(const Vec3& other) const noexcept
    {
        return std::acos(normalize().dot(other.normalize()));
    }

    inline Vec3 rotate(const double& euler_x, const double& euler_y, const double& euler_z) const noexcept
    {
        return transform(create_rotation_matrix(euler_x, euler_y, euler_z));
    }

    inline Vec3 rotate(const Vec3& euler_angles) noexcept
    {
        return rotate(euler_angles.X, euler_angles.Y, euler_angles.Z);
    }

    inline std::string to_string() const noexcept
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
            cy* cz, sx* sy* cz - cx * sz, cx* sy* cz + sx * sz,
                cx* sz, sx* sy* sz + cx * cz, cx* sy* sz - sy * cz,
                sy, sx* cy, cx* cy
        };
    }

    OSTREAM_OPERATOR(Vec3);
    CPP_IS_FUCKING_RETARDED(Vec3);

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
