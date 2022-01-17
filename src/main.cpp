#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <SFML/Audio.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(char const * path);

unsigned int loadCubemap(vector<std::string> faces);

void renderTerrain();

void renderQuad();

bool checkOverlapping(float targetX, float targetY, glm::vec3 center, float overlappingOffset);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool blinn = true;
bool blinnKeyPressed = false;
bool freeCamKeyPressed = false;
float heightScale = 0.1;
bool hdr = true;
bool hdrKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;
glm::vec3 lightColor = glm::vec3(150.0f,88.0f,34.0f);

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Audio
bool muteKeyPressed = false;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    float constant;
    float linear;
    float quadratic;
};


struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 tentPosition = glm::vec3(13.0f, 3.0f, 12.0f);
    float tentRotation = 137.0f;
    float tentScale = 500.0f;

    glm::vec3 tent2Position = glm::vec3(-16.0f, 0.0f, 8.0f);
    float tent2Rotation = 31.0f;
    float tent2Scale = 400.0f;

    glm::vec3 logPosition = glm::vec3(5.0f,-0.2f,0.0f);
    float logRotation = 90.0f;
    float logScale = 0.150f;

    glm::vec3 secondLogPosition = glm::vec3(-4.0f,-0.2f,0.0f);
    float secondLogRotation = 250.0f;
    float secondLogScale = 0.130f;

    glm::vec3 log2Position = glm::vec3(0.0f,0.0f,5.0f);
    float log2Rotation = 250.0f;
    float log2Scale = 0.250f;

    glm::vec3 guitarPosition = glm::vec3(4.0f, 0.0f, -1.0f);
    float guitarRotationY = 272.0f;
    float guitarRotationX = 354.0f;
    float guitarScale = 0.250;

    glm::vec3 ratPosition = glm::vec3(-42.0f, 0.0f, 43.0f);
    float ratRotationY = 1.0f;
    float ratScale = 0.015f;

    glm::vec3 boulder1Position = glm::vec3(2.380f, -3.0f, 16.220f);
    float boulder1Rotation = 180.630f;
    float boulder1Scale = 6.0f;

    glm::vec3 boulder2Position = glm::vec3(-5.560f, 12.0f, 17.150f);
    float boulder2RotationX = 1.0f;
    float boulder2RotationY = 1.0f;
    float boulder2RotationZ = 197.0f;
    float boulder2Scale = 2.3f;

    glm::vec3 axePosition = glm::vec3(-1.0f, 1.6f, 5.0f);
    float axeRotationX = 0.0f;
    float axeRotationY = 202.0f;
    float axeRotationZ = 123.0f;
    float axeScale = 2.05f;

    glm::vec3 towelPosition = glm::vec3(-12.0f,0.2f,6.0f);
    float towelRotation = 57.0f;
    float towelScale = 70.0f;

    PointLight pointLight;
    DirLight dirLight;
    SpotLight spotLight;
    ProgramState()
            : camera(glm::vec3(-22.0f, 3.0f, -47.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader entityShader("resources/shaders/entity_lighting.vs", "resources/shaders/entity_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader instancedShader("resources/shaders/instanced.vs", "resources/shaders/instanced.fs");
    Shader terrainShader("resources/shaders/terrain.vs", "resources/shaders/terrain.fs");
    Shader hdrShader("resources/shaders/hdr.vs", "resources/shaders/hdr.fs");
    Shader shaderBlur("resources/shaders/blur.vs", "resources/shaders/blur.fs");

    // load models
    // -----------
    // Tree model
    Model treeModel("resources/objects/trees/trees.obj");
    treeModel.SetShaderTextureNamePrefix("material.");
    // Grass model
    Model grassModel("resources/objects/grass-patches/grass.obj");
    grassModel.SetShaderTextureNamePrefix("material.");
    // Tent model
    Model tentModel("resources/objects/tent/tent.obj");
    tentModel.SetShaderTextureNamePrefix("material.");
    //Second tent model
    Model tent2Model("resources/objects/tent2.0/tent.obj");
    tent2Model.SetShaderTextureNamePrefix("material.");
    //Campfire model
    Model campfireModel("resources/objects/campfire/Campfire.obj");
    campfireModel.SetShaderTextureNamePrefix("material.");
    //Log model
    Model logModel("resources/objects/log/log.obj");
    logModel.SetShaderTextureNamePrefix("material.");
    //Upright log model
    Model log2Model("resources/objects/log2/log2.obj");
    log2Model.SetShaderTextureNamePrefix("material.");

    // Guitar Model
    Model guitarModel("resources/objects/guitar/guitar.obj");
    guitarModel.SetShaderTextureNamePrefix("material.");

    // Rat Model
    Model ratModel("resources/objects/rat/rat.obj");
    ratModel.SetShaderTextureNamePrefix("material.");

    // Boulder Model
    Model boulder2Model("resources/objects/boulder2/model.obj");
    boulder2Model.SetShaderTextureNamePrefix("material.");

    // Axe Model
    Model axeModel("resources/objects/axe/axe.obj");
    axeModel.SetShaderTextureNamePrefix("material.");

    //Boulder model
    Model boulderModel("resources/objects/boulder/boulder1.obj");
    boulderModel.SetShaderTextureNamePrefix("material.");

    Model towelModel("resources/objects/towel/towel.obj");
    towelModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;

    pointLight.position = glm::vec3(0.0f, 0.1f, 1.0);
    pointLight.ambient = glm::vec3(lightColor.x*0.1, lightColor.y*0.1, lightColor.z*0.1);
    pointLight.diffuse = lightColor;
    pointLight.specular = lightColor;

    pointLight.constant = 0.0f;
    pointLight.linear = 0.05f;
    pointLight.quadratic = 7.05f;

    DirLight& dirLight = programState->dirLight;
    dirLight.direction = glm::vec3(-1.0f, 173.8f, -35.3f);
    dirLight.ambient =   glm::vec3(0.0f, 0.0f, 0.0f);
    dirLight.diffuse =   glm::vec3( 0.05f, 0.05f, 0.05f);
    dirLight.specular =  glm::vec3(0.2f, 0.2f, 0.2f);

    SpotLight& spotLight = programState->spotLight;
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    //skybox VAO,VBO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //load textures
    //------------
    unsigned int terrainTexture = loadTexture(FileSystem::getPath("resources/textures/terrain.jpeg").c_str());
    unsigned int terrainNormal  = loadTexture(FileSystem::getPath("resources/textures/terrain_normal.jpeg").c_str());
    unsigned int terrainSpecular = loadTexture(FileSystem::getPath("resources/textures/terrain_metallic.jpeg").c_str());
    unsigned int terrainHeight = loadTexture(FileSystem::getPath("resources/textures/terrain_height.jpeg").c_str());

    /*
         Mozete da probate da stavite brick teksturu na kojoj se bolje primeti uticaj parallax mapping-a
     */

    //load skybox
    vector<std::string> faces
    {
        FileSystem::getPath("/resources/textures/skybox/right.png"),
        FileSystem::getPath("/resources/textures/skybox/left.png"),
        FileSystem::getPath("/resources/textures/skybox/top.png"),
        FileSystem::getPath("/resources/textures/skybox/bottom.png"),
        FileSystem::getPath("/resources/textures/skybox/front.png"),
        FileSystem::getPath("/resources/textures/skybox/back.png")
    };
    unsigned int cubemapTexture = loadCubemap(faces);


    // Instancing //
    glm::vec3 tentCenter = programState->tentPosition;
    glm::vec3 tent2Center = programState->tent2Position;
    // Trees
    glm::mat4 *modelMatrices;

    unsigned int amount = 100;
    float radius = 50.0f;
    float offset = 18.0f;
    float overlappingOffset = 30.0f;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed

    for(unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;

        model = glm::translate(model, glm::vec3(x, 0.0f, z));
        modelMatrices[i] = model;
    }

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for(unsigned int i = 0; i < treeModel.meshes.size(); i++)
    {
        unsigned int VAO = treeModel.meshes[i].VAO;
        glBindVertexArray(VAO);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

        glBindVertexArray(0);
    }

    // Grass
    amount = 500;
    radius = 20.0f;
    offset = 15.0f;
    overlappingOffset = 7.7f;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed
    for(unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);

        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;

        if(checkOverlapping(x, z, tentCenter, overlappingOffset) || checkOverlapping(x, z, tent2Center, overlappingOffset) || checkOverlapping(x, z, glm::vec3(0.0f), overlappingOffset))
        {
            x = 100.0f;
            z = 100.0f;
        }
        model = glm::translate(model, glm::vec3(x, 0.0f, z));
        model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));

        modelMatrices[i] = model;
    }

    unsigned int buffer2;
    glGenBuffers(1, &buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for(unsigned int i = 0; i < grassModel.meshes.size(); i++)
    {
        unsigned int VAO = grassModel.meshes[i].VAO;
        glBindVertexArray(VAO);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

        glBindVertexArray(0);
    }

    // configure floating point framebuffer
    // ------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffers[2]; // FragColor i BrightColor
    glGenTextures(2, colorBuffers);
    for(unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    shaderBlur.use();
    shaderBlur.setInt("image", 0);

    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);
    hdrShader.setInt("bloomBlur", 1);

    int timer = 0;

    // Audio
    sf::Music crows;
    sf::Music wind;
    sf::Music walking;
    sf::Music horror;
    sf::Music screech;

    if (!crows.openFromFile("resources/audio/crows.mp3"))
        return -1; // error
    crows.play();

    if(!wind.openFromFile("resources/audio/wind.mp3"))
        return -1;
    wind.play();

    if(!walking.openFromFile("resources/audio/walking.mp3"))
        return -1;
    walking.play();

    if(!horror.openFromFile("resources/audio/horror.mp3"))
        return -1;

    if(!screech.openFromFile("resources/audio/screech.mp3"))
        return -1;

    horror.setVolume(20.f);
    screech.setVolume(20.f);
    crows.setVolume(20.f);
    wind.setVolume(40.f);
    walking.setVolume(20.f);

    bool firstTimeFront = true;
    bool firstTimeBehind = true;
    float firstCoor, thirdCoord;
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        timer++;

        // input
        // -----
        processInput(window);

        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. We render the scene into the floating point framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE); // Enable face culling

        // don't forget to enable shader before setting uniforms
        entityShader.use();
        // Directional light
        entityShader.setVec3("dirLight.direction", dirLight.direction);
        entityShader.setVec3("dirLight.ambient", dirLight.ambient);
        entityShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        entityShader.setVec3("dirLight.specular", dirLight.specular);
        // Point light
        entityShader.setVec3("pointLight.position", pointLight.position);
        entityShader.setVec3("pointLight.ambient", pointLight.ambient);
        entityShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        entityShader.setVec3("pointLight.specular", pointLight.specular);
        entityShader.setFloat("pointLight.constant", pointLight.constant);
        entityShader.setFloat("pointLight.linear", pointLight.linear);
        entityShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        // Spotlight
        entityShader.setVec3("spotLight.position", programState->camera.Position);
        entityShader.setVec3("spotLight.direction", programState->camera.Front);
        entityShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        entityShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        entityShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        entityShader.setFloat("spotLight.constant", spotLight.constant);
        entityShader.setFloat("spotLight.linear", spotLight.linear);
        entityShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        entityShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
        entityShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        entityShader.setVec3("viewPos", programState->camera.Position);
        entityShader.setInt("blinn", blinn);
        entityShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 view = programState->camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        entityShader.setMat4("projection", projection);
        entityShader.setMat4("view", view);

        // Render loaded tent model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->tentPosition);
        model = glm::rotate(model, glm::radians(programState->tentRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->tentScale));
        entityShader.setMat4("model", model);
        tentModel.Draw(entityShader);

        glDisable(GL_CULL_FACE);

        //Render loaded second tent model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->tent2Position);
        model = glm::rotate(model, glm::radians(programState->tent2Rotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->tent2Scale));
        entityShader.setMat4("model", model);
        tent2Model.Draw(entityShader);


        glEnable(GL_CULL_FACE);

        // Render loaded campfire model
        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLight.position);
        entityShader.setMat4("model", model);
        campfireModel.Draw(entityShader);

        // Render loaded log model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->logPosition);
        model = glm::rotate(model, glm::radians(programState->logRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->logScale));
        entityShader.setMat4("model", model);
        logModel.Draw(entityShader);

        // Render second log of same model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->secondLogPosition);
        model = glm::rotate(model, glm::radians(programState->secondLogRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->secondLogScale));
        entityShader.setMat4("model", model);
        logModel.Draw(entityShader);

        // Render loaded log 2 model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->log2Position);
        model = glm::rotate(model, glm::radians(programState->log2Rotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->log2Scale));
        entityShader.setMat4("model", model);
        log2Model.Draw(entityShader);


        // Render guitar model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->guitarPosition);
        model = glm::rotate(model, glm::radians(programState->guitarRotationY), glm::vec3(0,1,0));
        model = glm::rotate(model, glm::radians(programState->guitarRotationX), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(programState->guitarScale));
        entityShader.setMat4("model", model);
        guitarModel.Draw(entityShader);


        // Render rat model
        model = glm::mat4(1.0f);
        if(timer <= 600)
        {
            model = glm::translate(model, programState->ratPosition);
            model = glm::rotate(model, glm::radians(programState->ratRotationY), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));
        }
        else if(timer > 600 && timer <= 1200)
        {
            crows.pause();
            model = glm::translate(model, glm::vec3(25.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(108.0f), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));
            screech.play();
            if(timer == 650)
                horror.play();
        }
        else if(timer > 1200 && timer <= 1800)
        {

            model = glm::translate(model, glm::vec3(-23.0f, 0.0f, -28.0f));
            model = glm::rotate(model, glm::radians(234.0f), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));
        }
        else if(timer > 1800 && timer <= 2100)
        {
            model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 21.0f));
            model = glm::rotate(model, glm::radians(38.0f), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));
            wind.pause();
        }
        else if(timer > 2100 && timer <= 2200)
        {
            if(firstTimeFront) {
                firstCoor = programState->camera.Position.x;
                thirdCoord = programState->camera.Position.z;
                firstTimeFront = false;
            }
            model = glm::translate(model, glm::vec3(firstCoor, 0.0f,thirdCoord + 5.0f));
            model = glm::rotate(model, glm::radians(38.0f), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));
        }
        else if(timer > 2200 && timer <= 2450)
        {
            if(firstTimeBehind) {
                firstCoor = programState->camera.Position.x;
                thirdCoord = programState->camera.Position.z;

                firstTimeBehind = false;
            }
            model = glm::translate(model, glm::vec3(firstCoor, 0.0f,thirdCoord - 5.0f));
            model = glm::rotate(model, glm::radians(218.0f), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));
        }
        else {
            crows.play();
            screech.stop();
            model = glm::translate(model, programState->ratPosition);
            model = glm::rotate(model, glm::radians(programState->ratRotationY), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(programState->ratScale));

            if(timer > 5000)
            {
                wind.play();
            }
        }


        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        {

            horror.setVolume(0.f);
            screech.setVolume(0.f);
            crows.setVolume(0.f);
            wind.setVolume(0.f);
            walking.setVolume(0.f);
        }



        entityShader.setMat4("model", model);
        ratModel.Draw(entityShader);
        //Render boulder model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->boulder1Position);
        model = glm::rotate(model, glm::radians(programState->boulder1Rotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->boulder1Scale));
        entityShader.setMat4("model", model);
        boulderModel.Draw(entityShader);

        // Render boulder2 model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->boulder2Position);
        model = glm::rotate(model, glm::radians(programState->boulder2RotationX), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(programState->boulder2RotationY), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(programState->boulder2RotationZ), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(programState->boulder2Scale));
        entityShader.setMat4("model", model);
        boulder2Model.Draw(entityShader);

        //Render towel model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->towelPosition);
        model = glm::rotate(model, glm::radians(programState->towelRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->towelScale));
        entityShader.setMat4("model", model);
        towelModel.Draw(entityShader);

        // Render axe model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->axePosition);
        model = glm::rotate(model, glm::radians(programState->axeRotationX), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(programState->axeRotationY), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(programState->axeRotationZ), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(programState->axeScale));
        entityShader.setMat4("model", model);
        axeModel.Draw(entityShader);

        //Loading terrain
        terrainShader.use();
        terrainShader.setInt("terrainTexture", 0);
        terrainShader.setInt("terrainNormal", 1);
        terrainShader.setInt("terrainHeight",2);
        terrainShader.setInt("terrainSpecular", 3);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        //dirLight
        terrainShader.setVec3("dirLight.direction", dirLight.direction);
        terrainShader.setVec3("dirLight.ambient", dirLight.ambient);
        terrainShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        terrainShader.setVec3("dirLight.specular", dirLight.specular);
        //pointLight
        terrainShader.setVec3("pointLight.position", pointLight.position);
        terrainShader.setVec3("pointLight.ambient", pointLight.ambient);
        terrainShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        terrainShader.setVec3("pointLight.specular", pointLight.specular);
        terrainShader.setFloat("pointLight.constant", pointLight.constant);
        terrainShader.setFloat("pointLight.linear", pointLight.linear);
        terrainShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        // Spotlight
        terrainShader.setVec3("spotLight.position", programState->camera.Position);
        terrainShader.setVec3("spotLight.direction", programState->camera.Front);
        terrainShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        terrainShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        terrainShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        terrainShader.setFloat("spotLight.constant", spotLight.constant);
        terrainShader.setFloat("spotLight.linear", spotLight.linear);
        terrainShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        terrainShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        terrainShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        terrainShader.setVec3("viewPos", programState->camera.Position);
        terrainShader.setInt("blinn", blinn);
        
        // render normal-mapped terrain
        model = glm::mat4(1.0f);
        terrainShader.setMat4("model", model);
        terrainShader.setFloat("height_scale", heightScale);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, terrainHeight);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, terrainSpecular);
        renderTerrain();

        glDisable(GL_CULL_FACE);

        // Render instanced trees
        instancedShader.use();
        //directional light
        instancedShader.setVec3("dirLight.direction", dirLight.direction);
        instancedShader.setVec3("dirLight.ambient", dirLight.ambient);
        instancedShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        instancedShader.setVec3("dirLight.specular", dirLight.specular);
        //point light
        instancedShader.setVec3("pointLight.position", pointLight.position);
        instancedShader.setVec3("pointLight.ambient", pointLight.ambient);
        instancedShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        instancedShader.setVec3("pointLight.specular", pointLight.specular);
        instancedShader.setFloat("pointLight.constant", pointLight.constant);
        instancedShader.setFloat("pointLight.linear", pointLight.linear);
        instancedShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        //spotlight
        instancedShader.setVec3("spotLight.position", programState->camera.Position);
        instancedShader.setVec3("spotLight.direction", programState->camera.Front);
        instancedShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        instancedShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        instancedShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        instancedShader.setFloat("spotLight.constant", spotLight.constant);
        instancedShader.setFloat("spotLight.linear", spotLight.linear);
        instancedShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        instancedShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        instancedShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        instancedShader.setVec3("viewPos", programState->camera.Position);
        instancedShader.setInt("blinn", blinn);
        instancedShader.setFloat("material.shininess", 32.0f);
        instancedShader.setMat4("projection", projection);
        instancedShader.setMat4("view", view);
        instancedShader.setInt("texture_diffuse", 0);
        instancedShader.setInt("texture_normal",1);
        instancedShader.setInt("texture_specular", 2);

        for(unsigned int i = 0; i < treeModel.meshes.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0);
            if(i % 2 == 0) {
                glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[0].id);
                glDisable(GL_CULL_FACE); // We don't want face culling when the branches are drawn
            }
             else {
                glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[2].id);
                glEnable(GL_CULL_FACE);
            }
            glActiveTexture(GL_TEXTURE1);
            if(i % 2 == 0) {
                glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[1].id);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[4].id);
            }
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[3].id);
            glBindVertexArray(treeModel.meshes[i].VAO);

            glDrawElementsInstanced(
                        GL_TRIANGLES, treeModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount
            );
            glBindVertexArray(0);

        }

        glDisable(GL_CULL_FACE);
        for(unsigned int i = 0; i < grassModel.meshes.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassModel.textures_loaded[0].id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, grassModel.textures_loaded[1].id);
            glBindVertexArray(grassModel.meshes[i].VAO);

            glDrawElementsInstanced(
                        GL_TRIANGLES, grassModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount
            );
            glBindVertexArray(0);

        }


        // draw skybox as last
        glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));// remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        //skybox cube
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. We blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        glActiveTexture(GL_TEXTURE0);
        bool horizontal = true, first_iteration = true;
        unsigned int amountBlur = 10;
        shaderBlur.use();
        for (unsigned int i = 0; i < amountBlur; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Now we render floating point color buffer to a 2D quad and tonemap HDR colors to a default framebuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        hdrShader.setInt("bloom", bloom);
        hdrShader.setInt("hdr", hdr);
        hdrShader.setFloat("exposure", exposure);
        renderQuad();

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

         // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
//    glDeleteVertexArrays(1, &skyboxVAO);
    //glDeleteBuffers(1, &skyboxVBO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    //turning on FreeCam
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !freeCamKeyPressed) {
        programState->camera.freeCam = !programState->camera.freeCam;
        freeCamKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        freeCamKeyPressed = false;
    }
    //turning on Blinn-Phong lighting
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed) {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        blinnKeyPressed = false;
    }
    //heightScale for parallax mapping
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }
    // HDR and Exposure keybindings
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed)
      {
          hdr = !hdr;
          hdrKeyPressed = true;
      }
      if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
      {
          hdrKeyPressed = false;
      }
    //turning on/off bloom
      if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !bloomKeyPressed)
      {
          bloom = !bloom;
          bloomKeyPressed = true;
      }
      if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
      {
          bloomKeyPressed = false;
      }
    //turning on/off bloom
      if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
      {
          if (exposure > 0.0f)
              exposure -= 0.001f;
          else
              exposure = 0.0f;
      }
      else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
      {
          exposure += 0.001f;
      }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");

        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

        ImGui::Text("PointLight:");
        ImGui::DragFloat3("pointLight.position", (float*)&programState->pointLight.position);
        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 10.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 10.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 10.0);

        ImGui::Text("DirLight:");
        ImGui::DragFloat3("dirLight.direction", (float*)&programState->dirLight.direction);
        ImGui::DragFloat3("dirLight.ambient",  (float*)  &programState->dirLight.ambient);
        ImGui::DragFloat3("dirLight.diffuse", (float*) &programState->dirLight.diffuse);
        ImGui::DragFloat3("dirLight.specular",(float*) &programState->dirLight.specular);
        ImGui::Text("SpotLight:");
        ImGui::DragFloat("spotLight.constant", &programState->spotLight.constant, 0.05, 0.0, 3.0);
        ImGui::DragFloat("spotLight.linear", &programState->spotLight.linear, 0.05, 0.0, 3.0);
        ImGui::DragFloat("spotLight.quadratic", &programState->spotLight.quadratic, 0.05, 0.0, 3.0);
        ImGui::Text("Tent:");
        ImGui::DragFloat3("Tent position", (float*)&programState->tentPosition);
        ImGui::DragFloat("Tent rotation", &programState->tentRotation, 1.0, 0, 360);
        ImGui::DragFloat("Tent scale", &programState->tentScale, 1.0, 400.0, 500.0);

        ImGui::Text("Tent2:");
        ImGui::DragFloat3("Tent2 position", (float*)&programState->tent2Position);
        ImGui::DragFloat("Tent2 rotation", &programState->tent2Rotation, 1.0, 0, 360);
        ImGui::DragFloat("Tent2 scale", &programState->tent2Scale, 1.0, 400.0, 500.0);

        ImGui::Text("Log");
        ImGui::DragFloat3("Log position", (float*)&programState->logPosition);
        ImGui::DragFloat("Log rotation", &programState->logRotation, 1.0, 0, 360);
        ImGui::DragFloat("Log scale", &programState->logScale, 0.05, 0.1, 4.0);

        ImGui::Text("Second log:");
        ImGui::DragFloat3("Second position", (float*)&programState->secondLogPosition);
        ImGui::DragFloat("Second rotation", &programState->secondLogRotation, 1.0, 0, 360);
        ImGui::DragFloat("Second scale", &programState->secondLogScale, 0.05, 0.1, 4.0);

        ImGui::Text("Log2:");
        ImGui::DragFloat3("Log2 position", (float*)&programState->log2Position);
        ImGui::DragFloat("Log2 rotation", &programState->log2Rotation, 1.0, 0, 360);
        ImGui::DragFloat("Log2 scale", &programState->log2Scale, 0.05, 0.1, 4.0);

        ImGui::Text("Guitar:");
        ImGui::DragFloat3("Guitar position", (float*)&programState->guitarPosition);
        ImGui::DragFloat("Guitar rotation Y", &programState->guitarRotationY, 1.0, 0, 360);
        ImGui::DragFloat("Guitar rotation X", &programState->guitarRotationX, 1.0, 0, 360);
        ImGui::DragFloat("Guitar scale", &programState->guitarScale, 0.05, 0.1, 4.0);

        ImGui::Text("Rat:");
        ImGui::DragFloat3("Rat position", (float*)&programState->ratPosition);
        ImGui::DragFloat("Rat rotation Y", &programState->ratRotationY, 1.0, 0, 360);
        ImGui::DragFloat("Rat scale", &programState->ratScale, 0.001, 0.01, 1.0);

        ImGui::Text("Axe:");
        ImGui::DragFloat3("Axe position", (float*)&programState->axePosition);
        ImGui::DragFloat("Axe rotation X", &programState->axeRotationX, 1.0, 0, 360);
        ImGui::DragFloat("Axe rotation Y", &programState->axeRotationY, 1.0, 0, 360);
        ImGui::DragFloat("Axe rotation Z", &programState->axeRotationZ, 1.0, 0, 360);
        ImGui::DragFloat("Axe scale", &programState->axeScale, 0.05, 0.1, 4.0);

        ImGui::Text("Boulder1:");
        ImGui::DragFloat3("Boulder1 position", (float*)&programState->boulder1Position);
        ImGui::DragFloat("Boulder1 rotation", &programState->boulder1Rotation, 1.0, 0, 360);
        ImGui::DragFloat("Boulder1 scale", &programState->boulder1Scale, 1.0, 1.0, 10.0);

        ImGui::Text("Boulder2:");
        ImGui::DragFloat3("Boulder position", (float*)&programState->boulder2Position);
        ImGui::DragFloat("Boulder rotation X", &programState->boulder2RotationX, 1.0, 0, 360);
        ImGui::DragFloat("Boulder rotation Y", &programState->boulder2RotationY, 1.0, 0, 360);
        ImGui::DragFloat("Boulder rotation Z", &programState->boulder2RotationZ, 1.0, 0, 360);
        ImGui::DragFloat("Boulder scale", &programState->boulder2Scale, 0.05, 0.1, 4.0);

        ImGui::Text("Towel:");
        ImGui::DragFloat3("towel position", (float*)&programState->towelPosition);
        ImGui::DragFloat("towel rotation", &programState->towelRotation, 1.0, 0, 360);
        ImGui::DragFloat("towel scale", &programState->towelScale, 1.0, 50.0, 70.0);

        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT ); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

