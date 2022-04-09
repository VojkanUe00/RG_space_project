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
#include <random>


void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(char const * path);

unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 1000;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


struct DirectionLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};


Camera camera(glm::vec3(0.0f, 0.0f, 70.0f));

bool ssaoButton=false;
bool mouseEnabled = false;
bool cameraMouseMovementUpdateEnabled = true;
float rotateAngle = 0.0f;

void renderCube();
void renderQuad();
float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SPACE", NULL, NULL);
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
    //stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/model.vs", "resources/shaders/model.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader blendingShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");
    Shader shaderGeometryPass("resources/shaders/ssao_geometry.vs", "resources/shaders/ssao_geometry.fs");
    Shader shaderLightingPass("resources/shaders/ssao.vs", "resources/shaders/ssao_lighting.fs");
    Shader shaderSSAO("resources/shaders/ssao.vs", "resources/shaders/ssao.fs");
    Shader shaderSSAOBlur("resources/shaders/ssao.vs", "resources/shaders/ssao_blur.fs");


    // load models
    // -----------
    Model planetModel("resources/objects/mercury_planet/scene.gltf");
    planetModel.SetShaderTextureNamePrefix("material.");

    Model shipModel("resources/objects/space_ship1/mc80-liberty-type-star-cruiser/source/MC80 Liberty type Star Cruiser.obj");
    shipModel.SetShaderTextureNamePrefix("material.");

    Model rockModel("resources/objects/rock/rock.obj");
    rockModel.SetShaderTextureNamePrefix("material.");

    glm::vec3 planetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 shipPosition;

    glm::vec3 rockScale = glm::vec3(0.7f);

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

    float transparentVertices[] = {
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/saturn.png").c_str());

    vector<glm::vec3> saturn
            {
                    glm::vec3(25.0f, 15.0f, -12.48f),
            };

    // shader configuration
    // --------------------
    blendingShader.use();
    blendingShader.setInt("texture1", 0);

    // load textures
    // -------------
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/space/right.jpg"),
                    FileSystem::getPath("resources/textures/space/left.jpg"),
                    FileSystem::getPath("resources/textures/space/up.jpg"),
                    FileSystem::getPath("resources/textures/space/down.jpg"),
                    FileSystem::getPath("resources/textures/space/front.jpg"),
                    FileSystem::getPath("resources/textures/space/back.jpg")
            };
    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    // --------------------
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    modelShader.use();
    modelShader.setInt("material.texture_diffuse1", 0);
    modelShader.setInt("material.texture_specular1", 1);

    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedo;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // also create framebuffer to hold SSAO processing stage
    // -----------------------------------------------------
    unsigned int ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0;

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    // generate noise texture
    // ----------------------
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // lighting info
    // -------------
    glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
    glm::vec3 lightColor = glm::vec3(0.86, 0.3f, 0.2f);

    // shader configuration
    // --------------------
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedo", 2);
    shaderLightingPass.setInt("ssao", 3);
    shaderSSAO.use();
    shaderSSAO.setInt("gPosition", 0);
    shaderSSAO.setInt("gNormal", 1);
    shaderSSAO.setInt("texNoise", 2);
    shaderSSAOBlur.use();
    shaderSSAOBlur.setInt("ssaoInput", 0);


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

        if(mouseEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }


        // render
        // ------
        glClearColor(0.0, 0.0, 0.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(ssaoButton==true){
            // 1. geometry pass: render scene's geometry/color data into gbuffer
            // -----------------------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                                    0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);
            shaderGeometryPass.use();
            shaderGeometryPass.setMat4("projection", projection);
            shaderGeometryPass.setMat4("view", view);

            model = glm::mat4(1.0f);
            model = glm::translate(model, planetPosition);
            model = glm::scale(model, glm::vec3(4.0f));
            shaderGeometryPass.setMat4("model", model);

            // render the loaded model
            planetModel.Draw(shaderGeometryPass);
            shaderGeometryPass.setFloat("material.shininess", 2.0f);
            glm::mat4 model1 = glm::mat4(1.0f);

            model1 = glm::translate(model1, camera.Position +
                                            glm::vec3(camera.Front.x, camera.Front.y - 0.4f, camera.Front.z - 0.8f));
            model1 = glm::scale(model1, glm::vec3(0.0001f));
            model1 = glm::rotate(model1, (float) glm::radians(rotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            model1 = glm::rotate(model1, (float) glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model1 = glm::rotate(model1, glm::radians(-camera.Yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                rotateAngle -= 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                rotateAngle += 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                model1 = glm::rotate(model1, (float) glm::radians(3.0), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                model1 = glm::rotate(model1, (float) glm::radians(-3.0), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            shaderGeometryPass.setMat4("model", model1);

            // render the loaded model
            shipModel.Draw(shaderGeometryPass);

            glm::mat4 model2 = glm::mat4(1.0f);
            glm::vec3 rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()),
                                           10.0f * glm::sin(glfwGetTime()) * glm::cos(glfwGetTime()),
                                           20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(30.0), glm::vec3(0.0f, 1.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()),
                                 20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()),
                                 20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(30.0), glm::vec3(0.0f, 0.0f, 1.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::sin(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()),
                                 20.0f * glm::cos(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(60.0), glm::vec3(0.0f, 1.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::sin(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()),
                                 20.0f * glm::cos(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(60.0), glm::vec3(1.0f, 0.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f, 20.0f * glm::sin(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(60.0), glm::vec3(0.0f, 0.0f, 1.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()), 20.0f);
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(90.0), glm::vec3(0.0f, 1.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::sin(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()), 20.0f);
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(90.0), glm::vec3(1.0f, 0.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f, 20.0f * glm::cos(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(90.0), glm::vec3(0.0f, 0.0f, 1.0f));
            model2 = glm::scale(model2, rockScale);
            shaderGeometryPass.setMat4("model", model2);
            rockModel.Draw(shaderGeometryPass);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 2. generate SSAO texture
            // ------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAO.use();
            // Send kernel + rotation
            for (unsigned int i = 0; i < 64; ++i)
                shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
            shaderSSAO.setMat4("projection", projection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            renderQuad();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);


            // 3. blur SSAO texture to remove noise
            // ------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAOBlur.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            renderQuad();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);


            // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
            // -----------------------------------------------------------------------------------------------------
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shaderLightingPass.use();
            // send light relevant uniforms
            glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
            shaderLightingPass.setVec3("light.Position", lightPosView);
            shaderLightingPass.setVec3("light.Color", lightColor);
            // Update attenuation parameters
            const float linear = 0.09;
            const float quadratic = 0.032;
            shaderLightingPass.setFloat("light.Linear", linear);
            shaderLightingPass.setFloat("light.Quadratic", quadratic);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedo);
            glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
            glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
            renderQuad();
        }
        else {

            // don't forget to enable shader before setting uniforms
            modelShader.use();
            modelShader.setVec3("viewPos", camera.Position);


            // directional light
            modelShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
            modelShader.setVec3("dirLight.ambient", 0.08f, 0.08f, 0.08f);
            modelShader.setVec3("dirLight.diffuse", 1.4f, 1.4f, 1.4f);
            modelShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);


            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                    (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            modelShader.setMat4("projection", projection);
            modelShader.setMat4("view", view);

            modelShader.setFloat("material.shininess", 128.0f);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, planetPosition);
            model = glm::scale(model, glm::vec3(4.0f));
            modelShader.setMat4("model", model);

            // render the loaded model
            planetModel.Draw(modelShader);
            modelShader.setFloat("material.shininess", 2.0f);
            glm::mat4 model1 = glm::mat4(1.0f);

            model1 = glm::translate(model1, camera.Position +
                                            glm::vec3(camera.Front.x, camera.Front.y - 0.4f, camera.Front.z - 0.8f));
            model1 = glm::scale(model1, glm::vec3(0.0001f));
            model1 = glm::rotate(model1, (float) glm::radians(rotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            model1 = glm::rotate(model1, (float) glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model1 = glm::rotate(model1, glm::radians(-camera.Yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                rotateAngle -= 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                rotateAngle += 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                model1 = glm::rotate(model1, (float) glm::radians(3.0), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                model1 = glm::rotate(model1, (float) glm::radians(-3.0), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            modelShader.setMat4("model", model1);

            // render the loaded model
            shipModel.Draw(modelShader);

            glm::mat4 model2 = glm::mat4(1.0f);
            glm::vec3 rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()),
                                           10.0f * glm::sin(glfwGetTime()) * glm::cos(glfwGetTime()),
                                           20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(30.0), glm::vec3(0.0f, 1.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()),
                                 20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()),
                                 20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(30.0), glm::vec3(0.0f, 0.0f, 1.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::sin(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()),
                                 20.0f * glm::cos(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(60.0), glm::vec3(0.0f, 1.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::sin(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()),
                                 20.0f * glm::cos(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(60.0), glm::vec3(1.0f, 0.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f, 20.0f * glm::sin(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(60.0), glm::vec3(0.0f, 0.0f, 1.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::cos(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()), 20.0f);
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(90.0), glm::vec3(0.0f, 1.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f * glm::sin(glfwGetTime()), 20.0f * glm::cos(glfwGetTime()), 20.0f);
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(90.0), glm::vec3(1.0f, 0.0f, 0.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);

            model2 = glm::mat4(1.0f);
            rotation = glm::vec3(20.0f, 20.0f * glm::cos(glfwGetTime()), 20.0f * glm::sin(glfwGetTime()));
            model2 = glm::translate(model2, rotation);
            model2 = glm::rotate(model2, (float) glm::radians(90.0), glm::vec3(0.0f, 0.0f, 1.0f));
            model2 = glm::scale(model2, rockScale);
            modelShader.setMat4("model", model2);
            rockModel.Draw(modelShader);


            // draw skybox as last
            glDepthFunc(
                    GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            // skybox cube
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); // set depth function back to default

            // draw objects
            blendingShader.use();
            projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f,
                                          100.0f);
            view = camera.GetViewMatrix();
            model = glm::mat4(1.0f);
            blendingShader.setMat4("projection", projection);
            blendingShader.setMat4("view", view);

            glBindVertexArray(transparentVAO);
            glBindTexture(GL_TEXTURE_2D, transparentTexture);
            for (unsigned int i = 0; i < saturn.size(); i++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, saturn[i]);
                model = glm::scale(model, glm::vec3(15.0f));
                blendingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
        ssaoButton=true;
    }
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE){
        ssaoButton=false;
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

    if (cameraMouseMovementUpdateEnabled)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        cameraMouseMovementUpdateEnabled = !cameraMouseMovementUpdateEnabled;
        mouseEnabled = !mouseEnabled;
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
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
