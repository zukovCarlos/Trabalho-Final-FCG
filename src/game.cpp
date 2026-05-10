#include "game.h"
#include "maths.h"
#include <exception>
#include <stdio.h>
#include <glm/gtc/type_ptr.hpp>

// ObjModel::ObjModel(const char *filename, const char *basepath, bool triangulate){
//         printf("Carregando objetos do arquivo \"%s\"...\n", filename);

//         // Se basepath == NULL, então setamos basepath como o dirname do
//         // filename, para que os arquivos MTL sejam corretamente carregados caso
//         // estejam no mesmo diretório dos arquivos OBJ.
//         std::string fullpath(filename);
//         std::string dirname;
//         if (basepath == NULL)
//         {
//             auto i = fullpath.find_last_of("/");
//             if (i != std::string::npos)
//             {
//                 dirname = fullpath.substr(0, i + 1);
//                 basepath = dirname.c_str();
//             }
//         }

//         std::string warn;
//         std::string err;
//         bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

//         if (!err.empty())
//             fprintf(stderr, "\n%s\n", err.c_str());

//         // if (!ret)
//         //     throw std::runtime_error("Erro ao carregar modelo.");

//         for (size_t shape = 0; shape < shapes.size(); ++shape)
//         {
//             if (shapes[shape].name.empty())
//             {
//                 fprintf(stderr,
//                         "*********************************************\n"
//                         "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
//                         "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
//                         "*********************************************\n",
//                         filename);
//                 // throw std::runtime_error("Objeto sem nome.");
//             }
//             printf("- Objeto '%s'\n", shapes[shape].name.c_str());
//         }

//         printf("OK.\n");
//     }

void SceneObject::update()
{

}

Pool<SceneObject, 1024>     GameContext::m_sceneObjects;
Pool<ObjModel, 64>          GameContext::m_models;
List<SceneObject *, 512>    GameContext::m_asteroids;

ObjModel *                  GameContext::m_sphereModel = nullptr;
ObjModel *                  GameContext::m_dogModel = nullptr;
ObjModel *                  GameContext::m_shipModel = nullptr;
ObjModel *                  GameContext::m_vidroModel = nullptr;
ObjModel *                  GameContext::m_asteroidModel = nullptr;

Texture *                   GameContext::m_mapTexture = nullptr;
Texture *                   GameContext::m_starsTexture = nullptr;
Texture *                   GameContext::m_dogTexture = nullptr;
Texture *                   GameContext::m_metalTexture = nullptr;
Texture *                   GameContext::m_eyesTexture = nullptr;
Texture *                   GameContext::m_asteroidTexture = nullptr;
Texture *                   GameContext::m_glassTexture = nullptr;
Texture *                   GameContext::m_sunTexture = nullptr;

Shader *                    GameContext::m_doItAllShader;

SceneObject *               GameContext::m_sunObject = nullptr;
SceneObject *               GameContext::m_earthObject = nullptr;

Player                      GameContext::m_player;


// // Velocidade do personagem e flags de movimento
// const float MAX_SPEED = 1.3f;
// float speed_X = 0.0f;
// float speed_Z = 0.0f;
// float speed = 0.01f;
// float speedFreio = 0.001f;

// bool walk_up = false;
// bool walk_down = false;
// bool walk_left = false;
// bool walk_right = false;

// float deltaTime = 0.0f; // Variação de tempo entre quadros (ERA DOUBLE)
// float lastFrame = 0.0f;  // Tempo do último quadro
// float deltaSpeed = 0.0f; // Velocidade multiplicada pela variação de tempo

// // "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// // pressionado no momento atual. Veja função MouseButtonCallback().
// bool g_LeftMouseButtonPressed = false;
// bool g_RightMouseButtonPressed = false;  // Análogo para botão direito do mouse
// bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// // Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// // usuário através do mouse (veja função CursorPosCallback()). A posição
// // efetiva da câmera é calculada dentro da função main(), dentro do loop de
// // renderização.
// float g_CameraTheta = 0.0f;    // Ângulo no plano ZX em relação ao eixo Z
// float g_CameraPhi = 0.0f;      // Ângulo em relação ao eixo Y
// float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// // Para a câmera em primeira pessoa
// int tipoCamera = 0; // 0 = Look At, 1 = Primeira Pessoa
// glm::vec4 camera_w_vector;
// glm::vec4 camera_u_vector;
// glm::vec4 camera_position_c; // Ponto "c", centro da câmera
// glm::vec4 camera_view_vector;
// glm::vec4 camera_up_vector;


