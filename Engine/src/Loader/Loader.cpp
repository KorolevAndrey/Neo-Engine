#include "Loader.hpp"

#include "MeshGenerator.hpp"

#include "GLHelper/Texture.hpp"
#include "GLHelper/GLHelper.hpp"
#include "GLHelper/Framebuffer.hpp"
#include "Material/Material.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#include <iostream>

namespace neo {

    bool Loader::verbose = false;
    std::string Loader::RES_DIR = "";

    /* Library */
    std::unordered_map<std::string, Mesh *> Loader::meshes;
    std::unordered_map<std::string, Texture *> Loader::textures;
    std::unordered_map<std::string, Framebuffer *> Loader::framebuffers;
    std::unordered_map<std::string, Material *> Loader::materials;

    void Loader::init(const std::string &res, bool v) {
        RES_DIR = res;
        verbose = v;

    }

    Mesh * Loader::getMesh(const std::string &fileName, bool doResize) {
        /* Search map first */
        auto it = meshes.find(fileName);
        if (it != meshes.end()) {
            return it->second;
        }

        /* Check with static meshes first */
        if (!std::strcmp(fileName.c_str(), "cube")) {
            meshes.insert({ "cube", MeshGenerator::createCube() });
            return meshes.find(fileName)->second;
        }
        if (!std::strcmp(fileName.c_str(), "quad")) {
            meshes.insert({ "quad", MeshGenerator::createQuad() });
            return meshes.find(fileName)->second;
        }
        if (!std::strcmp(fileName.c_str(), "sphere")) {
            Mesh *mesh = MeshGenerator::createSphere(3);
            meshes.insert({ "sphere", mesh });
            // sphere defaults to ico_2
            meshes.insert({ "ico_2", mesh });
            return meshes.find(fileName)->second;
        }
        if (!std::strncmp(fileName.c_str(), "ico_", 4)) {
            int recursions = std::stoi(fileName.c_str() + 4);
            meshes.insert({ "ico_" + std::to_string(recursions), MeshGenerator::createSphere(recursions) });
            return meshes.find(fileName)->second;
        }

        /* If mesh was not found in map, read it in */
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string errString;
        // TODO : use assimp or another optimized asset loader
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (RES_DIR + fileName).c_str());
        if (!rc) {
            std::cerr << errString << std::endl;
            std::cin.get();
            exit(1);
        }

        /* Create a new empty mesh */
        Mesh * mesh = new Mesh;