//Normal mapping for terrain
unsigned int terrainVAO = 0;
unsigned int terrainVBO;
void renderTerrain()
{

    if (terrainVAO == 0)
    {
        // positions
        glm::vec3 pos1(60.0f,  0.0f, 60.0f);
        glm::vec3 pos2(60.0f, 0.0f, -60.0f);
        glm::vec3 pos3( -60.0f, 0.0f, -60.0f);
        glm::vec3 pos4( -60.0f,  0.0f, 60.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 20.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(20.0f, 0.0f);
        glm::vec2 uv4(20.0f, 20.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);
        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);
        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);

        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);

        float terrainVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure terrain VAO
        glGenVertexArrays(1, &terrainVAO);
        glGenBuffers(1, &terrainVBO);
        glBindVertexArray(terrainVAO);
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), &terrainVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}



unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


bool checkOverlapping(float targetX, float targetZ, glm::vec3 center, float overlappingOffset)
{

    // Offset around the campfire should be smaller
    if(center.x == 0 && center.z == 0)
    {
        overlappingOffset= 4;
    }

    if((targetX >= center.x - overlappingOffset && targetX <= center.x + overlappingOffset && targetZ >= center.z - overlappingOffset && targetZ <= center.z + overlappingOffset))
    {
        return true;
    }
    else return false;
}

