#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include <string>
#include <vector>

#include "Renderer/GLObjects/Material.hpp"
#include "Renderer/GLObjects/Texture2D.hpp"
#include "Renderer/GLObjects/TextureCubeMap.hpp"

namespace neo {

    class Mesh;

    struct Asset {
        const Mesh& mesh;
        Material material;
        Texture* ambientTexture = nullptr;      // map_Ka
        Texture* diffuseTexture = nullptr;      // map_Kd
        Texture* specularTexture = nullptr;     // map_Ks
        Texture* displacementTexture = nullptr; // disp
        Texture* alphaTex = nullptr;        // map_d
        // Texture* bumpTex = nullptr;         // map_bump, bump
        // Texture* specularHighlightTex;     // map_Ns

        Asset(const Mesh& mesh) :
            mesh(mesh)
        {}
    };


    class Loader {

        public:
            static void init(const std::string &, bool);

            // Stich .obj objects together into a single mesh
            static Mesh* loadStitchedMesh(const std::string &, bool = false);
            // Return list of .obj assets
            static const std::vector<Asset> loadMultiAsset(const std::string &);

            /* Retrieve Texture pointer from an image file */
            static Texture2D* loadTexture(const std::string &, TextureFormat);
            static TextureCubeMap* loadTexture(const std::string &, const std::vector<std::string> &);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void _resize(Mesh*, std::vector<float>&, bool);

            /* Load a single texture file */
            static uint8_t* _loadTextureData(int&, int&, int&, const std::string&, TextureFormat, bool = true);
            static void _cleanTextureData(uint8_t*);

            /* Private members */
            static std::string RES_DIR;
            static bool mVerbose;


    };
}