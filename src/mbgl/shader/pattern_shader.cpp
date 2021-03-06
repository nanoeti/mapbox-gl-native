#include <mbgl/shader/pattern_shader.hpp>
#include <mbgl/shader/pattern.vertex.hpp>
#include <mbgl/shader/pattern.fragment.hpp>
#include <mbgl/gl/gl.hpp>

using namespace mbgl;
using namespace shaders::pattern;

PatternShader::PatternShader(gl::ObjectStore& store)
    : Shader(::name, ::vertex, ::fragment, store) {
}

void PatternShader::bind(GLbyte *offset) {
    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, 0, offset));
}