void GameContext::init()
{
    m_sphereModel = loadModel("../../data/bola.obj");
    m_dogModel = loadModel("../../data/dog.obj");
    m_shipModel = loadModel("../../data/ship.obj");
    m_vidroModel = loadModel("../../data/vidro.obj");
    m_asteroidModel = loadModel("../../data/asteroid.obj");

    Texture::Desc textureDesc = {
        .m_type = GL_TEXTURE_2D,
        .m_internalFormat = DataFormat::RGBA8,
        .m_minFilter = GL_LINEAR_MIPMAP_LINEAR,
        .m_magFilter = GL_LINEAR,
        .m_wrapS = GL_CLAMP_TO_EDGE,
        .m_wrapT = GL_CLAMP_TO_EDGE,
    };

    m_mapTexture = GfxContext::loadTexture("../../data/map.jpg", &textureDesc);
    m_starsTexture = GfxContext::loadTexture("../../data/stars.jpeg", &textureDesc);
    m_dogTexture = GfxContext::loadTexture("../../data/dog.png", &textureDesc);
    m_metalTexture = GfxContext::loadTexture("../../data/metal.jpg", &textureDesc);
    m_eyesTexture = GfxContext::loadTexture("../../data/eyes.png", &textureDesc);
    m_asteroidTexture = GfxContext::loadTexture("../../data/asteroid.jpg", &textureDesc);
    m_glassTexture = GfxContext::loadTexture("../../data/glass.jpg", &textureDesc);
    m_sunTexture = GfxContext::loadTexture("../../data/sun.png", &textureDesc);

    Shader::VertexAttrib vertexAttribs[] = {
        {.m_name = "model_coefficients",    .m_type = DataFormat::RGBA32F,  .m_offset = offsetof(ObjModel::Vertex, m_position),  .m_normalized = false, .m_divisor = VertexDivisor::Vertex},
        {.m_name = "normal_coefficients",   .m_type = DataFormat::RGBA32F,  .m_offset = offsetof(ObjModel::Vertex, m_normal),    .m_normalized = false, .m_divisor = VertexDivisor::Vertex},
        {.m_name = "texture_coefficients",  .m_type = DataFormat::RG32F,    .m_offset = offsetof(ObjModel::Vertex, m_texCoords), .m_normalized = false, .m_divisor = VertexDivisor::Vertex},
    };

    Shader::VertexLayout vertexLayout = {
        .m_attribs = vertexAttribs,
        .m_attribCount = std::size(vertexAttribs),
        .m_stride = sizeof(ObjModel::Vertex)
    };

    Shader::Uniform uniforms[] = {
        {.m_name = "model",                 .m_type = UniformType::Mat4},
        {.m_name = "view",                  .m_type = UniformType::Mat4},
        {.m_name = "projection",            .m_type = UniformType::Mat4},
        {.m_name = "object_id",             .m_type = UniformType::Int},
        {.m_name = "bbox_min",              .m_type = UniformType::Vec4},
        {.m_name = "bbox_max",              .m_type = UniformType::Vec4},
        {.m_name = "light_position",        .m_type = UniformType::Vec4},

        {.m_name = "TextureImage0",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage1",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage2",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage3",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage4",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage5",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage6",        .m_type = UniformType::Tex},
        {.m_name = "TextureImage7",        .m_type = UniformType::Tex},
    };

    Shader::Desc shaderDesc = {
        .m_vertexSource = "../../src/shader_vertex.glsl",
        .m_fragmentSource = "../../src/shader_fragment.glsl",
        .m_vertexLayout = vertexLayout,
        .m_uniforms = uniforms,
        .m_uniformCount = std::size(uniforms)
    };
    
    m_doItAllShader = GfxContext::loadShader(&shaderDesc);
    // m_starsTexture
    // m_dogTexture
    // m_metalTexture
    // m_eyesTexture
    // m_asteroidTexture
    // m_glassTexture
    // m_sunTexture


    glm::mat4 transform;
    m_sunObject = createSceneObject(transform, m_sphereModel);
    m_earthObject = createSceneObject(transform, m_sphereModel);

    m_player.m_dogObject = createSceneObject(transform, m_dogModel);
    m_player.m_shipObject = createSceneObject(transform, m_shipModel);
    m_player.m_glassObject = createSceneObject(transform, m_vidroModel);
}

void GameContext::shutdown()
{

}

