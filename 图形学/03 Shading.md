# Shading(着色)

[TOC]



## 1. Introduction

当光栅化把三角形画在屏幕对应像素后，考虑到光影，反光之类的影响，这些像素的颜色都应该是什么，这就是着色负责完成的

<img src="Images\image-20240212171126281.png" alt="image-20240212171126281" style="zoom:67%;" />

## 2. Blinn-Phong Reflectance Model(着色模型：冯氏反射模型)

 冯氏反射模型有三个重要参数：

- **高光（Specular highlights）**；
- **漫反射（Diffuse reflection）**；
- **环境光照（Ambient lighting）**;

<img src="Images\image-20240212204207163.png" alt="image-20240212204207163" style="zoom:67%;" />

<img src="Images\image-20240212203848570.png" alt="image-20240212203848570" style="zoom:67%;" />

着色只考虑局部阴影，不考虑是否光线和某点之间有物体遮挡（**着色 != 阴影**），如下图所示，地面上没有生成物体的阴影

<img src="Images\image-20240212204600442.png" alt="image-20240212204600442" style="zoom:67%;" />

### 2.1 Diffuse Reflection(漫反射)

光线I和法线n之间的夹角决定了接收到的光照，即决定了着色点的单位面积有多亮

<img src="Images\image-20240212204834668.png" alt="image-20240212204834668" style="zoom:67%;" />

<img src="Images\image-20240212210028720.png" alt="image-20240212210028720" style="zoom:67%;" />



<img src="Images\image-20240212210158778.png" alt="image-20240212210158778" style="zoom: 67%;" />

**$ k_d$** 代表吸收率或漫反射系数，如果吸收率为 1 表示不吸收能量全部释放出去，为 0 表示全部吸收，此时为黑色

**Tip**: 漫反射和观察方向没有任何关系

<img src="Images\image-20240212211431465.png" alt="image-20240212211431465" style="zoom:67%;" />

### 2.2 Specular highlights(高光)

**Tip**：观察方向越接近镜面反射方向高光越明显，高光与观察方向有关

<img src="Images\image-20240214151415268.png" alt="image-20240214151415268" style="zoom:50%;" />



冯氏模型的计算方法：观察方向V越接近镜面反射方向，那么法线n和半程向量h之间的夹角越小，所以与漫反射类似，高光的大小与n 和 h之间的夹角有关

<img src="Images\image-20240214151654172.png" alt="image-20240214151654172" style="zoom: 50%;" />

$k_s$ 是镜面反射系数，指数 $p$ 越大高光的可视角度范围越小，一般 $p$ 的大小是 $(100, 200)$ 

**Tip**: 因为冯氏模型是经验模型，所以没有考虑入射角度导致的光的能量衰减，即没有像漫反射公式一样再乘上$n \cdot l$ 

<img src="Images\image-20240214152846034.png" alt="image-20240214152846034" style="zoom: 50%;" />



### 2.3 Ambient lighting(环境光)

冯氏模型假设每个点接受到环境光的强度都是一致的，而实际上的环境光与全局光照有关，非常复杂，冯氏模型的环境光模型是一个非常大胆的假设模型

**Tip**: 环境光是一个常数，和观察方向无关

![image-20240214153054570](Images\image-20240214153054570.png)



### 2.4 Blinn-Phong Reflection Model

布林-冯反射模型就是将三个光加在一起

<img src="Images\image-20240214153438652.png" alt="image-20240214153438652" style="zoom:80%;" />

 ## 3. Shading Frequencies(着色频率)

**Introduction**： 着色频率是指要把着色应用在哪些点上

左边的图是着色应用在每个面，中间的图是着色应用在每个顶点再插值，右边的图是插值获得每个像素的法线，着色应用在每个像素上

<img src="Images\image-20240214160810020.png" alt="image-20240214160810020" style="zoom:80%;" />

- **Flat** shading (着色应用在每个面)：

<img src="Images\image-20240214161345659.png" alt="image-20240214161345659" style="zoom:80%;" />

