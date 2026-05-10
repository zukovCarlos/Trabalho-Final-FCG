#include "gfx.h"
#include "stb_image.h"
#include <stdio.h>

GfxContext g_gfxContext;

FormatInfo g_formatInfo[] = {
    { /* .m_internalFormat = */ GL_R8,                     /* .m_dataFormat = */ GL_R,                   /* .m_dataType = */ GL_UNSIGNED_BYTE,                 /* .m_size = */ 1},
    { /* .m_internalFormat = */ GL_RG32F,                  /* .m_dataFormat = */ GL_RG,                  /* .m_dataType = */ GL_FLOAT,                         /* .m_size = */ 2},
    { /* .m_internalFormat = */ GL_RGB8,                   /* .m_dataFormat = */ GL_RGB,                 /* .m_dataType = */ GL_UNSIGNED_BYTE,                 /* .m_size = */ 3},
    { /* .m_internalFormat = */ GL_RGB8I,                  /* .m_dataFormat = */ GL_RGB,                 /* .m_dataType = */ GL_BYTE,                          /* .m_size = */ 3},
    { /* .m_internalFormat = */ GL_RGB8UI,                 /* .m_dataFormat = */ GL_RGB_INTEGER,         /* .m_dataType = */ GL_UNSIGNED_BYTE,                 /* .m_size = */ 3},
    { /* .m_internalFormat = */ GL_RGB32F,                 /* .m_dataFormat = */ GL_RGB,                 /* .m_dataType = */ GL_FLOAT,                         /* .m_size = */ 3},
    { /* .m_internalFormat = */ GL_RGBA8,                  /* .m_dataFormat = */ GL_RGBA,                /* .m_dataType = */ GL_UNSIGNED_BYTE,                 /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA8,                  /* .m_dataFormat = */ GL_BGRA,                /* .m_dataType = */ GL_UNSIGNED_INT_8_8_8_8_REV,      /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA8I,                 /* .m_dataFormat = */ GL_RGBA,                /* .m_dataType = */ GL_BYTE,                          /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA8UI,                /* .m_dataFormat = */ GL_RGBA_INTEGER,        /* .m_dataType = */ GL_UNSIGNED_BYTE,                 /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA16F,                /* .m_dataFormat = */ GL_RGBA,                /* .m_dataType = */ GL_HALF_FLOAT,                    /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA32I,                /* .m_dataFormat = */ GL_RGBA_INTEGER,        /* .m_dataType = */ GL_INT,                           /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA32UI,               /* .m_dataFormat = */ GL_RGBA_INTEGER,        /* .m_dataType = */ GL_UNSIGNED_INT,                  /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_RGBA32F,                /* .m_dataFormat = */ GL_RGBA,                /* .m_dataType = */ GL_FLOAT,                         /* .m_size = */ 4},
    { /* .m_internalFormat = */ GL_DEPTH_COMPONENT32F,     /* .m_dataFormat = */ GL_DEPTH_COMPONENT,     /* .m_dataType = */ GL_FLOAT,                         /* .m_size = */ 1},
    { /* .m_internalFormat = */ GL_DEPTH24_STENCIL8,       /* .m_dataFormat = */ GL_DEPTH_STENCIL,       /* .m_dataType = */ GL_UNSIGNED_INT_24_8,             /* .m_size = */ 2},
};

Shader::Shader()
{
    m_handle = 0;
    m_uniforms = nullptr;
    m_uniformCount = 0;
}

Shader::~Shader()
{
    if (m_handle != 0)
    {
        glDeleteProgram(m_handle);
        m_handle = 0;

        if (m_uniformCount > 0)
        {
            free(m_uniforms);
            m_uniforms = nullptr;
        }

        m_uniformCount = 0;
    }
}

//void Shader::bind(Shader *shader)
//{
//    if (shader != nullptr)
//    {
//        glUseProgram(shader->m_handle);
//    }
//}

