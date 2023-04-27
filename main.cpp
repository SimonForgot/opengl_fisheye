#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>

#include <vector>
#include <iostream>

// settings
const unsigned int width = 600;
const unsigned int height = 600;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height)
                                   { glViewport(0, 0, width, height); });

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 1.generate 6 direction textures, merge into a cubemap
    // 2.use fisheye model to sample this cubemap
    Shader fishEyeShader("../fishEyeShader.vs", "../fishEyeShader.fs", "../fishEyeShader.gs");
    Shader viewShader("../viewShader.vs", "../viewShader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    struct triangle_mesh_data
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        std::vector<GLfloat> uvs; // for visualizing quad only
        GLuint VAO, VBO[2], EBO;

        void setupVAO()
        {
            // create
            glCreateVertexArrays(1, &VAO);
            glCreateBuffers(2, VBO);
            glCreateBuffers(1, &EBO);
            // move buffer data
            glNamedBufferStorage(VBO[0], sizeof(GLfloat) * vertices.size(), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
            glNamedBufferStorage(EBO, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
            vertices.clear();
            indices.clear();
            // how to interpret the data storage in buffer
            glVertexArrayVertexBuffer(VAO, 0, VBO[0], 0, sizeof(GLfloat) * 3);
            glVertexArrayElementBuffer(VAO, EBO);
            // specify how to fetch data from buffer to vertex shader
            glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
            glVertexArrayAttribBinding(VAO, 0, 0);
            glEnableVertexArrayAttrib(VAO, 0);
            if (!uvs.empty())
            {
                glNamedBufferStorage(VBO[1], sizeof(GLfloat) * uvs.size(), uvs.data(), GL_DYNAMIC_STORAGE_BIT);
                uvs.clear();
                glVertexArrayVertexBuffer(VAO, 1, VBO[1], 0, sizeof(GLfloat) * 2);
                glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribBinding(VAO, 1, 1);
                glEnableVertexArrayAttrib(VAO, 1);
            }
        }
    } cube, quad;
    cube.vertices = {0.5f, 0.5f, 0.5f,
                     0.5f, -0.5f, 0.5f,
                     -0.5f, -0.5f, 0.5f,
                     -0.5f, 0.5f, 0.5f,
                     0.5f, 0.5f, -0.5f,
                     0.5f, -0.5f, -0.5f,
                     -0.5f, -0.5f, -0.5f,
                     -0.5f, 0.5f, -0.5f};
    cube.indices = {0, 1, 3, 1, 2, 3,
                    0, 4, 5, 5, 1, 0,
                    4, 7, 5, 5, 7, 6,
                    7, 3, 6, 3, 2, 6,
                    7, 4, 0, 7, 0, 3,
                    2, 1, 6, 1, 5, 6};
    quad.vertices = {1.0f, 1.0f, 0.0f,
                     1.0f, -1.0f, 0.0f,
                     -1.0f, -1.0f, 0.0f,
                     -1.0f, 1.0f, 0.0f};
    quad.indices = {0, 1, 3, 1, 2, 3};
    quad.uvs = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    cube.setupVAO();
    quad.setupVAO();

    // 创建FBO
    GLuint FBO;
    glCreateFramebuffers(1, &FBO);

    // in DSA,cubemap is a kind of array texture with 6 layer
    GLuint colorBuffer;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &colorBuffer);
    glTextureStorage2D(colorBuffer, 1, GL_RGBA8, width, height); // one level, no mipmaps
    glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, colorBuffer, 0);

    GLuint depthBuffer;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &depthBuffer);
    glTextureStorage2D(depthBuffer, 1, GL_DEPTH24_STENCIL8, width, height);
    glNamedFramebufferTexture(FBO, GL_DEPTH_ATTACHMENT, depthBuffer, 0);

    // 检查FBO是否完整
    if (glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "FBO is not complete !\n";
        return 0;
    }

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // make sure fbo can render proper size
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    glm::mat4 views[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),  // front
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),   // back
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),   // up
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // down
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),  // left
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))};  // right
    glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 100.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.0f, 0.1f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(-0.1f, 0.0f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.1f, 0.0f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.1f));
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.1f));
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            model = glm::mat4(1.0f);

        // step 1
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindVertexArray(cube.VAO);
        // set mvp
        fishEyeShader.use();
        fishEyeShader.setMat4("model", model);
        fishEyeShader.setMat4("projection", projection);
        // render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        fishEyeShader.setMat4("view", views[0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        /*
        // step 2
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(quad.VAO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textures[4]);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, textures[5]);

        viewShader.use();
        viewShader.setInt("texture1", 0);
        viewShader.setInt("texture2", 1);
        viewShader.setInt("texture3", 2);
        viewShader.setInt("texture4", 3);
        viewShader.setInt("texture5", 4);
        viewShader.setInt("texture6", 5);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        */
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, colorBuffer);
    for (int i = 0; i < 6; i++)
    {
        GLfloat *pixels = new GLfloat[width * height * 4];
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        std::string filename = std::string{"output"} + std::to_string(i) + ".png";
        stbi_write_png(filename.c_str(), width, height, 4, pixels, 0);
    }
    glfwTerminate();
    return 0;
}