ObjModel *GameContext::loadModel(const char *filename, const char *basepath, bool triangulate)
{
    ObjModel *model = nullptr;
    printf("Carregando objetos do arquivo \"%s\"...\n", filename);

    // Se basepath == NULL, então setamos basepath como o dirname do
    // filename, para que os arquivos MTL sejam corretamente carregados caso
    // estejam no mesmo diretório dos arquivos OBJ.
    std::string fullpath(filename);
    std::string dirname;
    if (basepath == NULL)
    {
        auto i = fullpath.find_last_of("/");
        if (i != std::string::npos)
        {
            dirname = fullpath.substr(0, i + 1);
            basepath = dirname.c_str();
        }
    }

    std::string warn;
    std::string err;

    tinyobj::attrib_t                   attrib;
    std::vector<tinyobj::shape_t>       shapes;
    std::vector<tinyobj::material_t>    materials;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

    if (!err.empty())
    {
        fprintf(stderr, "\n%s\n", err.c_str());
    }

    if(ret)
    {
        // for (size_t shapeIndex = 0; shapeIndex < shapes.size(); ++shapeIndex)
        // {
        //     if (shapes[shapeIndex].name.empty())
        //     {
        //         fprintf(stderr,
        //             "*********************************************\n"
        //             "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
        //             "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
        //             "*********************************************\n",
        //             filename);

        //         // return nullptr;
        //         continue;
        //     }
            
        //     printf("- Objeto '%s'\n", shapes[shapeIndex].name.c_str());
        // }
        
        model = m_models.addElem(nullptr);
        model->m_batchCount = shapes.size();
        model->m_batches = (ObjModel::Batch *)calloc(model->m_batchCount, sizeof(ObjModel::Batch));
        model->m_drawMode = GL_TRIANGLES;              // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        // model->m_attrib = attrib;
        // model->m_shapes = shapes;
        // model->m_materials = materials;

        size_t num_vertices = attrib.vertices.size() / 3;
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            size_t num_triangles = shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                // assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

                glm::vec4 vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = shapes[shape].mesh.indices[3 * triangle + vertex];
                    const float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    const float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    const float vz = attrib.vertices[3 * idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx, vy, vz, 1.0);
                }

                const glm::vec4 a = vertices[0];
                const glm::vec4 b = vertices[1];
                const glm::vec4 c = vertices[2];

                const glm::vec4 n = crossproduct(b - a, c - a);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = shapes[shape].mesh.indices[3 * triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                    shapes[shape].mesh.indices[3 * triangle + vertex].normal_index = idx.vertex_index;
                }
            }
        }

        attrib.normals.resize(3 * num_vertices);

        for (size_t i = 0; i < vertex_normals.size(); ++i)
        {
            glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
            n /= norm(n);
            attrib.normals[3 * i + 0] = n.x;
            attrib.normals[3 * i + 1] = n.y;
            attrib.normals[3 * i + 2] = n.z;
        }

        // GLuint vertex_array_object_id;
        glGenVertexArrays(1, &model->m_vao);
        glBindVertexArray(model->m_vao);

        std::vector<GLuint> indices;
        std::vector<float> model_coefficients;
        std::vector<float> normal_coefficients;
        std::vector<float> texture_coefficients;

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            ObjModel::Batch *batch = model->m_batches + shape;
            size_t first_index = indices.size();
            size_t num_triangles = shapes[shape].mesh.num_face_vertices.size();

            const float minval = std::numeric_limits<float>::min();
            const float maxval = std::numeric_limits<float>::max();

            glm::vec3 bbox_min = glm::vec3(maxval, maxval, maxval);
            glm::vec3 bbox_max = glm::vec3(minval, minval, minval);

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                // assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = shapes[shape].mesh.indices[3 * triangle + vertex];

                    indices.push_back(first_index + 3 * triangle + vertex);

                    const float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    const float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    const float vz = attrib.vertices[3 * idx.vertex_index + 2];
                    // printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                    model_coefficients.push_back(vx);   // X
                    model_coefficients.push_back(vy);   // Y
                    model_coefficients.push_back(vz);   // Z
                    model_coefficients.push_back(1.0f); // W

                    bbox_min.x = std::min(bbox_min.x, vx);
                    bbox_min.y = std::min(bbox_min.y, vy);
                    bbox_min.z = std::min(bbox_min.z, vz);
                    bbox_max.x = std::max(bbox_max.x, vx);
                    bbox_max.y = std::max(bbox_max.y, vy);
                    bbox_max.z = std::max(bbox_max.z, vz);

                    // Inspecionando o código da tinyobjloader, o aluno Bernardo
                    // Sulzbach (2017/1) apontou que a maneira correta de testar se
                    // existem normais e coordenadas de textura no ObjModel é
                    // comparando se o índice retornado é -1. Fazemos isso abaixo.

                    if (idx.normal_index != -1)
                    {
                        const float nx = attrib.normals[3 * idx.normal_index + 0];
                        const float ny = attrib.normals[3 * idx.normal_index + 1];
                        const float nz = attrib.normals[3 * idx.normal_index + 2];
                        normal_coefficients.push_back(nx);   // X
                        normal_coefficients.push_back(ny);   // Y
                        normal_coefficients.push_back(nz);   // Z
                        normal_coefficients.push_back(0.0f); // W
                    }

                    if (idx.texcoord_index != -1)
                    {
                        const float u = attrib.texcoords[2 * idx.texcoord_index + 0];
                        const float v = attrib.texcoords[2 * idx.texcoord_index + 1];
                        texture_coefficients.push_back(u);
                        texture_coefficients.push_back(v);
                    }
                }
            }

            size_t last_index = indices.size() - 1;

            // SceneObject theobject;
            // theobject.name = model->shapes[shape].name;
            batch->m_start = first_index;                  // Primeiro índice
            batch->m_count = last_index - first_index + 1; // Número de indices
            model->m_min = glm::min(model->m_min, bbox_min);
            model->m_max = glm::max(model->m_max, bbox_max);
        }

        GLuint VBO_model_coefficients_id;
        glGenBuffers(1, &VBO_model_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
        GLuint location = 0;            // "(location = 0)" em "shader_vertex.glsl"
        GLint number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (!normal_coefficients.empty())
        {
            GLuint VBO_normal_coefficients_id;
            glGenBuffers(1, &VBO_normal_coefficients_id);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
            glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
            location = 1;             // "(location = 1)" em "shader_vertex.glsl"
            number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        if (!texture_coefficients.empty())
        {
            GLuint VBO_texture_coefficients_id;
            glGenBuffers(1, &VBO_texture_coefficients_id);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
            glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
            location = 2;             // "(location = 1)" em "shader_vertex.glsl"
            number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        GLuint indices_id;
        glGenBuffers(1, &indices_id);

        // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
        //

        // "Desligamos" o VAO, evitando assim que operações posteriores venham a
        // alterar o mesmo. Isso evita bugs.
        glBindVertexArray(0);

        printf("OK.\n");    
    }

    return model;
}
    
void GameContext::destroyModel(ObjModel *model)
{

}

SceneObject *GameContext::createSceneObject(glm::mat4& transform, ObjModel *model)
{
    SceneObject *object = m_sceneObjects.addElem(nullptr);
    object->m_model = model;
    object->m_transform = transform;
    return object;
}

void GameContext::destroySceneObject(SceneObject *object)
{

}

void updatePlayer(float deltaTime)
{

}

void updateAsteroids(float deltaTime)
{

}

void update(float deltaTime)
{
    // updateSpeed();
}

void draw()
{
    // glm::mat4 modelSphere = Matrix_Identity(); // Transformação identidade de modelagem
    // modelSphere = Matrix_Rotate_X(0.5);
    // glm::mat4 modelSphereInverse = Matrix_Identity(); // Transformação identidade de modelagem

    // modelSphere = Matrix_Rotate_X(-speed_X / 500) * Matrix_Rotate_Z(-speed_Z / 500) * modelSphere;
        
    // modelSphereInverse = Matrix_Rotate_X(speed_X / 500) * Matrix_Rotate_Z(speed_Z / 500) * modelSphereInverse;
    // glm::mat4 model = Matrix_Translate(0.0f, -31.0f, 0.0f) * Matrix_Scale(30.0f, 30.0f, 30.0f) * modelSphere;
    // // glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    // // glUniform1i(g_object_id_uniform, BOLA);
    // GameContext::m_doItAllShader->uniformMat4v(0, glm::value_ptr(model));
    // GameContext::m_doItAllShader->uniform1i(3, BOLA);
    
    // DrawVirtualObject(GameContext::m_earthObject);
}


// Função para atualizar a velocidade do personagem baseado nas teclas pressionadas.
// Chamada a cada atualização de quadro e possui um teste para evitar que a velocidade ultrapasse um limite.
// void updateSpeed(){
//     if (walk_left)
//         if (speed_Z > -MAX_SPEED)
//             speed_Z -= speed;
//     if (!walk_left)
//         if (speed_Z < 0){
//             speed_Z += speedFreio;
//             if (fabs(speed_Z) < SPEEDTHRESHOLD)
//                 speed_Z = 0;
//         }
            
//     if (walk_right)
//         if (speed_Z < MAX_SPEED)
//             speed_Z += speed;
//     if (!walk_right)
//         if (speed_Z > 0){
//             speed_Z -= speedFreio;
//             if (fabs(speed_Z) < SPEEDTHRESHOLD)
//                 speed_Z = 0;
//         }
            
//     if (walk_up)
//         if (speed_X < MAX_SPEED)
//             speed_X += speed;
//     if (!walk_up)
//         if (speed_X > 0){
//             speed_X -= speedFreio;
//             if (fabs(speed_X) < SPEEDTHRESHOLD)
//                 speed_X = 0;
//         }

//     if (walk_down)
//         if (speed_X > -MAX_SPEED)
//             speed_X -= speed;
//     if (!walk_down)
//         if (speed_X < 0){
//             speed_X += speedFreio;
//             if (fabs(speed_X) < SPEEDTHRESHOLD)
//                 speed_X = 0;
//         }
            
// }