#ifndef GAME_H
#define GAME_H

// #include <glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <glm/glm.hpp>
#include <vector>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "pool.h"
#include "list.h"
#include "gfx.h"

#define SPEEDTHRESHOLD 0.01f

#define BOLA      0
#define DOG       1
#define CEU       2
#define SHIP      3
#define EYES      4
#define FACE      5
#define ASTEROID  6
#define GLASS     7
#define SUN       8
#define BLACKHOLE 9

struct ObjModel : public PoolElem
{
    struct Vertex
    {
        float m_position[4];
        float m_normal[4];
        float m_texCoords[2];
    };

    struct Batch
    {
        uint32_t    m_start;
        uint32_t    m_count;
    };

    // tinyobj::attrib_t                   m_attrib;
    // std::vector<tinyobj::shape_t>       m_shapes;
    // std::vector<tinyobj::material_t>    m_materials;

    Batch *     m_batches;
    uint32_t    m_batchCount;

    GLenum      m_drawMode;
    GLuint      m_vao;
    uint32_t    m_modelStart;
    uint32_t    m_modelCount;

    glm::vec3   m_min;
    glm::vec3   m_max;

    // glm::vec3                           m_bboxMin;            // Axis-Aligned Bounding Box do objeto
    // glm::vec3                           m_bboxMax;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    // ObjModel(const char *filename, const char *basepath = NULL, bool triangulate = true);
};

struct SceneObject : public PoolElem
{
    // size_t      m_first_index;            // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    // size_t      m_num_indices;            // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    // GLenum      m_rendering_mode;         // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    // GLuint      m_vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    ObjModel*       m_model;

    glm::vec3       m_bboxMin;            // Axis-Aligned Bounding Box do objeto
    glm::vec3       m_bboxMax;

    glm::mat4       m_transform;


    void update();
};

struct Player
{
    SceneObject *m_dogObject;
    SceneObject *m_shipObject;
    SceneObject *m_glassObject;
};

struct GameContext
{
    static Pool<SceneObject, 1024>     m_sceneObjects;
    static Pool<ObjModel, 64>          m_models;
    // static std::unordered_map<std::string, ObjModel>    m_models;
    static List<SceneObject *, 512>    m_asteroids;

    static void init();
    static void shutdown();

    static ObjModel *loadModel(const char *filename, const char *basepath = NULL, bool triangulate = true);
    static void destroyModel(ObjModel *model);

    static SceneObject *createSceneObject(glm::mat4& transform, ObjModel *model);
    static void destroySceneObject(SceneObject *object);

    static ObjModel *   m_sphereModel;
    static ObjModel *   m_dogModel;
    static ObjModel *   m_shipModel;
    static ObjModel *   m_vidroModel;
    static ObjModel *   m_asteroidModel;

    static Texture *    m_mapTexture;
    static Texture *    m_starsTexture;
    static Texture *    m_dogTexture;
    static Texture *    m_metalTexture;
    static Texture *    m_eyesTexture;
    static Texture *    m_asteroidTexture;
    static Texture *    m_glassTexture;
    static Texture *    m_sunTexture;

    static Shader *     m_doItAllShader;

    static SceneObject *m_sunObject;
    static SceneObject *m_earthObject;

    static Player       m_player;
};

void updatePlayer(float deltaTime);

void updateAsteroids(float deltaTime);

void update(float deltaTime);

void draw();

// void updateSpeed();

#endif