        int vertCount = 0;
        /* For every shape in the loaded file */
        for (unsigned int i = 0; i < shapes.size(); i++) {
            /* Concatenate the shape's vertices, normals, and textures to the mesh */
            mesh->buffers.vertBuf.insert(mesh->buffers.vertBuf.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
            mesh->buffers.norBuf.insert(mesh->buffers.norBuf.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
            mesh->buffers.texBuf.insert(mesh->buffers.texBuf.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

            /* Concatenate the shape's indices to the new mesh
             * Indices need to be incremented as we concatenate shapes */
            for (unsigned int i : shapes[i].mesh.indices) {
                mesh->buffers.eleBuf.push_back(i + vertCount);
            }
            vertCount += int(shapes[i].mesh.positions.size()) / 3;
        }

        /* Provide VBO info */
        mesh->vertBufSize = int(mesh->buffers.vertBuf.size());
        mesh->norBufSize = int(mesh->buffers.norBuf.size());
        mesh->texBufSize = int(mesh->buffers.texBuf.size());
        mesh->eleBufSize = int(mesh->buffers.eleBuf.size());

        /* Optional resize */
        if (doResize) {
            resize(mesh->buffers);
        }

        /* Add new mesh to library */
        meshes.insert({ fileName, mesh });

        /* Load mesh to GPU */
        mesh->upload();

        if (verbose) {
            std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;
        }

        return mesh;
    }

    Texture2D * Loader::getTexture(const std::string &fileName, GLint inFormat, GLenum format, GLint filter, GLenum mode) {
        Texture *texture = findTexture(fileName);

        /* Create an empty texture if it is not already exist in the library */
        if (!texture) {
            texture = new Texture2D;

            /* Use stbi if name is an existing file */
            FILE *f;
            if (fopen_s(&f, (RES_DIR + fileName).c_str(), "rb")) {
                stbi_set_flip_vertically_on_load(true);
                uint8_t *data = stbi_load((RES_DIR + fileName).c_str(), &texture->width, &texture->height, &texture->components, STBI_rgb_alpha);   // TODO - allow ability to specify number of components
                if (data) {
                    texture->upload(inFormat, format, filter, mode, true, &data);
                    texture->generateMipMaps();
                    stbi_image_free(data);
                    if (verbose) {
                        std::cout << "Loaded texture " << fileName << " [" << texture->width << ", " << texture->height << "]" << std::endl;
                    }
                }
                else {
                    std::cerr << "Error opening texture file " << fileName << std::endl;
                }
            }

            textures.insert({ fileName, texture });
        }
        return (Texture2D *)texture;
    }

    TextureCubeMap * Loader::getTexture(const std::string &name, const std::vector<std::string> & files) {
        Texture *texture = findTexture(name);

        /* Create an empty texture if it is not already exist in the library */
        if (!texture) {
            texture = new TextureCubeMap;

            /* Use stbi if name is an existing file */
            FILE *f;
            if (fopen_s(&f, (RES_DIR + name).c_str(), "rb")) {
                uint8_t* data[6];
                for (int i = 0; i < 6; i++) {
                    data[i] = stbi_load((RES_DIR + files[i]).c_str(), &texture->width, &texture->height, &texture->components, STBI_rgb_alpha);
                    if (data[i]) {
                        if (verbose) {
                            std::cout << "Loaded texture " << files[i] << " [" << texture->width << ", " << texture->height << "]" << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Could not find texture file " << files[i] << std::endl;
                    }
                }

                /* Upload data to GPU and free from CPU */
                texture->upload(GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE, true, data);
                for (int i = 0; i < 6; i++) {
                    stbi_image_free(data[i]);
                }
            }

            textures.insert({ name, texture });

        }
    
        return (TextureCubeMap *) texture;
    }

    Framebuffer * Loader::getFBO(const std::string &name) {
        auto it = framebuffers.find(name);
        if (it == framebuffers.end()) {
            framebuffers.emplace(name, new Framebuffer);
            it = framebuffers.find(name);
        }
        return it->second;
    }

    Material * Loader::getMaterial(const std::string &name, float amb, glm::vec3 dif, glm::vec3 spec, float shine) {
        auto it = materials.find(name);
        if (it == materials.end()) {    
            materials.emplace(name, new Material(amb, dif, spec, shine));
            it = materials.find(name);
        }
        else {
            it->second->ambient = amb;
            it->second->diffuse = dif;
            it->second->specular = spec;
            it->second->shine = shine;
        }
        return it->second;
    }

    Texture * Loader::findTexture(const std::string &name) {
        auto it = textures.find(name);
        if (it == textures.end()) {
            return nullptr;
        }
        else {
            return it->second;
        }
    }

    /* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
    void Loader::resize(Mesh::MeshBuffers & buffers) {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
        float scaleX, scaleY, scaleZ;
        float shiftX, shiftY, shiftZ;
        float epsilon = 0.001f;

        minX = minY = minZ = 1.1754E+38F;
        maxX = maxY = maxZ = -1.1754E+38F;

        //Go through all vertices to determine min and max of each dimension
        for (size_t v = 0; v < buffers.vertBuf.size() / 3; v++) {
            if (buffers.vertBuf[3 * v + 0] < minX) minX = buffers.vertBuf[3 * v + 0];
            if (buffers.vertBuf[3 * v + 0] > maxX) maxX = buffers.vertBuf[3 * v + 0];

            if (buffers.vertBuf[3 * v + 1] < minY) minY = buffers.vertBuf[3 * v + 1];
            if (buffers.vertBuf[3 * v + 1] > maxY) maxY = buffers.vertBuf[3 * v + 1];

            if (buffers.vertBuf[3 * v + 2] < minZ) minZ = buffers.vertBuf[3 * v + 2];
            if (buffers.vertBuf[3 * v + 2] > maxZ) maxZ = buffers.vertBuf[3 * v + 2];
        }

        //From min and max compute necessary scale and shift for each dimension
        float maxExtent, xExtent, yExtent, zExtent;
        xExtent = maxX - minX;
        yExtent = maxY - minY;
        zExtent = maxZ - minZ;
        if (xExtent >= yExtent && xExtent >= zExtent) {
            maxExtent = xExtent;
        }
        if (yExtent >= xExtent && yExtent >= zExtent) {
            maxExtent = yExtent;
        }
        if (zExtent >= xExtent && zExtent >= yExtent) {
            maxExtent = zExtent;
        }
        scaleX = 2.f / maxExtent;
        shiftX = minX + (xExtent / 2.f);
        scaleY = 2.f / maxExtent;
        shiftY = minY + (yExtent / 2.f);
        scaleZ = 2.f / maxExtent;
        shiftZ = minZ + (zExtent) / 2.f;

        //Go through all verticies shift and scale them
        for (size_t v = 0; v < buffers.vertBuf.size() / 3; v++) {
            buffers.vertBuf[3 * v + 0] = (buffers.vertBuf[3 * v + 0] - shiftX) * scaleX;
            assert(buffers.vertBuf[3 * v + 0] >= -1.0 - epsilon);
            assert(buffers.vertBuf[3 * v + 0] <= 1.0 + epsilon);
            buffers.vertBuf[3 * v + 1] = (buffers.vertBuf[3 * v + 1] - shiftY) * scaleY;
            assert(buffers.vertBuf[3 * v + 1] >= -1.0 - epsilon);
            assert(buffers.vertBuf[3 * v + 1] <= 1.0 + epsilon);
            buffers.vertBuf[3 * v + 2] = (buffers.vertBuf[3 * v + 2] - shiftZ) * scaleZ;
            assert(buffers.vertBuf[3 * v + 2] >= -1.0 - epsilon);
            assert(buffers.vertBuf[3 * v + 2] <= 1.0 + epsilon);
        }
    }
}