void Shader::create(Desc *desc)
{
    m_handle = 0;
    m_uniforms = nullptr;
    m_uniformCount = 0;

    if (desc->m_vertexSource != nullptr && desc->m_fragmentSource != nullptr)
    {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &desc->m_vertexSource, nullptr);
        glCompileShader(vertexShader);

        GLint status;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

        if (status)
        {
            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &desc->m_fragmentSource, nullptr);
            glCompileShader(fragmentShader);

            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

            if (status)
            {
                m_handle = glCreateProgram();
                glAttachShader(m_handle, vertexShader);
                glAttachShader(m_handle, fragmentShader);
                GLuint geometryShader = 0;

                if (desc->m_geometrySource != nullptr)
                {
                    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
                    glShaderSource(geometryShader, 1, &desc->m_geometrySource, nullptr);
                    glCompileShader(geometryShader);

                    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &status);

                    if (status)
                    {
                        glAttachShader(m_handle, geometryShader);
                    }
                    else
                    {
                        GLint infoLogLength = 0;
                        glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                        char *infoLog = (char *)calloc(1, infoLogLength);
                        glGetShaderInfoLog(geometryShader, infoLogLength, nullptr, infoLog);
                        printf("Geometry shader compilation failed!\nInfo log: %s\n", infoLog);
                        free(infoLog);
                    }
                }

                if (status)
                {
                    glLinkProgram(m_handle);
                    glGetProgramiv(m_handle, GL_LINK_STATUS, &status);

                    if (status)
                    {
                        if (desc->m_uniformCount > 0)
                        {
                            m_uniforms = (Uniform *)calloc(desc->m_uniformCount, sizeof(Uniform));
                            m_uniformCount = desc->m_uniformCount;

                            for (uint32_t uniformIndex = 0; uniformIndex < desc->m_uniformCount; uniformIndex++)
                            {
                                m_uniforms[uniformIndex].m_type = desc->m_uniforms[uniformIndex].m_type;

                                if (m_uniforms[uniformIndex].m_type == UniformType::UBO)
                                {
                                    GLuint locationIndex = glGetProgramResourceLocationIndex(m_handle, GL_UNIFORM_BLOCK, desc->m_uniforms[uniformIndex].m_name);

                                    if (locationIndex != GL_INVALID_INDEX)
                                    {
                                        glShaderStorageBlockBinding(m_handle, locationIndex, desc->m_uniforms[uniformIndex].m_binding);
                                    }
                                }
                                else if (m_uniforms[uniformIndex].m_type == UniformType::SBO)
                                {
                                    GLuint locationIndex = glGetProgramResourceLocation(m_handle, GL_SHADER_STORAGE_BUFFER, desc->m_uniforms[uniformIndex].m_name);

                                    if (locationIndex != GL_INVALID_INDEX)
                                    {
                                        glUniformBlockBinding(m_handle, locationIndex, desc->m_uniforms[uniformIndex].m_binding);
                                    }
                                }
                                else
                                {
                                    m_uniforms[uniformIndex].m_location = glGetUniformLocation(m_handle, desc->m_uniforms[uniformIndex].m_name);
                                }
                            }
                        }

                        if (desc->m_vertexLayout.m_attribCount > 0)
                        {
                            m_vertexLayout.m_attribs = (VertexAttrib *)calloc(desc->m_vertexLayout.m_attribCount, sizeof(VertexAttrib));
                            m_vertexLayout.m_attribCount = desc->m_vertexLayout.m_attribCount;
                            m_vertexLayout.m_stride = desc->m_vertexLayout.m_stride;
                            memcpy((void *)m_vertexLayout.m_attribs, (void *)desc->m_vertexLayout.m_attribs, sizeof(VertexAttrib) * desc->m_vertexLayout.m_attribCount);

                            for (uint32_t attribIndex = 0; attribIndex < desc->m_vertexLayout.m_attribCount; attribIndex++)
                            {
                                VertexAttrib* attrib = desc->m_vertexLayout.m_attribs + attribIndex;
                                m_vertexLayout.m_attribs[attribIndex].m_location = glGetAttribLocation(m_handle, attrib->m_name);
                            }
                        }
                    }
                    else
                    {
                        GLint infoLogLength = 0;
                        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &infoLogLength);
                        char *infoLog = (char *)calloc(1, infoLogLength);
                        glGetProgramInfoLog(m_handle, infoLogLength, nullptr, infoLog);
                        printf("Shader linking failed!\nInfo log: %s\n", infoLog);
                        free(infoLog);

                        glDeleteProgram(m_handle);
                    }
                }

            }
            else
            {
                GLint infoLogLength = 0;
                glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                char *infoLog = (char *)calloc(1, infoLogLength);
                glGetShaderInfoLog(fragmentShader, infoLogLength, nullptr, infoLog);
                printf("Fragment shader compilation failed!\nInfo log: %s\n", infoLog);
                free(infoLog);
            }

            glDeleteShader(fragmentShader);
        }
        else
        {
            GLint infoLogLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
            char *infoLog = (char *)calloc(1, infoLogLength);
            glGetShaderInfoLog(vertexShader, infoLogLength, nullptr, infoLog);
            //std::cout << "Vertex shader compilation failed!" << std::endl << "Info log: " << infoLog << std::endl;
            printf("Vertex shader compilation failed!\nInfo log: %s\n", infoLog);
            free(infoLog);
        }

        glDeleteShader(vertexShader);
    }
}