- **Gouraud** shading (着色应用在每个顶点)：

<img src="Images\image-20240214161420273.png" alt="image-20240214161420273" style="zoom:80%;" />

- **Phong** shading (着色应用在每个像素)：

**Tip**: Phong shading 和 Blinn-Phong Reflectance Model 二者没有关系

<img src="Images\image-20240214161647794.png" alt="image-20240214161647794" style="zoom:80%;" />

**问：着色效果一定是Phong > Gouraud > Flat 吗？**

**答：**当模型足够复杂时，Flat shading效果就已经很好了

<img src="Images\image-20240214161820557.png" alt="image-20240214161820557" style="zoom:80%;" />

### 3.1 顶点法线

将顶点周围面的法线加在一起求平均，按照面积加权平均效果更好

<img src="Images\image-20240214162431162.png" alt="image-20240214162431162" style="zoom:80%;" />

### 3.2 像素法线

顶点之间插值并归一化



## 4. Graphics (Real-time Rendering) Pipeline(渲染管线)

顶点着色器-几何着色器-光栅化-片段着色器-帧缓冲

<img src="Images\image-20240214163313509.png" alt="image-20240214163313509" style="zoom:80%;" />

 Shader是能在硬件上执行的语言，在Shader里关注一个像素需要注意哪些东西，Shader分为 Vertex shader 和 Fragment shader

<img src="Images\image-20240214164426451.png" alt="image-20240214164426451" style="zoom:80%;" />

