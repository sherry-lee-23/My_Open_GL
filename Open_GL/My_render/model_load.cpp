#include <glad/glad.h> 
#include <GLFW\glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#include"shader.h"
#include"Camera.h"
#include"mesh.h"
#include"model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH =1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("C:\\Users\\Lenovo\\source\\repos\\Open_GL\\Open_GL\\My_render\\1.model_loading.vs",
        "C:\\Users\\Lenovo\\source\\repos\\Open_GL\\Open_GL\\My_render\\1.model_loading.fs");

    // load models
    // -----------
    Model ourModel("resources/old_street/old_street.obj");

    std::vector<glm::vec3> lightPositions =
    {
        {-0.304284f, -1.43316f, 1.7528f},
        {-0.313679f, -1.40737f, 1.13882f},
        {-0.307625f, -1.42418f, 0.459039f},
        {-1.24758f,  -1.38567f, 2.11225f},
        {-1.06493f,  -1.39086f, 1.61783f},
        {-1.04174f,  -1.27249f, 0.606554f},
        { -0.306607, - 1.42174, 1.78611},
        { - 3.86744, - 1.26542, 0.0492588},
        { -1.59988, - 1.46574, 1.09972 }
        //{ -6.58501, - 1.42954, - 0.246204 },
        //{ -6.52097, - 1.4373, - 1.17733},
        //{ -4.00341, -1.23614, 0.00637184}
        //{ -2.21152, - 1.23723, - 0.589492},
        //{ -1.61703, - 1.40736, - 0.687754},
        //{ -0.348447, -1.4294, -1.56532},
        //{ -1.32026, -1.43049, -1.81467}
    };

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.005f, 0.01f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // don't forget to enable shader before setting uniforms
        ourShader.use();

        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setVec3("dirLight.direction",
            glm::vec3(-1.0f, -1.0f, -0.5f));

        ourShader.setVec3("dirLight.ambient",
            glm::vec3(0.01f, 0.01f, 0.03f));

        ourShader.setVec3("dirLight.diffuse",
            glm::vec3(0.25f, 0.2f, 0.45f));  // 蓝紫主光

        ourShader.setInt("pointLightCount",
            lightPositions.size());
        for (int i = 0; i < lightPositions.size(); i++)
        {
            std::string base =
                "pointLights[" + std::to_string(i) + "]";

            ourShader.setVec3(base + ".position", lightPositions[i]);

            ourShader.setVec3(base + ".ambient",
                glm::vec3(0.003f, 0.002f, 0.0f));

            ourShader.setVec3(base + ".diffuse",
                glm::vec3(1.0f, 0.65f, 0.35f)); // 更暖一点但更弱

            ourShader.setFloat(base + ".constant", 1.0f);
            ourShader.setFloat(base + ".linear", 0.55f);
            ourShader.setFloat(base + ".quadratic", 0.45f);
        }


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


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
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime/5);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime/5);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime/5);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime/5);

    static bool lastMouseLeft = false;

    bool currentMouseLeft =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (currentMouseLeft && !lastMouseLeft)
    {
        std::cout << "Camera Position: "
            << camera.Position.x << " "
            << camera.Position.y << " "
            << camera.Position.z << std::endl;
    }

    lastMouseLeft = currentMouseLeft;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

