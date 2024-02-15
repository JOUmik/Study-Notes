# Rasterization(光栅化)

[TOC]

## 1. Introduction

在经过MVP后物体和摄像机被摆放在了正确的位置，下一步就是把物体画出来，这就是rasterization要做的事情， 屏幕就是光栅成像设备，所以光栅化和pixel有关



## 2. Canonical Cube to Screen

Games101中Pixel的原点在左下角（UE设定是左上角为原点），而Canonical Cube是[-1, 1]范围的，要把它映射到对应Pixel上

<img src="Images\image-20240211223353833.png" alt="image-20240211223353833" style="zoom:67%;" />

<img src="Images\image-20240211223629128.png" alt="image-20240211223629128" style="zoom:67%;" />



### 2.1 Viewport transformation(视口变换)

为了将Canonical Cube映射到屏幕空间上，要使用视口变换

<img src="Images\image-20240211223736910.png" alt="image-20240211223736910" style="zoom: 67%;" />

## 3. Triangles - Fundamental Shape Primitives

<img src="Images\image-20240211230155822.png" alt="image-20240211230155822" style="zoom: 67%;" />



## 4. Sample(采样)

在图形学中，采样是一种关键思想，在光栅化这部分，采样用来确定屏幕空间中某个像素对应的RGB应该是多少

<img src="Images\image-20240212105753273.png" alt="image-20240212105753273" style="zoom:67%;" />

在不同的像素中心，确认其是 1 还是 0 ， 1 代表在三角形内， 0 代表在三角形外

<img src="Images\image-20240212110012966.png" alt="image-20240212110012966" style="zoom:67%;" />

判断一点是否在三角形内的方法是叉乘，使用右手螺旋定则，如果方向一致则在三角形内，否则在三角形外

<img src="Images\image-20240212110417486.png" alt="image-20240212110417486" style="zoom:67%;" />

**Tip:**

当某一像素点处于三角形1和三角形2的共边上，那么该像素点的规则自己定义（OpenGL是**记上不记下，记左不记右**）



### 4.1 Bounding Box(包围盒)

如果到此结束，那么每个三角形都要遍历一遍整个屏幕空间，为了降低这种巨大的时间和性能开销，包围盒这一概念随即出现

#### 4.1.1 Axis-aligned Bounding Box(AABB)

<img src="Images\image-20240212111824443.png" alt="image-20240212111824443" style="zoom:67%;" />

 

## 5. Antialiasing(抗锯齿)(反走样)

直接采样后图像会出现如图所示的锯齿，解决锯齿问题是图形学非常重要的一大课题

<img src="Images\image-20240212112603383.png" alt="image-20240212112603383" style="zoom:67%;" />

### 5.1 Sampling Artifacts in CG

#### 5.1.1 Aliasing(锯齿)

#### 5.1.2 Moire Patterns(摩尔纹)

隔行采样等方法会出现这种问题

<img src="Images\image-20240212114901109.png" alt="image-20240212114901109" style="zoom:67%;" />

#### 5.1.3 Wagon Wheel Illusion

有时能看到高速行驶的车，轮子的方向好像反了，这是因为人眼在时间上的采样率低

### 5.2 Blurring Before Sampling

<img src="Images\image-20240212120044537.png" alt="image-20240212120044537" style="zoom:67%;" />

模糊后再采样，锯齿问题小了很多

<img src="Images\image-20240212120153128.png" alt="image-20240212120153128" style="zoom:67%;" />

**Tip：**

不可以先采样再模糊，采样的频率不够导致信息丢失，此时再模糊没有意义（更深层次的原因涉及到信号处理）

### 5.3 Frequency Domain(频域)

​                                         <img src="Images\image-20240212121453526.png" alt="image-20240212121453526" style="zoom:67%;" />

频率（f） 与周期（T）成反比， f 越大变化越快

<img src="Images\image-20240212121546203.png" alt="image-20240212121546203" style="zoom:67%;" />

#### 5.3.1 Fourier Transform(傅里叶变换)

**傅里叶级数展开**：任何一个周期函数都可以把它写成一系列正弦和余弦函数的线性组合以及一个常数项

<img src="Images\image-20240212122053543.png" alt="image-20240212122053543" style="zoom:67%;" />

傅里叶变换（时域 => 频域）和逆傅里叶变换（频域 => 时域）

<img src="Images\image-20240212122159714.png" alt="image-20240212122159714" style="zoom:67%;" />

频率更高的函数需要更好频率的采样才能保证不走样：

