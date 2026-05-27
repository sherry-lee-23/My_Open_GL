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

## 4. 光照贴图
### 4.1 漫反射贴图
漫反射贴图(Diffuse Map)是一个表现了物体所有的漫反射颜色的纹理图像。

将原来的vec3漫反射颜色向量替换为sampler2D
！注意sampler2D是所谓的不透明类型(Opaque Type)，也就是说我们不能将它实例化，只能通过uniform来定义它。如果我们使用除uniform以外的方法（比如函数的参数）实例化这个结构体，GLSL会抛出一些奇怪的错误。这同样也适用于任何封装了不透明类型的结构体。

```cpp
struct Material {
    sampler2D diffuse;  //漫反射贴图
    vec3      specular;
    float     shininess;
}; 
...
in vec2 TexCoords; //当前片段的uv坐标

//main中：
//从纹理中采样片段的漫反射颜色值  
vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
//将环境光的材质颜色设置为漫反射材质颜色同样的值。
vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

//texture(material.diffuse, TexCoords)是去纹理图片中采样当前片段的颜色值

```
### 4.2 镜面光贴图
一个专门用于镜面高光的纹理贴图
```cpp
struct Material {
    sampler2D diffuse;  //漫反射贴图
    sampler2D specular; //镜面光贴图
    float     shininess;
};

vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));   //环境光与漫反射一致
vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));  //漫反射贴图
vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)); //镜面光贴图
FragColor = vec4(ambient + diffuse + specular, 1.0);
```

## 5. 投光物
### 5.1 平行光
当一个光源处于很远的地方时，来自光源的每条光线就会近似于互相平行  
当我们使用一个假设光源处于无限远处的模型时，它就被称为定向光  

所以光源位置不重要
```cpp
struct Light {
    // vec3 position; // 使用定向光就不再需要了
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
...
void main()
{
  vec3 lightDir = normalize(-light.direction);  //向量取反，计算从片源至光照的方向，并标准化
  ...
}
```

### 5.2 点光源
点光源是处于世界中某一个位置的光源，它会朝着所有方向发光，但光线会随着距离逐渐衰减  
#### 衰减
随着光线传播距离的增长逐渐削减光的强度通常叫做衰减(Attenuation) 
Fatt=1.0/(Kc+Kl∗d+Kq∗d2)
你可以看到光在近距离的时候有着最高的强度，但随着距离增长，它的强度明显减弱，并缓慢地在距离大约100的时候强度接近0。
#### 实现衰减
```cpp
struct Light {
    vec3 position;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    //实现衰减要用的公式参数
    float constant; //常数项
    float linear;   //一次项
    float quadratic;//二次项
};

//main中
float distance    = length(light.position - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + 
                light.quadratic * (distance * distance));

ambient  *= attenuation; 
diffuse  *= attenuation;
specular *= attenuation;
```
系数表  

| 距离 | 常数项 | 一次项 | 二次项 |
|------|--------|--------|--------|
| 7    | 1.0    | 0.7    | 1.8 |
| 13   | 1.0    | 0.35   | 0.44 |
| 20   | 1.0    | 0.22   | 0.20 |
| 32   | 1.0    | 0.14   | 0.07 |
| 50   | 1.0    | 0.09   | 0.032 |
| 65   | 1.0    | 0.07   | 0.017 |
| 100  | 1.0    | 0.045  | 0.0075 |
| 160  | 1.0    | 0.027  | 0.0028 |
| 200  | 1.0    | 0.022  | 0.0019 |
| 325  | 1.0    | 0.014  | 0.0007 |
| 600  | 1.0    | 0.007  | 0.0002 |
| 3250 | 1.0    | 0.0014 | 0.000007 |

### 5.3 聚光
聚光是位于环境中某个位置的光源，它只朝一个特定方向而不是所有方向照射光线。这样的结果就是只有在聚光方向的特定半径内的物体才会被照亮，其它的物体都会保持黑暗


OpenGL中聚光是用一个世界空间位置、一个方向和一个切光角(Cutoff Angle)来表示的，切光角指定了聚光的半径（译注：是圆锥的半径不是距光源距离那个半径）。对于每个片段，我们会计算片段是否位于聚光的切光方向之间（也就是在锥形内），如果是的话，我们就会相应地照亮片段。  
LightDir：从片段指向光源的向量。  
SpotDir：聚光所指向的方向。  
Phiϕ：指定了聚光半径的切光角。落在这个角度之外的物体都不会被这个聚光所照亮。  
Thetaθ：LightDir向量和SpotDir向量之间的夹角。在聚光内部的话θ值应该比ϕ值小。  
所以我们要做的就是计算LightDir向量和SpotDir向量之间的点积（还记得它会返回两个单位向量夹角的余弦值吗？），并将它与切光角ϕ
值对比。

#### 手电筒
手电筒(Flashlight)是一个位于观察者位置的聚光，通常它都会瞄准玩家视角的正前方。基本上说，手电筒就是普通的聚光，但它的位置和方向会随着玩家的位置和朝向不断更新  
```cpp
struct Light {
    vec3  position;
    vec3  direction;
    float cutOff;
    ...
};
...
//main中：
float theta = dot(lightDir, normalize(-light.direction));

if(theta > light.cutOff) 
{       
  // 执行光照计算
}
else  // 否则，使用环境光，让场景在聚光之外时不至于完全黑暗
  color = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);

```

#### 边缘柔化
我们需要一个外圆锥，来让光从内圆锥逐渐减暗，直到外圆锥的边界。  
为了创建一个外圆锥，我们只需要再定义一个余弦值来代表聚光方向向量和外圆锥向量（等于它的半径）的夹角。然后，如果一个片段处于内外圆锥之间，将会给它计算出一个0.0到1.0之间的强度值。如果片段在内圆锥之内，它的强度就是1.0，如果在外圆锥之外强度值就是0.0。  
I=(θ−γ)/ϵ  
这里ϵ
(Epsilon)是内（ϕ）和外圆锥（γ）之间的余弦值差（ϵ=ϕ−γ）。最终的I值就是在当前片段聚光的强度。  

| θ | θ（角度） | ϕ（内光切） | ϕ（角度） | γ（外光切） | γ（角度） | ϵ | I |
|---|---|---|---|---|---|---|---|
| 0.87 | 30° | 0.91 | 25° | 0.82 | 35° | 0.91 - 0.82 = 0.09 | (0.87 - 0.82) / 0.09 = 0.56 |
| 0.90 | 26° | 0.91 | 25° | 0.82 | 35° | 0.91 - 0.82 = 0.09 | (0.90 - 0.82) / 0.09 = 0.89 |
| 0.97 | 14° | 0.91 | 25° | 0.82 | 35° | 0.91 - 0.82 = 0.09 | (0.97 - 0.82) / 0.09 = 1.67 |
| 0.83 | 34° | 0.91 | 25° | 0.82 | 35° | 0.91 - 0.82 = 0.09 | (0.83 - 0.82) / 0.09 = 0.11 |
| 0.64 | 50° | 0.91 | 25° | 0.82 | 35° | 0.91 - 0.82 = 0.09 | (0.64 - 0.82) / 0.09 = -2.0 |
| 0.966 | 15° | 0.9978 | 12.5° | 0.953 | 17.5° | 0.9978 - 0.953 = 0.0448 | (0.966 - 0.953) / 0.0448 = 0.29 |

```cpp
float theta     = dot(lightDir, normalize(-light.direction));
float epsilon   = light.cutOff - light.outerCutOff;
float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    
...
// 将不对环境光做出影响，让它总是能有一点光
diffuse  *= intensity;
specular *= intensity;
...
```

