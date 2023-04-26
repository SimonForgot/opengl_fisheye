#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>

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
    Shader fishEyeShader("../fishEyeShader.vs", "../fishEyeShader.fs");
    Shader viewShader("../viewShader.vs", "../viewShader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<std::vector<float>> vertices;
    vertices.push_back({0.5f, 0.5f, 0.5f,
                        0.5f, -0.5f, 0.5f,
                        -0.5f, -0.5f, 0.5f,
                        -0.5f, 0.5f, 0.5f,
                        0.5f, 0.5f, -0.5f,
                        0.5f, -0.5f, -0.5f,
                        -0.5f, -0.5f, -0.5f,
                        -0.5f, 0.5f, -0.5f});
    vertices.push_back({1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f});
    std::vector<std::vector<unsigned int>> indices;
    indices.push_back({0, 1, 3,
                       1, 2, 3,
                       0, 4, 5,
                       5, 1, 0,
                       4, 7, 5,
                       5, 7, 6,
                       7, 3, 6,
                       3, 2, 6,
                       7, 4, 0,
                       7, 0, 3,
                       2, 1, 6,
                       1, 5, 6});
    indices.push_back({0, 1, 3,
                       1, 2, 3,
                       0, 4, 5,
                       5, 1, 0});

    GLuint VBO[2], VAO[2], EBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);
    for (GLuint i = 0; i < 2; i++)
    {
        glBindVertexArray(VAO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[i][0]) * vertices[i].size(), vertices[i].data(), GL_STATIC_DRAW);

        if (i == 0)
        {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);
        }
        else
        {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[i][0]) * indices[i].size(), indices[i].data(), GL_STATIC_DRAW);
    }
    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // EBO is recorded in VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 创建FBO
    GLuint fbo[6];
    glGenFramebuffers(6, fbo);
    GLuint textures[6];
    glGenTextures(6, textures);
    GLuint rbo[6];
    glGenRenderbuffers(6, rbo);
    for (GLuint i = 0; i < 6; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);

        // 创建纹理
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);

        // 创建渲染缓冲区
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo[i]);

        // 检查FBO是否完整
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "FBO is not complete !\n";
            return 0;
        }
    }
    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

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
        glBindVertexArray(VAO[0]);
        // set mvp
        fishEyeShader.use();
        fishEyeShader.setMat4("model", model);
        fishEyeShader.setMat4("projection", projection);
        // render
        for (GLuint i = 0; i < 6; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            fishEyeShader.setMat4("view", views[i]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // step 2
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(VAO[1]);
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

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto i = 0; i < 6; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        GLfloat *pixels = new GLfloat[width * height * 4];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        std::string filename = std::string{"output"} + std::to_string(i) + ".png";
        stbi_write_png(filename.c_str(), width, height, 4, pixels, 0);
    }
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(2, EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}