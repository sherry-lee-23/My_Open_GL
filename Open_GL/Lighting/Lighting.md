# OpenGL 光照部分

## 1. 颜色Color
### 1.1 RGB
OpenGL 中颜色一般使用：
```
glm::vec3(r, g, b)
```
范围：
```
0.0 ~ 1.0
```
### 1.2 物体颜色*光色
最终显示的颜色，是物体颜色与光照颜色的乘积  
片源shader中：
```cpp
vec3 result = objectColor * lightColor;
FragColor = vec4(result, 1.0);
```

## 2. 基础光照Basic Lighting
### 2.1 基础光
1. 环境光照(Ambient Lighting)：即使在黑暗的情况下，世界上通常也仍然有一些光亮（月亮、远处的光），所以物体几乎永远不会是完全黑暗的。为了模拟这个，我们会使用一个环境光照常量，它永远会给物体一些颜色。  
2. 漫反射光照(Diffuse Lighting)：模拟光源对物体的方向性影响(Directional Impact)。它是风氏光照模型中视觉上最显著的分量。物体的某一部分越是正对着光源，它就会越亮。  
3. 镜面光照(Specular Lighting)：模拟有光泽物体上面出现的亮点。镜面光照的颜色相比于物体的颜色会更倾向于光的颜色。  

### 2.2 环境光
我们用光的颜色乘以一个很小的常量环境因子，再乘以物体的颜色，然后将最终结果作为片段的颜色：  
```cpp
void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = ambient * objectColor;
    FragColor = vec4(result, 1.0);
}
```
应用环境光照，保证没有其他光源时也不是全黑的  
### 2.3 漫反射
计算漫反射，需要：
1. 法向量：一个垂直于顶点表面的向量。
2. 定向的光线：作为光源的位置与片段的位置之间向量差的方向向量。为了计算这个光线，我们需要光的位置向量和片段的位置向量。

法向量：在顶点属性中,shader中为Normal
定向的光线：
        光源是静态变量，可以声明为uniform vec3 lightPos;  
顶点shader
```cpp
out vec3 FragPos;  
out vec3 Normal;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;
}
```
片段shader
```cpp
in vec3 FragPos;
vec3 norm = normalize(Normal);   //法线标准化
vec3 lightDir = normalize(lightPos - FragPos); //计算光方向
float diff = max(dot(norm, lightDir), 0.0);  //点乘计算亮度
vec3 diffuse = diff * lightColor;  //得到最终漫反射的颜色

//环境光加上漫反射分量，结果乘物体颜色，得到最终输出颜色
vec3 result = (ambient + diffuse) * objectColor; 
FragColor = vec4(result, 1.0);

```

### 2.4 镜面光
镜面高光Specular Highlights  
观测点在反射光的发方向上时，镜面高光最亮，
uniform vec3 viewPos; //摄像机的位置，即观察点位置
```cpp
float specularStrength = 0.5; //镜面反射强度
vec3 viewDir = normalize(viewPos - FragPos);  //观测方向
vec3 reflectDir = reflect(-lightDir, norm);//第一个参数要的是光源到片段位置的向量，所以要取负值

//计算高光强度，镜面分量
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * lightColor;
```

## 3. 材质
### 3.1 材质属性
之前：
    物体颜色，光照颜色，环境光，漫反射光，镜面光  决定物体的输出效果
增添反光度属性，就有了全部物体材质属性：
```cpp
#version 330 core
struct Material{
    vec3 ambient;//环境光照
    vec3 diffuse;//漫反射光照
    vec3 specular;//镜面光照
    float shininess;//反光度
}
```

### 3.2 设置材质
```cpp
struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

void main()
{    
    // 环境光
    vec3 ambient  = light.ambient * material.ambient;

    // 漫反射 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

|参数|意义|例子|注|
|----|----|----|----|
|material.ambient|材质对环境光的反射能力|vec3(1.0,0.5,0.3)|橙色材质|
|material.diffuse|材质能漫反射的光色|vec3(1.0,0.5,0.3)|物体主要反射红光|
|material.specular|材质能镜面反射的光色/强度|vec3(1.0)|高光反射强度最强|
|material.shininess|材质的反光度，数值越大，反光越集中|4|高光扩散|
|light.ambient|光源的环境光颜色|vec3(0.1)|灰白环境光，RBG三通道都是0.1|
|light.diffuse|光源的漫反射颜色|vec3(1.0,1.0,1.0)|白色主光|
|light.specular|光源的镜面反射颜色|vec3(1.0,0.0,0.0)|红色高光|