void Shader::destroy()
{

}

void Shader::bindVertexLayout()
{
    for (uint32_t attribIndex = 0; attribIndex < m_vertexLayout.m_attribCount; attribIndex++)
    {
        VertexAttrib *attrib = m_vertexLayout.m_attribs + attribIndex;
        FormatInfo *info = g_formatInfo + (uint32_t)attrib->m_type;
        if (attrib->m_location != GL_INVALID_INDEX)
        {
            glEnableVertexAttribArray(attrib->m_location);
            glVertexAttribPointer(attrib->m_location, info->m_size, info->m_dataType, attrib->m_normalized, m_vertexLayout.m_stride, (const void *)attrib->m_offset);
            glVertexAttribDivisor(attrib->m_location, (uint32_t)attrib->m_divisor);
        }
    }
}

void Shader::uniformBool(uint32_t uniformIndex, bool v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniform1i(m_uniforms[uniformIndex].m_location, (GLint)v);
    }
}

void Shader::uniform1i(uint32_t uniformIndex, int32_t v) 
{
    if(uniformIndex < m_uniformCount)
    {
        glUniform1i(m_uniforms[uniformIndex].m_location, v);
    }
}

void Shader::uniform1f(uint32_t uniformIndex, float v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniform1f(m_uniforms[uniformIndex].m_location, v);
    }
}

void Shader::uniform1fv(uint32_t uniformIndex, float *v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniform1fv(m_uniforms[uniformIndex].m_location, 1, v);
    }
}

void Shader::uniform2fv(uint32_t uniformIndex, float* v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniform2fv(m_uniforms[uniformIndex].m_location, 1, v);
    }
}

void Shader::uniform3fv(uint32_t uniformIndex, float* v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniform3fv(m_uniforms[uniformIndex].m_location, 1, v);
    }
}

void Shader::uniform4fv(uint32_t uniformIndex, float* v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniform4fv(m_uniforms[uniformIndex].m_location, 1, v);
    }
}

void Shader::uniformMat4v(uint32_t uniformIndex, float *v)
{
    if (uniformIndex < m_uniformCount)
    {
        glUniformMatrix4fv(m_uniforms[uniformIndex].m_location, 1, GL_FALSE, v);
    }
}


Texture::Texture()
{
    m_handle = 0;
    m_desc = {};
}

Texture::~Texture()
{
    if (m_handle != 0)
    {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }
}

void Texture::bind(Texture *texture)
{
    glBindTexture(texture->m_desc.m_type, texture->m_handle);
}

void Texture::create(Desc* desc)
{
    m_desc = *desc;

    glGenTextures(1, &m_handle);
    glBindTexture(m_desc.m_type, m_handle);

    if (m_desc.m_minFilter == 0)
    {
        m_desc.m_minFilter = GL_LINEAR;
    }

    if (m_desc.m_magFilter == 0)
    {
        m_desc.m_magFilter = GL_LINEAR;
    }

    if (m_desc.m_wrapS == 0)
    {
        m_desc.m_wrapS = GL_CLAMP_TO_BORDER;
    }

    if (m_desc.m_wrapT == 0)
    {
        m_desc.m_wrapT = GL_CLAMP_TO_BORDER;
    }

    if (m_desc.m_samples == 0)
    {
        m_desc.m_samples = 1;
    }

    glTexParameteri(m_desc.m_type, GL_TEXTURE_MIN_FILTER, m_desc.m_minFilter);
    glTexParameteri(m_desc.m_type, GL_TEXTURE_MAG_FILTER, m_desc.m_magFilter);
    glTexParameteri(m_desc.m_type, GL_TEXTURE_WRAP_S, m_desc.m_wrapS);
    glTexParameteri(m_desc.m_type, GL_TEXTURE_WRAP_T, m_desc.m_wrapT);
    glTexParameteri(m_desc.m_type, GL_TEXTURE_BASE_LEVEL, m_desc.m_baseLevel);
    glTexParameteri(m_desc.m_type, GL_TEXTURE_MAX_LEVEL, m_desc.m_maxLevel);
    resize(m_desc.m_width, m_desc.m_height);
}

