#pragma once

#include <cmath>

#define λ(x, body) x { return (body); }


struct Col
{
    double A, R, G, B;
};

struct Vec3
{
    const double X, Y, Z;

    Vec3() noexcept : Vec3(0) {}
    Vec3(double v) noexcept : Vec3(v, v, v) {}
    Vec3(double x, double y, double z) noexcept : X(x), Y(y), Z(z) {}
    Vec3(double x, double y, double z, double w) : Vec3(x / w, y / w, z / w) {}
    // Vec3(std::initializer_list<double> list) : Vec3(process(list)) {}
    // Vec3(const double* const arr) : Vec3(arr[0], arr[1], arr[2]) {}

    double norm() const noexcept
    {
        return X * X + Y * Y + Z * Z;
    }

    double length() const noexcept
    {
        return std::sqrt(norm());
    }

    Vec3 normalize() const
    {
        return *this / length();
    }

    double dot(const Vec3 other) const noexcept
    {
        return X * other.X + Y * other.Y + Z * other.Z;
    }

    Vec3 cross(const Vec3 other) const noexcept
    {
        return Vec3(
            Y * other.Z - Z * other.Y,
            Y * other.X - X * other.Y,
            X * other.Z - Z * other.X
        );
    }


    inline Vec3 operator+() const noexcept
    {
        return Vec3(X, Y, Z);
    }

    inline Vec3 operator-() const noexcept
    {
        return Vec3(-X, -Y, -Z);
    }

    inline Vec3 operator+(const Vec3 other) const noexcept
    {
        return Vec3(X + other.X, Y + other.Y, Z + other.Z);
    }

    inline Vec3 operator-(const Vec3 other) const noexcept
    {
        return Vec3(X - other.X, Y - other.Y, Z - other.Z);
    }

    inline double operator*(const Vec3 other) const noexcept
    {
        return dot(other);
    }

    inline Vec3 operator*(const double factor) const noexcept
    {
        return Vec3(X * factor, Y * factor, Z * factor);
    }

    inline Vec3 operator/(const double factor) const
    {
        return Vec3(X / factor, Y / factor, Z / factor);
    }

    inline Vec3 operator+=(const Vec3 other) const noexcept
    {
        return *this + other;
    }

    inline Vec3 operator-=(const Vec3 other) const noexcept
    {
        return *this - other;
    }

    inline double operator*=(const Vec3 other) const noexcept
    {
        return *this * other;
    }

    inline Vec3 operator*=(const double other) const noexcept
    {
        return *this * other;
    }

    inline Vec3 operator/=(const double factor) const
    {
        return *this / factor;
    }
};

struct Ray
{
    Vec3 Start, Direction;
};


