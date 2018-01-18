#include "MeshGenerator.hpp"

Mesh* MeshGenerator::generateCube(float scale) {
    Mesh *mesh = new Mesh;
    mesh->vertBuf = { 
            -scale,  scale, -scale,
            -scale, -scale, -scale,
             scale, -scale, -scale,
             scale, -scale, -scale,
             scale,  scale, -scale,
            -scale,  scale, -scale,
            
            -scale, -scale,  scale,
            -scale, -scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale,  scale,
            -scale, -scale,  scale,
            
             scale, -scale, -scale,
             scale, -scale,  scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
             scale,  scale, -scale,
             scale, -scale, -scale,
            
            -scale, -scale,  scale,
            -scale,  scale,  scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
             scale, -scale,  scale,
            -scale, -scale,  scale,
            
            -scale,  scale, -scale,
             scale,  scale, -scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
            -scale,  scale,  scale,
            -scale,  scale, -scale,
            
            -scale, -scale, -scale,
            -scale, -scale,  scale,
             scale, -scale, -scale,
             scale, -scale, -scale,
            -scale, -scale,  scale,
             scale, -scale,  scale
        };

    return mesh;
}

Mesh* MeshGenerator::generateSphere(int smoothness) {
    Mesh *mesh = new Mesh;

    /* Vertices */
    float t = (float) ((1.0 + sqrt(5.0)) / 2.0);
    mesh->vertBuf = std::vector<float>{
        -1,  t,  0,
         1,  t,  0,
        -1, -t,  0,
         1, -t,  0,

         0, -1,  t,
         0,  1,  t,
         0, -1, -t,
         0,  1, -t,

         t,  0, -1,
         t,  0,  1,
        -t,  0, -1,
        -t,  0,  1,
    };


    mesh->eleBuf = std::vector<unsigned int>{
        /* 5 faces around point 0 */
         0, 11,  5,
         0,  5,  1,
         0,  1,  7,
         0,  7, 10,
         0, 10, 11,
        /* 5 adjacent faces */
         1,  5,  9,
         5, 11,  4,
        11, 10,  2,
        10,  7,  6,
         7,  1,  8,
        /* 5 faces around point 3 */
         3,  9,  4,
         3,  4,  2,
         3,  2,  6,
         3,  6,  8,
         3,  8,  9,
        /* 5 adjacent faces */
         4,  9,  5,
         2,  4, 11,
         6,  2, 10,
         8,  6,  7,
         9,  8,  1
    };
}
