#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define DOG    1
#define CEU    2
#define SHIP   3
#define EYES   4
#define FACE   5

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;

// Posicao da luz
uniform vec4 light_position;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(light_position - p);

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2 * n * dot(n,l);

    // Coordenadas de textura U e V
    float U = 0.0f;
    float V = 0.0f;

    vec3 Kd0; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0,1.0,1.0); // Espectro da fonte de luz
    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2); // Espectro da luz ambiente

    if ( object_id == SPHERE )
    {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        float ro = 1;
        vec4 p_ = bbox_center + (position_model - bbox_center)/ro * length(position_model - bbox_center);
        vec4 p_Vet = p_ - bbox_center;

        float theta = atan(p_Vet.x, p_Vet.z);
        float phi = asin(p_Vet.y/ro);

        U = (theta + M_PI)/(2*M_PI);
        V = (phi + (M_PI/2))/M_PI;
        Kd0 = texture(TextureImage0, vec2(U,V)).rgb;

        Ks = vec3(0.05,0.05,0.05);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;

    }
    else if ( object_id == DOG )
    {
        U = texcoords.x;
        V = 1 - texcoords.y;
        Kd0 = texture(TextureImage2, vec2(U,V)).rgb;

        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;
    } else if(object_id == CEU)
    {
        
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        float ro = 1;
        vec4 p_ = bbox_center + (position_model - bbox_center)/ro * length(position_model - bbox_center);
        vec4 p_Vet = p_ - bbox_center;
        float theta = atan(p_Vet.x, p_Vet.z);
        float phi = asin(p_Vet.y/ro);

        U = (theta + M_PI)/(2*M_PI);
        V = (phi + (M_PI/2))/M_PI;
        Kd0 = texture(TextureImage1, vec2(U,V)).rgb;
        

        
        Ia = vec3(0.0, 0.0, 0.3);
        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0,0.0,0.3);
        q = 1.0;
    } else if(object_id == SHIP)
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureImage3, vec2(U,V)).rgb;

        Ks = vec3(0.7,0.7,0.7);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;
    } else if(object_id == EYES){
        U = 1 - texcoords.x;
        V = 1 - texcoords.y;
        Kd0 = texture(TextureImage4, vec2(U,V)).rgb;

        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;
    }
    

    // Termo de iluminação de lambert (difuso!)
    vec3 lambert_diffuse_term = Kd0 * I * (max(0,dot(n,l)) + 0.01);

    // Termo de iluminação ambiente
    vec3 ambient_term = Ka * Ia;

    // Termo especular utilizando o modelo de iluminação de Phong
    vec3 phong_specular_term  = Ks * I * pow(max(dot(r,v),0 ),q) * max(dot(n,l),0); // PREENCH AQUI o termo especular de Phong

    color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 

