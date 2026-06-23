#include <bits/stdc++.h>
using namespace std;

constexpr long double EPS = 1e-9L;
constexpr bool eq(long double a, long double b) noexcept { return fabsl(a - b) < EPS; }

template <typename T>
struct Vector {
    T x, y, z;
    Vector(T x_ = 0, T y_ = 0, T z_ = 0) : x(x_), y(y_), z(z_) {}

    template <typename U> constexpr auto operator+(const Vector<U>& v) const { return Vector<common_type_t<T, U>>(x + v.x, y + v.y, z + v.z); }
    template <typename U> constexpr auto operator-(const Vector<U>& v) const { return Vector<common_type_t<T, U>>(x - v.x, y - v.y, z - v.z); }
    template <typename U> constexpr auto operator*(U k) const { return Vector<common_type_t<T, U>>(x * k, y * k, z * k); }
    template <typename U> constexpr auto operator/(U k) const { return Vector<common_type_t<T, U>>(x / k, y / k, z / k); }

    constexpr Vector& operator+=(const Vector& v) { x += v.x; y += v.y; z += v.z; return *this; }
    constexpr Vector& operator-=(const Vector& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    constexpr Vector& operator*=(auto k) { x *= k; y *= k; z *= k; return *this; }
    constexpr Vector& operator/=(auto k) { x /= k; y /= k; z /= k; return *this; }

    template <typename U> friend constexpr auto operator*(U k, const Vector<T>& v) { return v * k; }
    friend ostream& operator<<(ostream& os, const Vector& v) {
        os << "<" << v.x << ", " << v.y << ", " << v.z << ">";
        return os;
    }
    
    template<typename U> constexpr common_type_t<T, U> dot(const Vector<U>& v) const { return x * v.x + y * v.y + z * v.z; }
    template<typename U> constexpr auto cross(const Vector<U>& v) const {
        return Vector<common_type_t<T, U>>(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    template<typename U> constexpr common_type_t<T, U> distance2(const Vector<U>& v) const { return (*this - v).norm2(); }
    template<typename U> long double distance(const Vector<U>& v) const { return (*this - v).norm(); }

    template<typename U> constexpr auto max(const Vector<U>& v) const { return (norm2() >= v.norm2() ? *this : v); }
    template<typename U> constexpr auto min(const Vector<U>& v) const { return (norm2() <= v.norm2() ? *this : v); }
    
    template<typename U> long double angle(const Vector<U>& v) const {
        long double na = norm(), nb = v.norm();
        if(na < EPS || nb < EPS) return 0;
        long double cos_theta = dot(v) / (na * nb);
        if(cos_theta > 1) cos_theta = 1.0L;
        if(cos_theta < -1) cos_theta = -1.0L;
        return acosl(cos_theta);
    }

    constexpr T norm2() const { return x * x + y * y + z * z; }
    long double norm() const { return sqrt((long double)norm2()); }
    Vector<long double> unit() const {
        long double n = norm();
        if(n < EPS) return {0.0L,0.0L,0.0L};
        return *this / n;
    }
};

template <typename T>
struct Point {
    T x, y, z;
    Point(T x_ = 0, T y_ = 0, T z_ = 0) : x(x_), y(y_), z(z_) {}

    constexpr bool operator<(const Point& p) const {
        if(!eq(x, p.x)) return x < p.x;
        if(!eq(y, p.y)) return y < p.y;
        if(!eq(z, p.z)) return z < p.z;
        return false;
    }
    constexpr bool operator>(const Point& p) const { return p < *this; }
    constexpr bool operator<=(const Point& p) const { return !(p < *this); }
    constexpr bool operator>=(const Point& p) const { return !(*this < p); }
    constexpr bool operator==(const Point& p) const { return eq(x, p.x) && eq(y, p.y) && eq(z, p.z); }

    template <typename U> constexpr auto operator-(const Point<U>& p) const { return Vector<common_type_t<T, U>>(x - p.x, y - p.y, z - p.z); }
    template <typename U> constexpr auto operator+(const Vector<U>& v) const { return Point<common_type_t<T, U>>(x + v.x, y + v.y, z + v.z); }
    template <typename U> constexpr auto operator-(const Vector<U>& v) const { return Point<common_type_t<T, U>>(x - v.x, y - v.y, z - v.z); }
    
    template <typename U> constexpr Point& operator+=(const Vector<U>& v) { x += v.x; y += v.y; z += v.z; return *this; }
    template <typename U> constexpr Point& operator-=(const Vector<U>& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    
    friend ostream& operator<<(ostream& os, const Point& p) {
        os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
        return os;
    }
};

template<typename A, typename B, typename C>
inline long double triangle_area(const Point<A>& a, const Point<B>& b, const Point<C>& c){
    Vector<common_type_t<A, B, C>> ab = b - a, ac = c - a;
    return ab.cross(ac).norm() * 0.5L;
}

template<typename P, typename A, typename B>
inline long double distance_point_to_line(const Point<P>& p, const Point<A>& a, const Point<B>& b){
    Vector<common_type_t<P, A, B>> ab = b - a, ap = p - a;
    long double area = ap.cross(ab).norm(), base = ab.norm();
    if(base < EPS) return ap.norm();
    return area / base;
}