void Texture::destroy()
{

}

void Texture::resize(uint32_t width, uint32_t height)
{
    m_desc.m_width = width;
    m_desc.m_height = height;
    FormatInfo *formatInfo = g_formatInfo + (uint32_t)m_desc.m_internalFormat;
    glBindTexture(m_desc.m_type, m_handle);

    if (m_desc.m_type == GL_TEXTURE_2D_MULTISAMPLE)
    {
        glTexImage2DMultisample(m_desc.m_type, m_desc.m_samples, formatInfo->m_internalFormat, m_desc.m_width, m_desc.m_height, true);
    }
    else
    {
        glTexImage2D(m_desc.m_type, 0, formatInfo->m_internalFormat, m_desc.m_width, m_desc.m_height, 0, formatInfo->m_dataFormat, formatInfo->m_dataType, nullptr);
    }

}

void Texture::fill(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height, void* data)
{
    FormatInfo *formatInfo = g_formatInfo + (uint32_t)m_desc.m_internalFormat;
    glBindTexture(m_desc.m_type, m_handle);
    glTexSubImage2D(m_desc.m_type, 0, offsetX, offsetY, width, height, formatInfo->m_dataFormat, formatInfo->m_dataType, data);

    if (m_desc.m_maxLevel > 0)
    {
        glGenerateMipmap(m_desc.m_type);
    }
}

GLuint          GfxContext::m_vao = 0;

GfxContext::GfxContext()
{
    m_vao = 0;
    // m_currentFramebuffer = nullptr;
    m_currentShader = nullptr;
    // m_glContext = nullptr;
    // m_window = nullptr;
    // m_boundBuffers[(uint32_t)BufferType::Vertex] = 0;
    // m_boundBuffers[(uint32_t)BufferType::Index] = 0;
    // m_boundBuffers[(uint32_t)BufferType::Uniform] = 0;
    // m_boundBuffers[(uint32_t)BufferType::Storage] = 0;
    // m_boundBuffers[(uint32_t)BufferType::Indirect] = 0;
}

GfxContext::~GfxContext()
{

}

bool GfxContext::init()
{
    // GfxContext::m_window = SDL_CreateWindow("SenCity", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // GfxContext::m_glContext = SDL_GL_CreateContext(GfxContext::m_window);

    GLenum status = glewInit();
    if (status != GLEW_NO_ERROR)
    {
        return false;
    }

    // SDL_GL_MakeCurrent(GfxContext::m_window, GfxContext::m_glContext);
    // SDL_GL_SetSwapInterval(1);

    glGenVertexArrays(1, &GfxContext::m_vao);
    glBindVertexArray(GfxContext::m_vao);

    // GfxContext::m_windowFramebuffer.m_handle = 0;
    // GfxContext::m_windowFramebuffer.m_index = POOL_INVALID_INDEX;
    // bindFramebuffer(&GfxContext::m_windowFramebuffer, FramebufferTarget::Draw);

    // g_gfxContext.m_heaps[(uint32_t)BufferType::Vertex].create(BufferType::Vertex, VERTEX_HEAP_SIZE);
    // g_gfxContext.m_heaps[(uint32_t)BufferType::Index].create(BufferType::Index, INDEX_HEAP_SIZE);
    // g_gfxContext.m_heaps[(uint32_t)BufferType::Indirect].create(BufferType::Indirect, INDIRECT_HEAP_SIZE * sizeof(DrawElementsIndirectCommand));

    // g_gfxContext.m_immediateVertexBuffer = GfxContext::allocBuffer(BufferType::Vertex, IMMEDIATE_VERTEX_BUFFER_SIZE, 16, nullptr);
    // g_gfxContext.m_immediateIndexBuffer = GfxContext::allocBuffer(BufferType::Index, IMMEDIATE_INDEX_BUFFER_SIZE, 16, nullptr);

    return true;
}

void GfxContext::shutdown()
{
    
}

