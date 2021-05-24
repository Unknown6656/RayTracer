#pragma once

#include "../argb.hpp"


namespace ray_tracer_2d
{
    struct vec2
    {
        static const vec2 Zero, UnitX, UnitY;

        const float X, Y;


        vec2() noexcept : vec2(0.f) {}

        vec2(float v) noexcept : vec2(v, v) {}

        vec2(float x, float y) noexcept : X(x), Y(y) {}

        vec2(float x, float y, float w) : vec2(x / w, y / w) {}

        // vec2(std::initializer_list<float> list) : vec2(process(list)) { }

        // vec2(const float* const arr) : vec2(arr[0], arr[1]) { }

        inline float squared_length() const noexcept
        {
            return X * X + Y * Y;
        }

        inline float length() const noexcept
        {
            return std::sqrt(squared_length());
        }

        inline vec2 normalize() const
        {
            return *this / length();
        }

        inline vec2 add(const vec2& other) const noexcept
        {
            return vec2(X + other.X, Y + other.Y);
        }

        inline vec2 sub(const vec2& other) const noexcept
        {
            return vec2(X - other.X, Y - other.Y);
        }

        inline vec2 scale(const float factor) const noexcept
        {
            return vec2(X * factor, Y * factor);
        }

        inline vec2 component_multiply(const vec2& other) const noexcept
        {
            return vec2(X * other.X, Y * other.Y);
        }

        inline vec2 component_divide(const vec2& other) const noexcept
        {
            return vec2(X / other.X, Y / other.Y);
        }

        inline float dot(const vec2& other) const noexcept
        {
            return X * other.X + Y * other.Y;
        }

        inline vec2 reflect(const vec2& normal) const noexcept
        {
            const float theta = dot(normal);

            return normal.scale(2 * theta).sub(*this);
        };

        inline vec2 refract(const vec2& normal, const float eta, bool* const total_reflection)
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

        inline vec2 transform(const std::vector<float>& matrix) const noexcept
        {
            return vec2(
                matrix[0] * X + matrix[1] * Y,
                matrix[2] * X + matrix[3] * Y
            );
        }

        inline float distance_to(const vec2& other) const noexcept
        {
            return sub(other).length();
        }

        inline float angle_to(const vec2& other) const noexcept
        {
            return std::acos(normalize().dot(other.normalize()));
        }

        inline vec2 rotate(const float angle) const noexcept
        {
            const float s = std::sin(angle);
            const float c = std::cos(angle);

            return transform(std::vector<float> { c, -s, s, c });
        }

        inline vec2 rotate(const float angle, const vec2& origin) const noexcept
        {
            return sub(origin).rotate(angle).add(origin);
        }

        TO_STRING(vec2, X << ", " << Y);
        CPP_IS_FUCKING_RETARDED(vec2);

        inline vec2 operator+() const noexcept
        {
            return *this;
        }

        inline vec2 operator-() const noexcept
        {
            return vec2(-X, -Y);
        }

        inline float operator[](int i) const
        {
            if (i == 0)
                return X;
            else if (i == 1)
                return Y;
            else
                throw std::range_error("Index is out of range!");
        }

        inline vec2 operator+(const vec2& other) const noexcept
        {
            return add(other);
        }

        inline vec2 operator-(const vec2& other) const noexcept
        {
            return sub(other);
        }

        inline vec2 operator*(const float factor) const noexcept
        {
            return scale(factor);
        }

        inline vec2 operator/(const float factor) const
        {
            return scale(1 / factor);
        }

        inline vec2 operator*(const vec2& other) const noexcept
        {
            return component_multiply(other);
        }

        inline vec2 operator/(const vec2& other) const noexcept
        {
            return component_divide(other);
        }

        inline vec2 operator+=(const vec2& other) const noexcept
        {
            return *this + other;
        }

        inline vec2 operator-=(const vec2& other) const noexcept
        {
            return *this - other;
        }

        inline vec2 operator*=(const float other) const noexcept
        {
            return *this * other;
        }

        inline vec2 operator/=(const float factor) const
        {
            return *this / factor;
        }

        inline vec2 operator*=(const vec2& other) const noexcept
        {
            return *this * other;
        }

        inline vec2 operator/=(const vec2& factor) const
        {
            return *this / factor;
        }
    };
};
