#pragma once

#include <cmath>
#include <iostream>

struct float2 {
    float x, y;

    float2(float x, float y) : x(x), y(y) {}

    float2(const float2 &other) = default;

    float2() = delete;

    static float2 direction(float radians) {
        return {
            std::cos(radians),
            std::sin(radians)
        };
    }

    float2 operator+(const float2 &other) const {
        float2 result = *this;
        result += other;
        return result;
    }

    float2 operator-(const float2 &other) const {
        float2 result = *this;
        result -= other;
        return result;
    }

    float2 operator*(const float2 &factor) const {
        float2 result = *this;
        result *= factor;
        return result;
    }

    float2 operator*(const float &factor) const {
        float2 result = *this;
        result *= factor;
        return result;
    }

    float2 operator/(const float &factor) const {
        float2 result = *this;
        result /= factor;
        return result;
    }

    void operator+=(const float2 &other) {
        this->x += other.x;
        this->y += other.y;

    }

    void operator-=(const float2 &other) {
        this->x += other.x;
        this->y += other.y;
    }

    void operator*=(const float2 &factor) {
        this->x *= factor.x;
        this->y *= factor.y;
    }

    void operator*=(const float &factor) {
        this->x *= factor;
        this->y *= factor;
    }

    void operator/=(const float &factor) {
        this->x /= factor;
        this->y /= factor;
    }

    friend std::ostream& operator<<(std::ostream &os, const float2 &value) {
        return os << '(' << value.x << ", " << value.y << ')';
    }
};