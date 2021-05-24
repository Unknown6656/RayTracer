#pragma once

#include "../argb.hpp"

#define EULER_ARG const ray_tracer_3d::vec3& euler_angles
#define EULER_OPTARG EULER_ARG = ray_tracer_3d::vec3(0.f, 0.f, 0.f)


namespace ray_tracer_3d
{
    struct vec3
    {
        static const vec3 Zero, UnitX, UnitY, UnitZ;

        const float X, Y, Z;


        vec3() noexcept : vec3(0.f) {}

        vec3(float v) noexcept : vec3(v, v, v) {}

        vec3(float x, float y, float z) noexcept : X(x), Y(y), Z(z) {}

        vec3(float x, float y, float z, float w) : vec3(x / w, y / w, z / w) {}

        // vec3(std::initializer_list<float> list) : vec3(process(list)) { }

        // vec3(const float* const arr) : vec3(arr[0], arr[1], arr[2]) { }

        inline float squared_length() const noexcept
        {
            return X * X + Y * Y + Z * Z;
        }

        inline float length() const noexcept
        {
            return std::sqrt(squared_length());
        }

        inline vec3 normalize() const
        {
            return *this / length();
        }

        inline vec3 add(const vec3& other) const noexcept
        {
            return vec3(X + other.X, Y + other.Y, Z + other.Z);
        }

        inline vec3 sub(const vec3& other) const noexcept
        {
            return vec3(X - other.X, Y - other.Y, Z - other.Z);
        }

        inline vec3 scale(const float factor) const noexcept
        {
            return vec3(X * factor, Y * factor, Z * factor);
        }

        inline vec3 component_multiply(const vec3& other) const noexcept
        {
            return vec3(X * other.X, Y * other.Y, Z * other.Z);
        }

        inline vec3 component_divide(const vec3& other) const noexcept
        {
            return vec3(X / other.X, Y / other.Y, Z / other.Z);
        }

        inline float dot(const vec3& other) const noexcept
        {
            return X * other.X + Y * other.Y + Z * other.Z;
        }

        inline vec3 cross(const vec3& other) const noexcept
        {
            return vec3(
                Y * other.Z - Z * other.Y,
                Z * other.X - X * other.Z,
                X * other.Y - Y * other.X
            );
        }

        inline vec3 reflect(const vec3& normal) const noexcept
        {
            const float theta = dot(normal);

            return normal.scale(2 * theta).sub(*this);
        };

        inline vec3 refract(const vec3& normal, const float eta, bool* const total_reflection)
        {
            const float theta = dot(normal);
            const float k = 1 - (eta * eta * (1 - theta * theta));

            if (total_reflection)
                *total_reflection = k < 0;

            if (k < 0)
                return reflect(-normal);
            else
                return scale(eta).add(normal.scale(eta * theta - std::sqrt(k)));
        }

        inline vec3 transform(const std::vector<float>& matrix) const noexcept
        {
            return vec3(
                matrix[0] * X + matrix[1] * Y + matrix[2] * Z,
                matrix[3] * X + matrix[4] * Y + matrix[5] * Z,
                matrix[6] * X + matrix[7] * Y + matrix[8] * Z
            );
        }

        inline float distance_to(const vec3& other) const noexcept
        {
            return sub(other).length();
        }

        inline float angle_to(const vec3& other) const noexcept
        {
            return std::acos(normalize().dot(other.normalize()));
        }

        inline vec3 rotate(const float euler_x, const float euler_y, const float euler_z) const noexcept
        {
            return transform(create_rotation_matrix(euler_x, euler_y, euler_z));
        }

        inline vec3 rotate(EULER_ARG) noexcept
        {
            return rotate(euler_angles.X, euler_angles.Y, euler_angles.Z);
        }

        inline vec3 rotate(EULER_ARG, const vec3& origin) const noexcept
        {
            return sub(origin).rotate(euler_angles).add(origin);
        }

        inline vec3 rotate(const float euler_x, const float euler_y, const float euler_z, const vec3& origin) const noexcept
        {
            return sub(origin).rotate(euler_x, euler_y, euler_z).add(origin);
        }

        inline std::string to_string() const noexcept
        {
            std::stringstream ss;
            ss << '[' << X << "," << Y << "," << Z << ']';

            return ss.str();
        }

        static std::vector<float> create_rotation_matrix(EULER_ARG) noexcept
        {
            return create_rotation_matrix(euler_angles.X, euler_angles.Y, euler_angles.Z);
        }

        static std::vector<float> create_rotation_matrix(const float euler_x, const float euler_y, const float euler_z) noexcept
        {
            const float sx = std::sin(euler_x);
            const float cx = std::cos(euler_x);
            const float sy = std::sin(euler_y);
            const float cy = std::cos(euler_y);
            const float sz = std::sin(euler_z);
            const float cz = std::cos(euler_z);

            return std::vector<float>
            {
                cy* cz, sx* sy* cz - cx * sz, cx* sy* cz + sx * sz,
                    cx* sz, sx* sy* sz + cx * cz, cx* sy* sz - sy * cz,
                    sy, sx* cy, cx* cy
            };
        }

        OSTREAM_OPERATOR(vec3);
        CPP_IS_FUCKING_RETARDED(vec3);

        inline vec3 operator+() const noexcept
        {
            return *this;
        }

        inline vec3 operator-() const noexcept
        {
            return vec3(-X, -Y, -Z);
        }

        inline float operator[](int i) const
        {
            if (i == 0)
                return X;
            else if (i == 1)
                return Y;
            else if (i == 2)
                return Z;
            else
                throw std::range_error("Index is out of range!");
        }

        inline vec3 operator+(const vec3& other) const noexcept
        {
            return add(other);
        }

        inline vec3 operator-(const vec3& other) const noexcept
        {
            return sub(other);
        }

        inline vec3 operator*(const float factor) const noexcept
        {
            return scale(factor);
        }

        inline vec3 operator/(const float factor) const
        {
            return scale(1 / factor);
        }

        inline vec3 operator*(const vec3& other) const noexcept
        {
            return component_multiply(other);
        }

        inline vec3 operator/(const vec3& other) const noexcept
        {
            return component_divide(other);
        }

        inline vec3 operator+=(const vec3 other) const noexcept
        {
            return *this + other;
        }

        inline vec3 operator-=(const vec3 other) const noexcept
        {
            return *this - other;
        }

        inline vec3 operator*=(const float other) const noexcept
        {
            return *this * other;
        }

        inline vec3 operator/=(const float factor) const
        {
            return *this / factor;
        }

        inline vec3 operator*=(const vec3 other) const noexcept
        {
            return *this * other;
        }

        inline vec3 operator/=(const vec3 factor) const
        {
            return *this / factor;
        }

        inline operator ARGB() const noexcept
        {
            return ARGB(X, Y, Z);
        }
    };
};
