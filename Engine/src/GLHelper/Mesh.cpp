#include "Mesh.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLHelper/GLHelper.hpp"

namespace neo {

    void Mesh::draw(unsigned size) const {
        if (eleBufSize) {
            // TODO - instanced?
            CHECK_GL(glDrawElements(mode, size ? size : eleBufSize, GL_UNSIGNED_INT, nullptr));
        }
        else {
            int vSize = size;
            if (!vSize) {
                switch (mode) {
                case GL_POINTS:
                case GL_LINE_STRIP:
                    vSize = vertBufSize / 3;
                case GL_LINES:
                    vSize = vertBufSize / 6;
                case GL_TRIANGLES:
                    vSize = vertBufSize / 9;
                default:
                    vSize = vertBufSize;
                }
            }
            CHECK_GL(glDrawArrays(mode, 0, vSize));
        }
    }

    void Mesh::upload(unsigned type) {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *) &vaoId));
        CHECK_GL(glBindVertexArray(vaoId));

        /* Copy vertex array */
        CHECK_GL(glGenBuffers(1, (GLuint *) &vertBufId));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertBufId));
        if (buffers.vertBuf.size()) {
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, buffers.vertBuf.size() * sizeof(float), &buffers.vertBuf[0], GL_STATIC_DRAW));
        }
        CHECK_GL(glEnableVertexAttribArray(0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertBufId));
        CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

        /* Copy normal array if it exists */
        if (!buffers.norBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &norBufId));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, norBufId));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, buffers.norBuf.size() * sizeof(float), &buffers.norBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(1));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, norBufId));
            CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy texture array if it exists */
        if (!buffers.texBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &texBufId));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, texBufId));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, buffers.texBuf.size() * sizeof(float), &buffers.texBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(2));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, texBufId));
            CHECK_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy element array if it exists -- also set mode */
        if (!buffers.eleBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &eleBufId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufId));
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffers.eleBuf.size() * sizeof(unsigned int), &buffers.eleBuf[0], GL_STATIC_DRAW));
        }

        if (type) {
            mode = type;
        }
        else if (!buffers.eleBuf.empty()) {
            mode = GL_TRIANGLES;
        }
        else {
            mode = GL_TRIANGLE_STRIP;
        }

        /* Unbind  */
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        /* Error check */
        assert(glGetError() == GL_NO_ERROR);
    }
}