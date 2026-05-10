#ifndef GFX_H
#define GFX_H

#include "glew/GL/glew.h"
#include <stdint.h>
#include "pool.h"

enum class DataFormat : uint16_t
{
    R8,
    RG32F,
    RGB8,
    RGB8I,
    RGB8UI,
    RGB32F,
    RGBA8,
    BGRA8,
    RGBA8I,
    RGBA8UI,
    RGBA16F,
    RGBA32I,
    RGBA32UI,
    RGBA32F,
    Depth32F,
    Depth24Stencil8,
    Last
};

enum struct UniformType : uint16_t
{
    Bool,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Int,
    IVec2,
    IVec3,
    Ivec4,
    Uint,
    UVec2,
    UVec3,
    UVec4,
    Mat2,
    Mat3,
    Mat4,
    Tex,
    UBO,
    SBO
};

struct FormatInfo
{
    uint16_t m_internalFormat;
    uint16_t m_dataFormat;
    uint16_t m_dataType;
    uint16_t m_size;
};

enum struct VertexDivisor : uint8_t
{
    Vertex,
    Instance
};

struct Shader : public PoolElem
{
    struct VertexAttrib
    {
        union
        {
            uint32_t    m_location;
            const char* m_name;
        };

        DataFormat      m_type;
        uint16_t        m_offset;
        uint8_t         m_normalized;
        VertexDivisor   m_divisor;
    };

    struct VertexLayout
    {
        VertexAttrib*   m_attribs;
        uint16_t        m_attribCount;
        uint16_t        m_stride;
    };

    struct Uniform
    {
        union
        {
            const char *    m_name;
            uint32_t        m_location;
        };

        UniformType         m_type;
        uint16_t            m_binding;
    };

    struct Desc
    {
        const char *    m_vertexSource;
        const char *    m_fragmentSource;
        const char *    m_geometrySource;

        VertexLayout    m_vertexLayout;
        Uniform *       m_uniforms;
        uint32_t        m_uniformCount;
    };

    Shader();
    ~Shader();

    void create(Desc *desc);
    void destroy();

    void bindVertexLayout();
    void uniformBool(uint32_t uniformIndex, bool v);
    void uniform1i(uint32_t uniformIndex, int32_t v);
    void uniform1f(uint32_t uniformIndex, float v);
    void uniform1fv(uint32_t uniformIndex, float *v);
    void uniform2fv(uint32_t uniformIndex, float *v);
    void uniform3fv(uint32_t uniformIndex, float *v);
    void uniform4fv(uint32_t uniformIndex, float *v);
    void uniformMat4v(uint32_t uniformIndex, float *v);

    GLuint          m_handle;
    Uniform *       m_uniforms;
    uint32_t        m_uniformCount;
    VertexLayout    m_vertexLayout;
};


struct Texture : public PoolElem
{
    struct Desc
    {
        uint16_t    m_type;
        uint16_t    m_width;
        uint16_t    m_height;
        uint16_t    m_depth;
        DataFormat  m_internalFormat;
        uint16_t    m_minFilter;
        uint16_t    m_magFilter;
        uint16_t    m_wrapS;
        uint16_t    m_wrapT;
        uint8_t     m_baseLevel;
        uint8_t     m_maxLevel;
        uint8_t     m_samples;
    };

    Texture();
    ~Texture();

    static void bind(Texture *texture);

    void create(Desc *desc);
    void destroy();
    void resize(uint32_t width, uint32_t height);
    void fill(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height, void *data);

    Desc        m_desc;
    GLuint      m_handle;
};

struct GfxContext
{
    Pool<Texture, 64>                                       m_textures;
    Pool<Shader, 64>                                        m_shaders;
    // Pool<Framebuffer, 64>                                   m_framebuffers;
    //Pool<Model, 64>                                         m_models;

    /* TODO: move those two to SceneView */
    //Pool<View, 8, offsetof(View, m_elem)>                   m_views;
    //List<View*, 8>                                          m_activeViews; 

    // List<ImmediateDataSlot, 0x8000>                         m_immediateData;

    // uint32_t                                                m_immediateVertexBufferCursor;
    // Buffer *                                                m_immediateVertexBuffer;

    // uint32_t                                                m_immediateIndexBufferCursor;
    // Buffer *                                                m_immediateIndexBuffer;

    // Heap                                                    m_heaps[(uint32_t)BufferType::Last];
    // GLuint                                                  m_boundBuffers[(uint32_t)BufferType::Last];

    // Framebuffer *                                           m_currentFramebuffer;
    Shader *                                                m_currentShader;

    // static SDL_Window *                                     m_window;
    // static SDL_GLContext                                    m_glContext;
    // static Framebuffer                                      m_windowFramebuffer;
    //View                                                    m_defaultView;
    static GLuint                                           m_vao;


    static bool init();
    static void shutdown();

    static Texture *loadTexture(const char *path, Texture::Desc *desc);
    static Texture *createTexture(Texture::Desc *desc); 
    static void destroyTexture(Texture *texture);
    static void bindTexture(Texture *texture);

    static Shader *loadShader(Shader::Desc *desc);
    static Shader *createShader(Shader::Desc *desc);
    static void destroyShader(Shader *shader);
    static void bindShader(Shader *shader);

    // static Framebuffer *createFramebuffer(Framebuffer::Desc *desc);
    // static void destroyFramebuffer(Framebuffer *framebuffer);
    // static void bindFramebuffer(Framebuffer *framebuffer, FramebufferTarget target);
    // static Framebuffer *getCurrentFramebuffer();

    // static Buffer *allocBuffer(BufferType type, size_t size, size_t align, void *data = nullptr);
    // static void freeBuffer(Buffer *buffer);

    // static ImmediateMesh allocImmediateMesh(uint32_t vertexSize, uint32_t vertexCount, uint32_t indexCount, bool allocData);
    // static void updateImmediateMesh(ImmediateMesh *mesh);
    // static void resetImmediateMeshes();

    /*static Model *createModel(Model::Desc *desc);
    static void destroyModel(Model *model);*/

    /*static View *createView(View::Desc *desc);
    static void destroyView(View *view);*/

    static void *allocImmediateData(size_t size);

    static void maximizeWindow();
    static void windowResizeEvent(uint32_t width, uint32_t height);

    static void beginFrame();
    static void draw();
    static void endFrame();
    static void swapBuffers();

    GfxContext();
    ~GfxContext();
};

#endif