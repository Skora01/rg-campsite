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
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool blinn = false;
bool blinnKeyPressed = false;
bool freeCamKeyPressed = false;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;
// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
    PointLight pointLight;
    DirLight dirLight;
    ProgramState()
            : camera(glm::vec3(-9.0f, 3.0f, -12.0f)) {}

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
    //glEnable(GL_CULL_FACE); // Neophodno promeniti redosled za teren da ne bi bio odstranjen

    // build and compile shaders
    // -------------------------
    Shader entityShader("resources/shaders/entity_lighting.vs", "resources/shaders/entity_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader instancedShader("resources/shaders/instanced.vs", "resources/shaders/instanced.fs");
    Shader terrainShader("resources/shaders/terrain.vs", "resources/shaders/terrain.fs");
    Shader hdrShader("resources/shaders/hdr.vs", "resources/shaders/hdr.fs");
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
    //Secode tent model
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

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(0.0f, 0.1f, 1.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 3.0f; // 0.032f;


    DirLight& dirLight = programState->dirLight;
    dirLight.direction = glm::vec3(-1.0f, 173.8f, -35.3f); // -1.0f, -0.2f, -2.3f
    dirLight.ambient =   glm::vec3(0.05f, 0.05f, 0.20f);
    dirLight.diffuse =   glm::vec3( 0.4f, 0.4f, 0.6f);
    dirLight.specular =  glm::vec3(0.5f, 0.5f, 0.7f);


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
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

    unsigned int amount = 100;
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed
    float radius = 50.0f; // r = 50.0f i o = 10.0f je okej sa amount = 100 // amount = 100, radius = 45.0f, offset = 100.0f
    float offset = 20.0f;
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

    amount = 500;
    radius = 10.0f;
    offset = 15.0f;
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
        model = glm::translate(model, glm::vec3(x, 0.0f, z));

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
    // create floating point color buffer
    unsigned int colorBuffer;
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // create depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // First we render the scene into the floating point framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Zasto opet glClear? // Jer sada clearujemo/postavljamo vrednosti za nas framebuffer a gore smo za default

        // don't forget to enable shader before setting uniforms
        entityShader.use();
        //directional light

        entityShader.setVec3("dirLight.direction", dirLight.direction);
        // entityShader.setVec3("dirLight.direction", -1.0f, -0.2f, -0.3f);
        entityShader.setVec3("dirLight.ambient", dirLight.ambient);
        entityShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        entityShader.setVec3("dirLight.specular", dirLight.specular);
        // point light norm = normalize(norm);
        //point light

        entityShader.setVec3("pointLight.position", pointLight.position);
        entityShader.setVec3("pointLight.ambient", pointLight.ambient);
        entityShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        entityShader.setVec3("pointLight.specular", pointLight.specular);
        entityShader.setFloat("pointLight.constant", pointLight.constant);
        entityShader.setFloat("pointLight.linear", pointLight.linear);
        entityShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        //spotlight (turn on if u want flashlight)
        entityShader.setVec3("spotLight.position", programState->camera.Position);
        entityShader.setVec3("spotLight.direction", programState->camera.Front);
        entityShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        entityShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        entityShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        entityShader.setFloat("spotLight.constant", 1.0f);
        entityShader.setFloat("spotLight.linear", 0.09);
        entityShader.setFloat("spotLight.quadratic", 0.032);
        entityShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
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

        //Render loaded tent model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->tentPosition);
        model = glm::rotate(model, glm::radians(programState->tentRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->tentScale));
        entityShader.setMat4("model", model);
        tentModel.Draw(entityShader);

        //Render loaded second tent model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->tent2Position);
        model = glm::rotate(model, glm::radians(programState->tent2Rotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->tent2Scale));
        entityShader.setMat4("model", model);
        tent2Model.Draw(entityShader);

        //Render loaded campfire model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               pointLight.position);
//        entityShader.setVec3("lightColor", glm::vec3(10.0f, 0.0f, 0.0f));//lightColor need to be adjusted
        entityShader.setVec3("lightColor", glm::vec3(150.0f,88.0f,34.0f));
        entityShader.setMat4("model", model);
        campfireModel.Draw(entityShader);

        //Render loaded log model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->logPosition);
        model = glm::rotate(model, glm::radians(programState->logRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->logScale));
        entityShader.setMat4("model", model);
        logModel.Draw(entityShader);

        //Render second log of same model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->secondLogPosition);
        model = glm::rotate(model, glm::radians(programState->secondLogRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->secondLogScale));
        entityShader.setMat4("model", model);
        logModel.Draw(entityShader);

        //Render loaded log 2 model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->log2Position);
        model = glm::rotate(model, glm::radians(programState->log2Rotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->log2Scale));
        entityShader.setMat4("model", model);
        log2Model.Draw(entityShader);


        //Loading terrain
        terrainShader.use();
        terrainShader.setInt("terrainTexture", 0);
        terrainShader.setInt("terrainNormal", 1);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setVec3("dirLight.direction", -1.0f, -0.2f, -0.3f);
        terrainShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.20f);
        terrainShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.6f);
        terrainShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.7f);
        terrainShader.setVec3("pointLight.position", pointLight.position);
        terrainShader.setVec3("pointLight.ambient", pointLight.ambient);
        terrainShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        terrainShader.setVec3("pointLight.specular", pointLight.specular);
        terrainShader.setFloat("pointLight.constant", pointLight.constant);
        terrainShader.setFloat("pointLight.linear", pointLight.linear);
        terrainShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        //spotlight (turn on if u want flashlight)
        terrainShader.setVec3("spotLight.position", programState->camera.Position);
        terrainShader.setVec3("spotLight.direction", programState->camera.Front);
        terrainShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        terrainShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        terrainShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        terrainShader.setFloat("spotLight.constant", 1.0f);
        terrainShader.setFloat("spotLight.linear", 0.09);
        terrainShader.setFloat("spotLight.quadratic", 0.032);
        terrainShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        terrainShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        terrainShader.setVec3("viewPos", programState->camera.Position);
        terrainShader.setVec3("lightColor", glm::vec3(150.0f,88.0f,34.0f));
        terrainShader.setInt("blinn", blinn);

        // render normal-mapped quad
        model = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0))); // rotate the quad to show normal mapping from multiple directions
        terrainShader.setMat4("model", model);
        terrainShader.setVec3("viewPos", programState->camera.Position);
        terrainShader.setVec3("lightPos", programState->pointLight.position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainNormal);
        renderTerrain();





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
        //spotlight (turn on if u want flashlight)
        instancedShader.setVec3("spotLight.position", programState->camera.Position);
        instancedShader.setVec3("spotLight.direction", programState->camera.Front);
        instancedShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        instancedShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        instancedShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        instancedShader.setFloat("spotLight.constant", 1.0f);
        instancedShader.setFloat("spotLight.linear", 0.09);
        instancedShader.setFloat("spotLight.quadratic", 0.032);
        instancedShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        instancedShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        instancedShader.setVec3("viewPos", programState->camera.Position);
        instancedShader.setInt("blinn", blinn);
        instancedShader.setFloat("material.shininess", 32.0f);
        instancedShader.setMat4("projection", projection);
        instancedShader.setMat4("view", view);
        instancedShader.setInt("texture_diffuse1", 0); // Neophodno jer nema .Draw nego glDrawElements


        instancedShader.setVec3("lightColor", glm::vec3(150.0f,88.0f,34.0f));


        glActiveTexture(GL_TEXTURE0); // Neophodno jer nema .Draw nego glDrawElements
        /*
            Ako pretpostavimo da je model zapravo skup od 4 drveta, taj skup sadrzi 4 debla i 4 grane npr.
            i to bi onda bilo treeModel.meshes.size() = 8.
            Za i = 0, prvo ce se povezati tekstura grane, bindovati VAO, i onda ce DrawElementsInstanced
            da iscrta za 10 (amount = 10) modela grane na njihovom prvom meshu (prvom drvetu u skupu/modelu).

            Onda ce za i = 1 da iscrta deblo za prvi mesh/drvo u skupu/modelu, onda opet za i=2 granu, i=3 deblo itd.
         */
        for(unsigned int i = 0; i < treeModel.meshes.size(); i++)
        {
            if(i % 2 == 0)
                glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[0].id); // Stavlja aktiviranu teksturu
             else
                glBindTexture(GL_TEXTURE_2D, treeModel.textures_loaded[2].id);
            glBindVertexArray(treeModel.meshes[i].VAO); // Binduje model sa aktivnom teksturom

            glDrawElementsInstanced(
                        GL_TRIANGLES, treeModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount
            );
            glBindVertexArray(0);

        }

        for(unsigned int i = 0; i < grassModel.meshes.size(); i++)
        {

            glBindTexture(GL_TEXTURE_2D, grassModel.textures_loaded[0].id);

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

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // Unbindujemo nas framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Sada renderujemo teksturu (== scenu) iz naseg framebuffera preko obicnog ali nakon
        // sto je provucemo kroz tone mapping algoritam iz hdrShader.fs
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Da li je ovo neohpodno ako smo to vec uradili pre pravljenja naseg framebuffera? Mozda za svaki slucaj.
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0); // Aktiviramo teksturu (== scenu)
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        hdrShader.setInt("hdr", hdr);
        hdrShader.setFloat("exposure", exposure);
        renderQuad(); // Iscrtamo pravougaonik (preko celog ekrana) na koji je nalepljena tekstura

         std::cout << "hdr: " << (hdr ? "on" : "off") << "| exposure: " << exposure << std::endl;
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !freeCamKeyPressed)
    {
        programState->camera.freeCam = !programState->camera.freeCam;
        freeCamKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
    {
        freeCamKeyPressed = false;
    }
    //turning on Blinn-Phong lighting
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
    {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinnKeyPressed = false;
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

      if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      {
          if (exposure > 0.0f)
              exposure -= 0.001f;
          else
              exposure = 0.0f;
      }
      else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
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
        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 3.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 3.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 3.0);
        ImGui::Text("DirLight:");
        ImGui::DragFloat3("dirLight.direction", (float*)&programState->dirLight.direction);
        ImGui::DragFloat3("dirLight.ambient",  (float*)  &programState->dirLight.ambient);
        ImGui::DragFloat3("dirLight.diffuse", (float*) &programState->dirLight.diffuse);
        ImGui::DragFloat3("dirLight.specular",(float*) &programState->dirLight.specular);
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
        ImGui::Text("Second log");
        ImGui::DragFloat3("Second position", (float*)&programState->secondLogPosition);
        ImGui::DragFloat("Second rotation", &programState->secondLogRotation, 1.0, 0, 360);
        ImGui::DragFloat("Second scale", &programState->secondLogScale, 0.05, 0.1, 4.0);
        ImGui::Text("Log2");
        ImGui::DragFloat3("Log2 position", (float*)&programState->log2Position);
        ImGui::DragFloat("Log2 rotation", &programState->log2Rotation, 1.0, 0, 360);
        ImGui::DragFloat("Log2 scale", &programState->log2Scale, 0.05, 0.1, 4.0);
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
        glm::vec3 pos1(25.0f,  0.0f, 25.0f);
        glm::vec3 pos2(-25.0f, 0.0f, -25.0f);
        glm::vec3 pos3( -25.0f, 0.0f, 25.0f);
        glm::vec3 pos4( 25.0f,  0.0f, -25.0f);
        // texture coordinates
        glm::vec2 uv1(20.0f, 0.0f);
        glm::vec2 uv2(0.0f, 20.0f);
        glm::vec2 uv3(0.0f, 0.0f);
        glm::vec2 uv4(20.0f, 20.0f);
        // normal vector
        glm::vec3 nm(0.0f, 1.0f, 0.0f);

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

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

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


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float terrainVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
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