Texture *GfxContext::loadTexture(const char *path, Texture::Desc *desc)
{
    Texture *texture = nullptr;

    FILE *textureFile = fopen(path, "rb");
    if (textureFile != nullptr)
    {
        stbi_set_flip_vertically_on_load(true);
        Texture::Desc textureDesc = *desc;
        int channels = 0;
        int width;
        int height;
        stbi_uc *pixels = stbi_load_from_file(textureFile, &width, &height, &channels, STBI_rgb_alpha);

        if (pixels != nullptr)
        {
            textureDesc.m_width = width;
            textureDesc.m_height = height;

            texture = GfxContext::createTexture(&textureDesc);
            texture->fill(0, 0, textureDesc.m_width, textureDesc.m_height, pixels);

            free(pixels);
        }

        fclose(textureFile);
    }

    return texture;
}

Texture *GfxContext::createTexture(Texture::Desc *desc) 
{
    Texture *texture = g_gfxContext.m_textures.addElem(nullptr);
    texture->create(desc);
    return texture;
}

void GfxContext::destroyTexture(Texture *texture)
{

}

void GfxContext::bindTexture(Texture *texture)
{
    if (texture != nullptr && texture->m_index != POOL_INVALID_INDEX)
    {
        glBindTexture(texture->m_desc.m_type, texture->m_handle);
    }
}

Shader *GfxContext::loadShader(Shader::Desc *desc)
{
    Shader *shader = nullptr;
    Shader::Desc shaderDesc = *desc;

    FILE *vertexShaderSourceFile = fopen(desc->m_vertexSource, "r");

    if (vertexShaderSourceFile != nullptr)
    {
        fseek(vertexShaderSourceFile, 0, SEEK_END);
        size_t fileSize = ftell(vertexShaderSourceFile);
        rewind(vertexShaderSourceFile);

        shaderDesc.m_vertexSource = (const char *)calloc(1, fileSize);
        if (shaderDesc.m_vertexSource != nullptr)
        {
            fread((void *)shaderDesc.m_vertexSource, 1, fileSize, vertexShaderSourceFile);

            FILE *fragmentShaderSourceFile = fopen(desc->m_fragmentSource, "r");

            if (fragmentShaderSourceFile != nullptr)
            {
                fseek(fragmentShaderSourceFile, 0, SEEK_END);
                size_t fileSize = ftell(fragmentShaderSourceFile);
                rewind(fragmentShaderSourceFile);
                shaderDesc.m_fragmentSource = (const char *)calloc(1, fileSize);

                if (shaderDesc.m_fragmentSource != nullptr)
                {
                    fread((void *)shaderDesc.m_fragmentSource, 1, fileSize, fragmentShaderSourceFile);
                }

                fclose(fragmentShaderSourceFile);

                if (desc->m_geometrySource != nullptr)
                {
                    FILE *geometryShaderSourceFile = fopen(desc->m_geometrySource, "r");

                    if (geometryShaderSourceFile != nullptr)
                    {
                        fseek(geometryShaderSourceFile, 0, SEEK_END);
                        size_t fileSize = ftell(geometryShaderSourceFile);
                        rewind(geometryShaderSourceFile);
                        shaderDesc.m_geometrySource = (const char *)calloc(1, fileSize);
                        if (shaderDesc.m_geometrySource != nullptr)
                        {
                            fread((void *)shaderDesc.m_geometrySource, 1, fileSize, geometryShaderSourceFile);
                        }

                        fclose(geometryShaderSourceFile);
                    }
                }

                shader = GfxContext::createShader(&shaderDesc);
                free((void *)shaderDesc.m_fragmentSource);

                if (shaderDesc.m_geometrySource != nullptr)
                {
                    free((void *)shaderDesc.m_geometrySource);
                }
            }
        }

        fclose(vertexShaderSourceFile);
        free((void *)shaderDesc.m_vertexSource);
    }

    return shader;
}

Shader *GfxContext::createShader(Shader::Desc *desc) 
{
    Shader *shader = g_gfxContext.m_shaders.addElem(nullptr);
    shader->create(desc);
    return shader;
}

void GfxContext::destroyShader(Shader *shader)
{

}

void GfxContext::bindShader(Shader *shader)
{
    if (shader != nullptr && shader->m_index != POOL_INVALID_INDEX)
    {
        glUseProgram(shader->m_handle);
        shader->bindVertexLayout();
    }
    else
    {
        glUseProgram(0);
        shader = nullptr;
    }

    g_gfxContext.m_currentShader = shader;
}