<img src="Images\image-20240212122445165.png" alt="image-20240212122445165" style="zoom:67%;" />

 当采样高频率的函数时可能得到的采样结果和采样某低频率的函数相同，此时就会出现走样问题

<img src="Images\image-20240212123158851.png" alt="image-20240212123158851" style="zoom:67%;" />

**傅里叶变换结果：**

右侧的频域图理解方法：中心最低频区域，周围是更高频区域，在不同频率上有多少信息用亮度表示，所以示例图中频域图的大多数信息在低频区域，低频信息多，高频信息少

频域图中类似坐标轴一样的白线是因为把无数张一样的图无缝连接在一起，边界两边的变化剧烈导致的，可以**用此来判断图片是否可以无缝连接**，可以无缝连接的图片应该没有这个白色的轴

<img src="Images\image-20240212123500925.png" alt="image-20240212123500925" style="zoom:67%;" />

**高通滤波**：将低频信息抹掉再用逆傅里叶变换的结果，所以高频信息代表着图像中的轮廓边界，因为边界两边变化剧烈所以是高频的

<img src="Images\image-20240212123930427.png" alt="image-20240212123930427" style="zoom:67%;" />

 **低通滤波**：将高频信息去掉，即把轮廓去掉变模糊

<img src="Images\image-20240212124328778.png" alt="image-20240212124328778" style="zoom:67%;" />

**带通滤波**：不是特别明显的轮廓信息

<img src="Images\image-20240212124534756.png" alt="image-20240212124534756" style="zoom:67%;" />

#### 5.3.2 Convolution(卷积)

<img src="Images\image-20240212130622213.png" alt="image-20240212130622213" style="zoom: 67%;" />

对时域上的图进行卷积和对其对应频域上的图乘以卷积频域上的图得到的结果一样

<img src="Images\image-20240212130953222.png" alt="image-20240212130953222" style="zoom:67%;" />



#### 5.3.3 Filter(滤波器)

**Box Filter(低频滤波器)**:

<img src="Images\image-20240212131550818.png" alt="image-20240212131550818" style="zoom: 50%;" />

Box越大失去更多的较高频信息

### 5.4 Antialiasing

反走样有两种方法：

- 一种是物理上使用更高分辨率的屏幕来提高采样率；
- 一种是使用模糊过滤掉高频信息，再采样；

<img src="Images\image-20240212132615605.png" alt="image-20240212132615605" style="zoom:67%;" />

<img src="Images\image-20240212132827313.png" alt="image-20240212132827313" style="zoom:67%;" />

#### 5.4.1 MSAA(多重采样抗锯齿)

假设像素能被划分为无数个小的像素，每个像素都有一个中心，将它们的结果加起来平均

<img src="Images\image-20240212134211761.png" alt="image-20240212134211761" style="zoom: 80%;" />

**Step 1:** 近似实现模糊这个步骤

<img src="Images\image-20240212134312839.png" alt="image-20240212134312839" style="zoom:67%;" />

**Step 2**: 近似实现采样这个步骤

<img src="Images\image-20240212134354581.png" alt="image-20240212134354581" style="zoom:67%;" />

**Cost:** 

增大计算量（N*N）



#### 5.4.2 FXAA(快速近似抗锯齿)

不增加样本数，类似图像的后期处理，将图像渲染出来进行边缘检测，再将边缘进行模糊，之后显示在屏幕上



#### 5.4.3 TAA(时间性抗锯齿)

利用上一帧的信息



## 6. Z-buffering(深度缓存)

解决物体的可见性和物体之间的遮挡问题

### 6.1 Painter‘s Algorithm(画家算法)

由远及近一层层画，新画的覆盖之前画的

**问题**：会出现无法解决的深度顺序（互相遮挡），如下图所示：

<img src="Images\image-20240212162711821.png" alt="image-20240212162711821" style="zoom:67%;" />

### 6.2 Z-buffering

同时生成两张图，一张是frame buffer存储颜色信息，另一个是depth buffer存储深度信息

<img src="Images\image-20240212162847516.png" alt="image-20240212162847516" style="zoom:67%;" />

越远值越大颜色越白，越近值越小颜色越黑

<img src="Images\image-20240212163122852.png" alt="image-20240212163122852" style="zoom: 80%;" />

**伪代码**：

（相同深度的话不进行更新，保存最早的）

<img src="Images\image-20240212163434437.png" alt="image-20240212163434437" style="zoom:80%;" />
