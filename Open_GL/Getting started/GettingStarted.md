# OpenGL 入门总结（基于 GLFW + GLAD + GLM + Shader）

---

## 目录

1. [环境初始化](#环境初始化)
2. [窗口与上下文](#窗口与上下文)
3. [输入处理](#输入处理)
4. [顶点与缓冲对象](#顶点与缓冲对象)
5. [纹理](#纹理)
6. [矩阵与摄像机](#矩阵与摄像机)
7. [渲染循环与深度测试](#渲染循环与深度测试)
8. [Shader 使用](#shader-使用)
9. [代码规范与常用模板](#代码规范与常用模板)

---

## 1. 环境初始化

### 1.1 GLFW 初始化

```cpp
glfwInit();
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac 需要
#endif
```

**说明：**

* GLFW 用于创建窗口和处理输入。
* 设置 OpenGL 版本为 3.3 Core Profile。
* MacOS 下需要 `GLFW_OPENGL_FORWARD_COMPAT`。

---

### 1.2 GLAD 初始化

```cpp
if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
}
```

* GLAD 用于加载 OpenGL 函数指针。
* 必须在创建 OpenGL 上下文之后调用。

---

## 2. 窗口与上下文

```cpp
GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
glfwMakeContextCurrent(window);
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
```

* `glfwCreateWindow(width, height, title, ...)` 创建窗口。
* `glfwMakeContextCurrent(window)` 设置当前上下文。
* 回调函数用于窗口尺寸变化：

```cpp
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
```

---

## 3. 输入处理

### 示例

```cpp
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}
```

* `WASD` 移动摄像机位置。
* 使用 `glm::cross` 获取左右方向向量。
* `deltaTime` 确保移动速度与帧率无关。

---

## 4. 顶点与缓冲对象

### 4.1 顶点数组

```cpp
float vertices[] = {
    // positions        // tex coords
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    ...
};
```

### 4.2 VAO / VBO

```cpp
unsigned int VBO, VAO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);

glBindVertexArray(VAO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```

### 4.3 顶点属性指针

```cpp
// position
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
// texture
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
```

**说明：**

* VAO：存储顶点属性配置。
* VBO：存储顶点数据。
* `stride` 表示每个顶点的字节大小。
* `(void*)offset` 表示偏移量。

---

## 5. 纹理

### 5.1 创建与绑定

```cpp
unsigned int texture1;
glGenTextures(1, &texture1);
glBindTexture(GL_TEXTURE_2D, texture1);

// Wrapping
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// Filtering
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```

### 5.2 加载图片

```cpp
unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
if(data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}
stbi_image_free(data);
```

---

## 6. 矩阵与摄像机

### 6.1 摄像机参数

```cpp
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
```

### 6.2 视图矩阵（摄像机）

```cpp
glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
```

* 摄像机移动 → 改变 `view` 矩阵 → 物体在屏幕位置改变。
* 可用三角函数实现环绕摄像机旋转：

```cpp
float camX = sin(glfwGetTime()) * radius;
float camZ = cos(glfwGetTime()) * radius;
view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,1.0,0.0));
```

### 6.3 投影矩阵

```cpp
glm::mat4 projection = glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
```

* `fov` 视角，`aspect` 宽高比，`near/far` 裁剪平面。
* 将 3D 坐标映射到裁剪空间，最终显示在屏幕上。

### 6.4 模型矩阵

```cpp
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, cubePositions[i]);
model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
```

* 控制物体在世界中的位置和旋转。
* 每个物体可以有独立的 model 矩阵。

---

## 7. 渲染循环与深度测试

```cpp
while(!glfwWindowShouldClose(window)) {
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // 绑定纹理和 shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // 绘制物体
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

* `glEnable(GL_DEPTH_TEST)` 解决物体遮挡关系。
* `glClear` 清空颜色缓冲区和深度缓冲区。

---

## 8. Shader 使用

### 8.1 Shader 模板

```cpp
Shader ourShader("path/to/vertex_shader.vs", "path/to/fragment_shader.fs");
```

* `Shader` 是自定义类，用于封装顶点/片段着色器。
* 构造时加载 `.vs`（顶点着色器）和 `.fs`（片段着色器）文件。

---

### 8.2 Shader 使用方法

```cpp
ourShader.use(); // 激活 shader 程序
ourShader.setInt("texture1", 0); // 设置 uniform 变量
ourShader.setMat4("model", model);
ourShader.setMat4("view", view);
ourShader.setMat4("projection", projection);
```

* `use()`：激活 shader 程序，后续绘制调用都会使用该 shader。
* `setInt`、`setMat4` 等函数用于设置 uniform 变量。
* uniform 是 shader 中的全局变量，可在 CPU 端设置值传入 GPU。

---

### 8.3 顶点着色器 (Vertex Shader)

* 接收顶点位置、纹理坐标等输入。
* 应用 **Model / View / Projection 矩阵**：

```glsl
gl_Position = projection * view * model * vec4(aPos, 1.0);
```

* 作用：把顶点从模型空间 → 世界空间 → 视图空间 → 裁剪空间 → 屏幕空间。

---

### 8.4 片段着色器 (Fragment Shader)

* 接收顶点着色器输出的插值数据（如纹理坐标）。
* 核心功能：计算像素颜色。
* 可访问纹理：

```glsl
vec4 texColor = texture(texture1, TexCoords);
```

---

## 9. 代码规范与常用模板

### 9.1 常用模板结构

```cpp
int main() {
    // 1. 初始化 GLFW/GLAD
    // 2. 创建窗口
    // 3. 配置 VAO/VBO/EBO
    // 4. 加载纹理
    // 5. 创建 shader 程序
    // 6. 设置投影矩阵（projection）
    // 7. 渲染循环:
    //    - 处理输入
    //    - 清空缓冲区
    //    - 激活 shader
    //    - 设置 view/model 矩阵
    //    - 绘制物体
    //    - 交换缓冲/轮询事件
    // 8. 释放资源
}
```

### 9.2 代码规范建议

1. **变量命名**

   * 顶点数组：`vertices`
   * 纹理对象：`texture1`, `texture2`
   * 缓冲对象：`VBO`, `VAO`, `EBO`
   * 矩阵：`model`, `view`, `projection`
   * 摄像机向量：`cameraPos`, `cameraFront`, `cameraUp`

2. **矩阵初始化**

```cpp
glm::mat4 matrix = glm::mat4(1.0f); // 单位矩阵
```

3. **顶点属性**

* 使用 `glVertexAttribPointer` 配置位置、纹理坐标等。
* 注意 `stride` 和偏移量 `offset`。

4. **纹理**

* 每个纹理单独生成并绑定。
* 配置 wrap/filter 参数。
* 加载图片后调用 `glGenerateMipmap()`。

5. **渲染循环**

* 清空颜色/深度缓冲。
* 激活 shader。
* 绑定纹理。
* 设置矩阵 uniform。
* 绘制 VAO。
* 交换缓冲区、轮询事件。

6. **摄像机**

* 使用 `glm::lookAt()` 构建 view 矩阵。
* W/A/S/D 控制位置，环绕可用三角函数计算坐标。
* 视角变化需更新 view 矩阵。

---

### 9.3 常用参数说明

| 参数                                  | 说明                   |
| ----------------------------------- | -------------------- |
| `fov`                               | 投影矩阵视野角度（度数）         |
| `aspect`                            | 宽高比 (`width/height`) |
| `near`, `far`                       | 投影裁剪平面               |
| `glViewport(x, y, w, h)`            | 设置视口位置和大小            |
| `glEnable(GL_DEPTH_TEST)`           | 开启深度测试，防止遮挡错误        |
| `glDrawArrays(GL_TRIANGLES, 0, 36)` | 绘制三角形数组              |
| `glBindVertexArray(VAO)`            | 绑定顶点数组对象             |

---


