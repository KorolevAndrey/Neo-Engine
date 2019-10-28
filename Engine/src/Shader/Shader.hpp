#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <map>
#include <string>
#include <iostream>

namespace neo {

    class CameraComponent;

    class Shader {

        public:
            Shader(const std::string &);
            Shader(const std::string &, const std::string &, const std::string &);
            Shader(const std::string &, const std::string &, const std::string &, const std::string &);
            Shader(const std::string &, const char *, const char *);
            Shader(const std::string &, const char *, const std::string &);
            Shader(const std::string &, const char *, const char *, const char *);
            // Compute
            Shader(const std::string &, const std::string &);
            Shader(const std::string &, const char *);

            Shader(Shader&& rhs) = default;

            virtual ~Shader() = default;

            virtual void render(const CameraComponent &) {}
            virtual void imguiEditor() {}
            bool mActive = true;

            /* Names */
            const std::string mName = 0;

            /* Utility functions */
            void bind();
            void unbind();
            void reload();
            void cleanUp();
            void addAttribute(const std::string &);
            void addUniform(const std::string &);

            /* Parent load functions */
            void loadUniform(const std::string &, const bool) const;          // bool
            void loadUniform(const std::string &, const int) const;           // int
            void loadUniform(const std::string &, const GLuint) const;        // GLuint
            void loadUniform(const std::string &, const float) const;         // float
            void loadUniform(const std::string &, const glm::vec2 &) const;   // vec2
            void loadUniform(const std::string &, const glm::vec3 &) const;   // vec3
            void loadUniform(const std::string &, const glm::vec4 &) const;   // vec4
            void loadUniform(const std::string &, const glm::mat3 &) const;   // mat3
            void loadUniform(const std::string &, const glm::mat4 &) const;   // mat4

            /* Get shader location */
            GLint getAttribute(const std::string &) const;
            GLint getUniform(const std::string &) const;

        private:
            /* GLSL shader attributes */
            GLuint mPID = 0;
            GLint mVertexID = 0;
            GLint mFragmentID = 0;
            GLint mGeometryID = 0;
            GLint mComputeID = 0;
            std::map<std::string, GLint> mAttributes;
            std::map<std::string, GLint> mUniforms;

            std::string mVertexFile = "";
            std::string mFragmentFile = "";
            std::string mGeometryFile = "";
            std::string mComputeFile = "";
            const char* mVertexSource = nullptr;
            const char* mFragmentSource = nullptr;
            const char* mGeometrySource = nullptr;
            const char* mComputeSource = nullptr;

            void _init();
            std::string _getFullPath(const std::string&);
            GLuint _compileShader(GLenum, const char *);
            std::string _processShader(const char *);
            void _findAttributesAndUniforms(const char *);
    };
}