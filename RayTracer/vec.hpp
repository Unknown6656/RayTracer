
template <typename self, typename scalar, int size>
struct vec
{
    scalar coefficients[size];


    vec() noexcept : vec(0.f) {}

    vec(const scalar value) noexcept
    {
        for (int i = 0; i < size; ++i)
            coefficients[i] = value;
    }

    vec(const scalar* values) noexcept
    {
        for (int i = 0; i < size; ++i)
            coefficients[i] = values[i];
    }

    vec(const std::vector<scalar>& values) noexcept
    {
        for (int i = 0; i < size; ++i)
            coefficients[i] = values[i];
    }

    inline scalar squared_length() const noexcept
    {
        scalar length = scalar();

        for (int i = 0; i < size; ++i)
            length += coefficients[i] * coefficients[i];

        return length;
    }

    inline scalar length() const noexcept
    {
        return std::sqrt(squared_length());
    }

    inline self normalize() const
    {
        const scalar length = (scalar)1 / length();
        self v;


        const scalar coeff[size];

        for (int i = 0; i < size; ++i)
            coeff[i] *= length;

        return vec(coeff);
    }

    inline vec add(const vec& other) const noexcept
    {
        return vec2(X + other.X, Y + other.Y);
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

    inline vec3 rotate(const float& euler_x, const float& euler_y, const float& euler_z) const noexcept
    {
        return transform(create_rotation_matrix(euler_x, euler_y, euler_z));
    }

    inline vec3 rotate(const vec3& euler_angles) noexcept
    {
        return rotate(euler_angles.X, euler_angles.Y, euler_angles.Z);
    }

    inline std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << '[' << X << "," << Y << "," << Z << ']';

        return ss.str();
    }

    static std::vector<float> create_rotation_matrix(const vec3& euler_angles) noexcept
    {
        return create_rotation_matrix(euler_angles.X, euler_angles.Y, euler_angles.Z);
    }

    static std::vector<float> create_rotation_matrix(const float& euler_x, const float& euler_y, const float& euler_z) noexcept
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






};