// Framebuffer *GfxContext::createFramebuffer(Framebuffer::Desc *desc)
// {
//     Framebuffer *framebuffer = g_gfxContext.m_framebuffers.addElem(nullptr);
//     framebuffer->create(desc);
//     return framebuffer;
// }

// void GfxContext::destroyFramebuffer(Framebuffer *framebuffer)
// {

// }

// void GfxContext::bindFramebuffer(Framebuffer *framebuffer, FramebufferTarget target)
// {
//     if (target < FramebufferTarget::Last)
//     {
//         if (framebuffer == nullptr || framebuffer->m_index == POOL_INVALID_INDEX)
//         {
//             framebuffer = &g_gfxContext.m_windowFramebuffer;
//         }

//         glBindFramebuffer(g_framebufferTargetInfo[(uint32_t)target].m_target, framebuffer->m_handle);
//         glViewport(0, 0, framebuffer->m_width, framebuffer->m_height);
//         g_gfxContext.m_currentFramebuffer = framebuffer;
//     }
// }

// Framebuffer *GfxContext::getCurrentFramebuffer()
// {
//     return g_gfxContext.m_currentFramebuffer;
// }

// Buffer *GfxContext::allocBuffer(BufferType type, size_t size, size_t align, void *data)
// {
//     Buffer *buffer = nullptr;

//     if ((uint32_t)type < (uint32_t)BufferType::Last)
//     {
//         buffer = g_gfxContext.m_heaps[(uint32_t)type].alloc(size, align);

//         if (data != nullptr)
//         {
//             buffer->update(0, size, data);
//         }
//     }

//     return buffer;
// }

// void GfxContext::freeBuffer(Buffer *buffer)
// {
//     if (buffer != nullptr && buffer->m_index != POOL_INVALID_INDEX)
//     {
//         buffer->m_heap->free(buffer);
//     }
// }

// ImmediateMesh GfxContext::allocImmediateMesh(uint32_t vertexSize, uint32_t vertexCount, uint32_t indexCount, bool allocData)
// {
//     ImmediateMesh mesh = {};

//     if (vertexCount > 0 && vertexSize > 0)
//     {
//         uint32_t vertexStart = (g_gfxContext.m_immediateVertexBufferCursor + (vertexSize - 1)) & ~(vertexSize - 1);
//         uint32_t totalVertexSize = vertexSize * vertexCount;

//         if (IMMEDIATE_VERTEX_BUFFER_SIZE - vertexStart < totalVertexSize)
//         {
//             g_gfxContext.m_immediateVertexBufferCursor = 0;
//             vertexStart = 0;
//         }

//         mesh.m_vertexOffset = (vertexStart + g_gfxContext.m_immediateVertexBuffer->m_chunk.m_start) / vertexSize;
//         mesh.m_vertexCount = vertexCount;
//         mesh.m_vertexSize = vertexSize;

//         if (allocData)
//         {
//             mesh.m_vertices = GfxContext::allocImmediateData(totalVertexSize);
//         }

//         if (indexCount > 0)
//         {
//             uint32_t indexStart = g_gfxContext.m_immediateIndexBufferCursor;
//             if (IMMEDIATE_INDEX_BUFFER_SIZE - g_gfxContext.m_immediateIndexBufferCursor < sizeof(uint32_t) * indexCount)
//             {
//                 g_gfxContext.m_immediateIndexBufferCursor = 0;
//                 indexStart = 0;
//             }

//             mesh.m_firstIndex = (indexStart + g_gfxContext.m_immediateIndexBuffer->m_chunk.m_start) / sizeof(uint32_t);
//             mesh.m_indexCount = indexCount;

//             if (allocData)
//             {
//                 mesh.m_indices = (uint32_t *)GfxContext::allocImmediateData(sizeof(uint32_t) * indexCount);
//             }
//         }
//     }

//     return mesh;
// }

// void GfxContext::updateImmediateMesh(ImmediateMesh *mesh)
// {
//     if (mesh->m_vertices != nullptr && mesh->m_vertexSize > 0 && mesh->m_vertexCount > 0)
//     {
//         uint32_t vertexOffset = mesh->m_vertexOffset - g_gfxContext.m_immediateVertexBuffer->m_chunk.m_start / mesh->m_vertexSize;
//         g_gfxContext.m_immediateVertexBuffer->update(vertexOffset * mesh->m_vertexSize, mesh->m_vertexSize * mesh->m_vertexCount, mesh->m_vertices);

