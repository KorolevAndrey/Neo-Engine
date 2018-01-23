#include "EntityShader.hpp"

bool EntityShader::init(std::vector<Entity *> *entities) {
    /* Parent init */
    if (!entities || !Shader::init()) {
        return false;
    }
    
    /* Set render target */
    this->entitiesPointer = entities;

    /* Set enum type */
    this->type = MasterRenderer::ShaderTypes::ENTITY_SHADER;

    addAllLocations();
        
    return true;
}

void EntityShader::addAllLocations() {
    /* Add attributes */
    addAttribute("vertexPos");
    addAttribute("vertexNormal");
    addAttribute("vertexTexture");

    /* Add matrix transforms */
    addUniform("P");
    addUniform("M");
    addUniform("V");
    addUniform("cameraPosition");

    /* Add texture info */
    addUniform("usesTexture");
    addUniform("textureImage");

    /* Add material properties */
    addUniform("matAmbient");
    addUniform("matDiffuse");
    addUniform("matSpecular");
    addUniform("shine");

    /* Add light */
    addUniform("lightPos");
    addUniform("lightCol");
    addUniform("lightAtt");
}

void EntityShader::render() {
    /* Loop through every entity */
    // TODO : batched render
    glm::mat4 M;
    for (auto &e : *entitiesPointer) {
        /* If entity mesh doesn't contain geometry, skip it */
        if (!e->mesh || !e->mesh->vertBuf.size()) {
            continue;
        }

        /* Load model matrix */
        loadM(&e->M);

        /* Load texture/material */
        loadTexture(e->modelTexture);

        /* Load mesh */
        loadMesh(e->mesh);

        /* Draw */
        glDrawElements(GL_TRIANGLES, (int)e->mesh->eleBuf.size(), GL_UNSIGNED_INT, nullptr);

        /* Unload mesh */
        unloadMesh(e->mesh);

        /* Unload texture/material */
        unloadTexture(e->modelTexture);
    }
}

/* Bind mesh attributes to VAO/VBOs 
 * All Meshes are assumed to have valid vertices and element indices 
 * Other attributes can be exlcluded */
void EntityShader::loadMesh(const Mesh *mesh) {
    /* Bind mesh VAO */
    glBindVertexArray(mesh->vaoId);
    
    /* Bind vertex buffer VBO */
    int pos = getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Bind normal buffer VBO */
    pos = getAttribute("vertexNormal");
    if (pos != -1 && mesh->norBufId != 0) {
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->norBufId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    /* Bind texture coordinate buffer VBO */
    pos = getAttribute("vertexTexture");
    if (pos != -1 && mesh->texBufId != 0) {
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->texBufId);
        glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    /* Bind indices buffer VBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId);
}

/* Bind material properties and  texture if provided */
void EntityShader::loadTexture(const ModelTexture &modelTexture) {
    /* Bind texture if it exists */
    if(modelTexture.texture && modelTexture.texture->textureId != 0) {
        loadUsesTexture(true);
        loadTexture(modelTexture.texture);
        glActiveTexture(GL_TEXTURE0 + modelTexture.texture->textureId);
        glBindTexture(GL_TEXTURE_2D, modelTexture.texture->textureId);
    }
    else {
        loadUsesTexture(false);
    }

    /* Bind materials */
    loadMaterial(modelTexture.ambientColor, 
                          modelTexture.diffuseColor, 
                          modelTexture.specularColor);
    loadShine(modelTexture.shineDamper);
}

/* Unbind mesh */
void EntityShader::unloadMesh(const Mesh *mesh) {
    glDisableVertexAttribArray(getAttribute("vertexPos"));

    int pos = getAttribute("vertexNormal");
    if (pos != -1) {
        glDisableVertexAttribArray(pos);
    }

    pos = getAttribute("vertexTexture");
    if (pos != -1) {
        glDisableVertexAttribArray(pos);
    }

    Shader::unloadMesh();
}

/* Unbind model/texture*/
void EntityShader::unloadTexture(const ModelTexture &modelTexture) {
    if (modelTexture.texture) {
        Shader::unloadTexture(modelTexture.texture->textureId);
    }
}

void EntityShader::cleanUp() {
    Shader::cleanUp();
}

void EntityShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void EntityShader::loadMaterial(const float ambient, const glm::vec3 diffuse, const glm::vec3 specular) {
    this->loadFloat(getUniform("matAmbient"), ambient);
    this->loadVec3(getUniform("matDiffuse"), diffuse);
    this->loadVec3(getUniform("matSpecular"), specular);
}

void EntityShader::loadShine(const float s) {
    this->loadFloat(getUniform("shine"), s);
}

void EntityShader::loadUsesTexture(const bool b) {
    this->loadBool(getUniform("usesTexture"), b);
}

void EntityShader::loadTexture(const Texture *texture) {
    this->loadInt(getUniform("textureImage"), texture->textureId);
}