[Snail (shadertoy.com)](https://www.shadertoy.com/view/ld3Gz2)



## 5. Texture Mapping(纹理映射)

约定U和V都在0，1范围内

<img src="Images\image-20240214180435950.png" alt="image-20240214180435950" style="zoom:80%;" />

<img src="Images\image-20240214180603335.png" alt="image-20240214180603335" style="zoom:80%;" />

## 6. Interpolation Across Triangles: Barycentric Coordinates(重心坐标)

<img src="Images\image-20240214204418789.png" alt="image-20240214204418789" style="zoom:80%;" />

<img src="Images\image-20240214205059786.png" alt="image-20240214205059786" style="zoom:80%;" />

<img src="Images\image-20240214205521960.png" alt="image-20240214205521960" style="zoom:80%;" />

重心坐标可以用以求不同属性的值，比如RGB，UV坐标等

**重心坐标的问题：**在投影变换下无法保证重心坐标不变



## 7. Texture Antialiasing(纹理失真)

应用纹理的潜在问题：

### 7.1 Texture Magnification(纹理放大)

一个分辨率很低的纹理被应用在很大的物体上（比如一面墙），被拉大后因为原本纹理的像素值不足就会出现失真的问题，导致许多pixel共用同一个texel

<img src="Images\image-20240214212617757.png" alt="image-20240214212617757" style="zoom:67%;" />

#### 7.1.1 Bilinear Interpolation(双线性插值)

为了解决失真问题，可以采用双线性插值的方法，图中红点是某个pixel在纹理中对应的坐标，取与它最近的四个点做线性插值，使得每个pixel得到的纹理信息都有些微的不同，来减少失真

<img src="Images\image-20240214212951500.png" alt="image-20240214212951500" style="zoom: 67%;" />

<img src="Images\image-20240214213024729.png" alt="image-20240214213024729" style="zoom: 67%;" />

<img src="Images\image-20240214213306110.png" alt="image-20240214213306110" style="zoom: 67%;" />

### 7.2 Texture Contraction(纹理收缩)

一个分辨率很高的纹理被应用在很小的模型上或被应用在放置于远处的模型上，此时会导致比纹理放大更严重的问题（纹理小的话可以通过算法补充内容，但是纹理大的话只能选取其中有限的texel放置在pixel上）

<img src="Images\image-20240214214336039.png" alt="image-20240214214336039" style="zoom:80%;" />

纹理收缩导致的一个pixel对应多个texel（图中的蓝点是一个pixel）：

<img src="Images\image-20240214214426445.png" alt="image-20240214214426445" style="zoom:80%;" />

#### 7.2.1 Mipmap

 空间换时间，实现了快速的、近似的、**正方形的**范围查询

存储量只比之前多了 $\frac{1}{3}$ ，但是范围查询的时间开销被极大的降低

<img src="Images\image-20240214220205234.png" alt="image-20240214220205234" style="zoom:80%;" />

-  **范围查询**：

<img src="Images\image-20240214222728463.png" alt="image-20240214222728463" style="zoom:80%;" />

<img src="Images\image-20240214222831698.png" alt="image-20240214222831698" style="zoom:80%;" />

$D$ 就是对应的Mipmap的level， 如果 $L$ = 4， 那么在第2层Mipmap会变为1（原图为第0层）

当 $D$ 为浮点数时，比如1.8，此时用Level1和Level2的Mipmap进行插值即可：

<img src="Images\image-20240214223426264.png" alt="image-20240214223426264" style="zoom:80%;" />

（三线性插值）得到层与层之间完全连续的表达

<img src="Images\image-20240214223954264.png" alt="image-20240214223954264" style="zoom:80%;" />

- **Limitations(局限性)：**

过度模糊，只能应用正方形

<img src="Images\image-20240214224229403.png" alt="image-20240214224229403" style="zoom:80%;" />

#### 7.2.2 Anisotropic Filtering(各向异性过滤)

<img src="Images\image-20240214224515974.png" alt="image-20240214224515974" style="zoom:80%;" />

比Mipmap的空间存储量大，是原图的3倍，但是可以应用非正方形（游戏中的2X，4X之类的设置是指压缩多少层，数字越高对显存要求越高，但是并不影响性能）

<img src="Images\image-20240214224652202.png" alt="image-20240214224652202" style="zoom:80%;" />

对于长条形的也可以做较为准确的快速范围查询，但是它仍然无法对斜着的区域进行有效的查询（EWA filtering可以对斜着的区域进行有效的查询，但是时间开销会变大）

<img src="Images\image-20240214224757292.png" alt="image-20240214224757292" style="zoom:80%;" />



## 8. Applications of Textures(纹理的应用)

在现代GPU中，纹理是一块可以进行查询的地址

### 8.1 Environment Map(环境贴图)

<img src="Images\image-20240214230850710.png" alt="image-20240214230850710" style="zoom:80%;" />

将环境光映射到某个纹理中可以用来存储环境光（Spherical Environment Map）：

<img src="Images\image-20240214231257298.png" alt="image-20240214231257298" style="zoom:80%;" />

像地球仪一样展开会有扭曲问题：

<img src="Images\image-20240214231403591.png" alt="image-20240214231403591" style="zoom: 67%;" />

解决方法是使用立方体作为包围盒把它包住并将信息存在立方体（Cube Environment Map）上，这样展开后扭曲问题会少很多，缺点是有些额外的计算来判断光来自立方体的哪个面：

<img src="Images\image-20240214231551301.png" alt="image-20240214231551301" style="zoom: 50%;" />

### 8.2 Bump/Normal Mapping(凹凸贴图/法线贴图）

在不把几何形体变复杂的情况下，只需要一个存储高度（法线方向）的贴图来影响视觉效果

<img src="Images\image-20240214231807214.png" alt="image-20240214231807214" style="zoom:67%;" />

凹凸贴图通过改变相对高度间接改变法线方向：

<img src="Images\image-20240214232319605.png" alt="image-20240214232319605" style="zoom:67%;" />

 <img src="Images\image-20240214232724355.png" alt="image-20240214232724355" style="zoom:67%;" />

<img src="Images\image-20240214233127494.png" alt="image-20240214233127494" style="zoom:67%;" />

### 8.3 Displacement Mapping(位移贴图)

<img src="Images\image-20240214233156617.png" alt="image-20240214233156617" style="zoom:67%;" />