//         if (mesh->m_indices != nullptr && mesh->m_indexCount > 0)
//         {
//             uint32_t indexOffset = mesh->m_firstIndex - g_gfxContext.m_immediateIndexBuffer->m_chunk.m_start / sizeof(uint32_t);
//             g_gfxContext.m_immediateIndexBuffer->update(indexOffset * sizeof(uint32_t), sizeof(uint32_t) * mesh->m_indexCount, mesh->m_indices);
//         }
//     }
// }

// void GfxContext::resetImmediateMeshes()
// {
//     g_gfxContext.m_immediateVertexBufferCursor = 0;
//     g_gfxContext.m_immediateIndexBufferCursor = 0;
// }


// void *GfxContext::allocImmediateData(size_t size)
// {
//     void *immediateData = nullptr;
//     size_t bufferSize = g_gfxContext.m_immediateData.bufferSize();
//     if (size < bufferSize * sizeof(ImmediateDataSlot))
//     {
//         size_t slotCount = size / sizeof(ImmediateDataSlot);
//         if (size % sizeof(ImmediateDataSlot))
//         {
//             slotCount++;
//         }

//         size_t bufferOffset = g_gfxContext.m_immediateData.m_cursor % sizeof(ImmediateDataSlot);
//         size_t availableSlots = bufferSize - bufferOffset;

//         if (availableSlots < slotCount)
//         {
//             g_gfxContext.m_immediateData.m_cursor += availableSlots;
//         }

//         immediateData = g_gfxContext.m_immediateData.getElem(g_gfxContext.m_immediateData.addElem(nullptr));
//         slotCount--;

//         g_gfxContext.m_immediateData.m_cursor += slotCount;
//     }

//     return immediateData;
// }

// void GfxContext::maximizeWindow()
// {
//     SDL_DisplayID displayId = SDL_GetDisplayForWindow(g_gfxContext.m_window);
//     SDL_Rect displaySize;
//     SDL_GetDisplayUsableBounds(displayId, &displaySize);
//     int topSize;
//     int bottomSize;
//     int leftSize;
//     int rightSize;
//     SDL_GetWindowBordersSize(g_gfxContext.m_window, &topSize, &leftSize, &bottomSize, &rightSize);
//     SDL_SetWindowPosition(g_gfxContext.m_window, 0, topSize);
//     SDL_SetWindowSize(g_gfxContext.m_window, displaySize.w, displaySize.h - topSize);
// }

// void GfxContext::windowResizeEvent(uint32_t width, uint32_t height)
// {
//     g_gfxContext.m_windowFramebuffer.m_width = width;
//     g_gfxContext.m_windowFramebuffer.m_height = height;
// }

// void GfxContext::beginFrame()
// {
//     GfxContext::bindFramebuffer(nullptr, FramebufferTarget::Draw);

//     glBindVertexArray(g_gfxContext.m_vao);
//     glBindBuffer(GL_ARRAY_BUFFER, g_gfxContext.m_heaps[(uint32_t)BufferType::Vertex].m_handle);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_gfxContext.m_heaps[(uint32_t)BufferType::Index].m_handle);
//     glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gfxContext.m_heaps[(uint32_t)BufferType::Indirect].m_handle);

//     glClearColor(0, 0, 0, 1);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     g_gfxContext.m_immediateData.m_cursor = 0;
// }

//void GfxContext::draw()
//{
//    g_gfxContext.m_defaultView.activate();
//
//    for (uint32_t viewIndex = 0; viewIndex < g_gfxContext.m_activeViews.m_cursor; viewIndex++)
//    {
//        View *view = *(View **)g_gfxContext.m_activeViews.getElem(viewIndex);
//        view->draw();
//        view->m_activeIndex = INVALID_ACTIVE_VIEW_INDEX;
//    }
//
//    g_gfxContext.m_activeViews.m_cursor = 0;
//}

// void GfxContext::endFrame()
// {
//     swapBuffers();
// }

// void GfxContext::swapBuffers()
// {
//     SDL_GL_SwapWindow(g_gfxContext.m_window);
// }