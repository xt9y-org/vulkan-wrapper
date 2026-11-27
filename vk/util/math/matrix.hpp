#pragma once
#include "quaternion.hpp"
#include <stdexcept>

namespace vk::util::math {
struct mat4 {
    std::array<float,16> v{};
    constexpr float &operator()(std::size_t row, std::size_t col) { return v[col*4+row]; }
    constexpr float operator()(std::size_t row, std::size_t col) const { return v[col*4+row]; }
    float *data() { return v.data(); }
    const float *data() const { return v.data(); }
    static constexpr mat4 identity() {
        mat4 m;
        m(0,0)=m(1,1)=m(2,2)=m(3,3)=1;
        return m;
    }
};

constexpr mat4 operator*(mat4 a, mat4 b) {
    mat4 result;
    for (std::size_t r=0; r<4; ++r)
        for (std::size_t c=0; c<4; ++c)
            for (std::size_t k=0; k<4; ++k)
                result(r,c)+=a(r,k)*b(k,c);
    return result;
}
constexpr vec4 operator*(mat4 m, vec4 v) {
    return {
        m(0,0)*v.x + m(0,1)*v.y + m(0,2)*v.z + m(0,3)*v.w,
        m(1,0)*v.x + m(1,1)*v.y + m(1,2)*v.z + m(1,3)*v.w,
        m(2,0)*v.x + m(2,1)*v.y + m(2,2)*v.z + m(2,3)*v.w,
        m(3,0)*v.x + m(3,1)*v.y + m(3,2)*v.z + m(3,3)*v.w
    };
}
inline mat4 transpose(mat4 a) {
    mat4 b;
    for (std::size_t r=0;r<4;++r)
        for (std::size_t c=0;c<4;++c) b(r,c)=a(c,r);
    return b;
}
inline mat4 translate(vec3 t) {
    auto m=mat4::identity();
    m(0,3)=t.x; m(1,3)=t.y; m(2,3)=t.z;
    return m;
}
inline mat4 scale(vec3 s) {
    auto m=mat4::identity();
    m(0,0)=s.x; m(1,1)=s.y; m(2,2)=s.z;
    return m;
}
inline mat4 rotation(quat q) {
    q=normalize(q);
    float x=q.x,y=q.y,z=q.z,w=q.w;
    auto m=mat4::identity();
    m(0,0)=1-2*(y*y+z*z); m(0,1)=2*(x*y-z*w); m(0,2)=2*(x*z+y*w);
    m(1,0)=2*(x*y+z*w); m(1,1)=1-2*(x*x+z*z); m(1,2)=2*(y*z-x*w);
    m(2,0)=2*(x*z-y*w); m(2,1)=2*(y*z+x*w); m(2,2)=1-2*(x*x+y*y);
    return m;
}
inline mat4 trs(vec3 position, quat orientation, vec3 scaling) {
    return translate(position)*rotation(orientation)*scale(scaling);
}
inline mat4 look_at(vec3 eye, vec3 target, vec3 up={0,1,0}) {
    vec3 f=normalize(target-eye), s=normalize(cross(f,up)), u=cross(s,f);
    auto m=mat4::identity();
    m(0,0)=s.x; m(0,1)=s.y; m(0,2)=s.z; m(0,3)=-dot(s,eye);
    m(1,0)=u.x; m(1,1)=u.y; m(1,2)=u.z; m(1,3)=-dot(u,eye);
    m(2,0)=-f.x; m(2,1)=-f.y; m(2,2)=-f.z; m(2,3)=dot(f,eye);
    return m;
}
inline mat4 perspective(float fov_y, float aspect, float near_z, float far_z,
                        bool flip_y=true) {
    mat4 m;
    float f=1.0f/std::tan(fov_y*0.5f);
    m(0,0)=f/aspect;
    m(1,1)=flip_y ? -f : f;
    m(2,2)=far_z/(near_z-far_z);
    m(2,3)=(far_z*near_z)/(near_z-far_z);
    m(3,2)=-1;
    return m;
}
inline mat4 orthographic(float left, float right, float bottom, float top,
                         float near_z, float far_z, bool flip_y=true) {
    auto m=mat4::identity();
    m(0,0)=2/(right-left);
    m(1,1)=(flip_y ? -2.0f : 2.0f)/(top-bottom);
    m(2,2)=1/(near_z-far_z);
    m(0,3)=-(right+left)/(right-left);
    m(1,3)=-(top+bottom)/(top-bottom)*(flip_y ? -1.0f : 1.0f);
    m(2,3)=near_z/(near_z-far_z);
    return m;
}
inline mat4 inverse_affine(mat4 m)
{
    const vec3 a{m(0, 0), m(1, 0), m(2, 0)}, b{m(0, 1), m(1, 1), m(2, 1)}, c{m(0, 2), m(1, 2), m(2, 2)};

    const float determinant = dot(a, cross(b, c));

    if (std::abs(determinant) < 1e-10f)
        throw std::runtime_error("Singular affine transform");

    const auto r0 = cross(b, c) / determinant, r1 = cross(c, a) / determinant, r2 = cross(a, b) / determinant;
    auto out = mat4::identity();

    out(0,0)=r0.x; out(0,1)=r0.y; out(0,2)=r0.z;
    out(1,0)=r1.x; out(1,1)=r1.y; out(1,2)=r1.z;
    out(2,0)=r2.x; out(2,1)=r2.y; out(2,2)=r2.z;

    const vec3 p{m(0, 3), m(1, 3), m(2, 3)};
    out(0, 3) = -dot(r0, p);
    out(1, 3) = -dot(r1, p);
    out(2, 3) = -dot(r2, p);

    return out;
}

inline quat matrix_orientation(mat4 matrix)
{
    const vec3 x = normalize(vec3{matrix(0, 0), matrix(1, 0), matrix(2, 0)});
    const vec3 y = normalize(vec3{matrix(0, 1), matrix(1, 1), matrix(2, 1)});
    const vec3 z = normalize(cross(x, y));
    const std::array<vec3, 3> axis{x, y, z};
    const auto m = [&](int row, int col)
    { const auto &v=axis[col]; return row==0?v.x:row==1?v.y:v.z; };
    const float trace = m(0, 0) + m(1, 1) + m(2, 2);
    quat q{};
    if (trace > 0) {
        const float k = 2 * std::sqrt(trace + 1);
        q = {(m(2, 1) - m(1, 2)) / k, (m(0, 2) - m(2, 0)) / k, (m(1, 0) - m(0, 1)) / k, k * .25f};
    }
    else {
        int a = m(1, 1) > m(0, 0) ? 1 : 0;
        if (m(2, 2) > m(a, a)) a = 2;
        const int b = (a + 1) % 3, c = (a + 2) % 3;
        const float k = 2 * std::sqrt(1 + m(a, a) - m(b, b) - m(c, c));
        float v[4]{};
        v[a] = k * .25f;
        v[b] = (m(b, a) + m(a, b)) / k;
        v[c] = (m(c, a) + m(a, c)) / k;
        v[3] = (m(c, b) - m(b, c)) / k;
        q = {v[0], v[1], v[2], v[3]};
    }
    return normalize(q);
}
using mat4f=mat4;
}
namespace vk::util::math::matrix {
using ::vk::util::math::mat4;
using ::vk::util::math::mat4f;
using ::vk::util::math::transpose;
using ::vk::util::math::inverse_affine;
using ::vk::util::math::matrix_orientation;
using ::vk::util::math::translate;
using ::vk::util::math::scale;
using ::vk::util::math::rotation;
using ::vk::util::math::trs;
using ::vk::util::math::look_at;
using ::vk::util::math::perspective;
using ::vk::util::math::orthographic;
}
