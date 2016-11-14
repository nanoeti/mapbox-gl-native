#include <mbgl/gl/extension.hpp>
#include <mbgl/gl/gl.hpp>

#include <mutex>
#include <string>
#include <vector>

namespace mbgl {
namespace gl {
namespace detail {

using Probes = std::vector<ExtensionFunctionBase::Probe>;
using ExtensionFunctions = std::vector<std::pair<glProc*, Probes>>;
ExtensionFunctions& extensionFunctions() {
    static ExtensionFunctions functions;
    return functions;
}

ExtensionFunctionBase::ExtensionFunctionBase(std::initializer_list<Probe> probes) {
    extensionFunctions().emplace_back(&ptr, probes);
}

} // namespace detail

static std::once_flag initializeExtensionsOnce;

void InitializeExtensions(glProc (*getProcAddress)(const char*)) {
    std::call_once(initializeExtensionsOnce, [getProcAddress] {
        const char* extensionsPtr =
            reinterpret_cast<const char*>(MBGL_CHECK_ERROR(glGetString(GL_EXTENSIONS)));

        if (!extensionsPtr)
            return;

        const std::string extensions = extensionsPtr;
        for (auto fn : detail::extensionFunctions()) {
            for (auto probe : fn.second) {
                if (extensions.find(probe.first) != std::string::npos) {
                    *fn.first = getProcAddress(probe.second);
                    break;
                }
            }
        }
    });
}

} // namespace gl
} // namespace mbgl
