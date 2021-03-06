#include <mbgl/shader/icon_shader.hpp>
#include <mbgl/shader/icon.vertex.hpp>
#include <mbgl/shader/icon.fragment.hpp>
#include <mbgl/gl/gl.hpp>

using namespace mbgl;
using namespace shaders::icon;

IconShader::IconShader(gl::ObjectStore& store)
    : Shader(::name, ::vertex, ::fragment, store)
    , a_offset(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_offset")))
    , a_data1(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_data1")))
    , a_data2(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_data2"))) {
}

void IconShader::bind(GLbyte* offset) {
    const GLsizei stride = 16;

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, stride, offset + 0));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_offset));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_offset, 2, GL_SHORT, false, stride, offset + 4));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_data1));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_data1, 4, GL_UNSIGNED_BYTE, false, stride, offset + 8));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_data2));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_data2, 4, GL_UNSIGNED_BYTE, false, stride, offset + 12));
}
