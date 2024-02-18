# Ray Tracing(光线追踪)

[TOC]



## 1. Introduction(介绍)

光栅化最大的问题是不是很好去表示全局的效果（global effects），比如软阴影、Glossy reflection(光泽度材质)、间接光照(弹射不止一次的光线)

<img src="Images\image-20240217141247199.png" alt="image-20240217141247199" style="zoom: 80%;" />

这些情况下光栅化非常的低效和麻烦

****

**问：为什么使用光线追踪？**

**答：**因为光栅化是一种很快的，但是只能近似模拟的方法， 而光线追踪非常准确，质量非常高，但是缺点是非常慢，所以常用于电影特效等

****

### 1.1 Light Rays(光线)

**三个假设：**

- 光线沿直线传播；
- 光线不与其他任何物体发生碰撞；
- 光线从光源进入人眼，而人眼可以发出感知的光线到达光源（**光线追踪利用的就是光路的可逆性**）；



### 1.2 Ray Casting(光线投射)

从人眼向每个像素投射一个光线，当光线和某个最近的物体相交于某点（即看到该物体），从该点向光源做一个连线，如果中间没有遮挡则表示物体该点处于光源中，否则处于阴影中

<img src="Images\image-20240217144018814.png" alt="image-20240217144018814" style="zoom:67%;" />

****

**前提：**假设人眼是个针孔摄像机（单位面积的一个镜头），光源是点光源，光线会发生完美的反射或折射

<img src="Images\image-20240217144401408.png" alt="image-20240217144401408" style="zoom:67%;" />

<img src="Images\image-20240217144700370.png" alt="image-20240217144700370" style="zoom:67%;" />



## 2. Recursive (Whitted-Style) Ray Tracing(递归的光线追踪)

该方法考虑了光的多次折射和反射

<img src="Images\image-20240217144909946.png" alt="image-20240217144909946" style="zoom:67%;" />

 ### 2.1 Principle(原理)

<img src="Images\image-20240217173207159.png" alt="image-20240217173207159" style="zoom:67%;" />

所有能看到的折射或反射的点的着色结果都加回到该像素，经过折射或反射，能量都会有损失来防止过曝

<img src="Images\image-20240217173741933.png" alt="image-20240217173741933" style="zoom:67%;" />

### 2.2 Ray-Surface Intersection(光线与平面交点)

光线被定义为一条射线，有一个起点 $o$ 和方向 $d$ 

**Ray equation:**                   
$$
r(t)= o + td  \qquad 0 \leq t \lt \infty
$$

#### 2.2.1 Ray Intersection With Sphere(光线与球面的交点)

<img src="Images\image-20240217175012390.png" alt="image-20240217175012390" style="zoom:67%;" />

<img src="Images\image-20240217175112124.png" alt="image-20240217175112124" style="zoom:67%;" />

#### 2.2.2 Ray Intersection With Implicit Surface(光线与隐式表面的交点)

<img src="Images\image-20240217175316314.png" alt="image-20240217175316314" style="zoom:67%;" />

#### 2.2.3 Ray Intersection With Triangle Mesh(光线与三角面片的交点)

<img src="Images\image-20240217175638827.png" alt="image-20240217175638827" style="zoom:67%;" />

为了判断三角形是否和光线相交，可以**先计算三角形所在平面和光线的交点**，再用**叉乘**判断该点是否在三角形内

<img src="Images\image-20240217175915682.png" alt="image-20240217175915682" style="zoom:67%;" />

平面被该平面的法线和该平面的一点所决定（**点法式**）

<img src="Images\image-20240217180356466.png" alt="image-20240217180356466" style="zoom:67%;" />

<img src="Images\image-20240217182910254.png" alt="image-20240217182910254" style="zoom:67%;" />

****

**重心坐标的方法：**

<img src="Images\image-20240217183427861.png" alt="image-20240217183427861" style="zoom:67%;" />



#### 2.2.4 Accelerating Ray-Surface Intersection(光线与表面求交的加速)

**简单的思想**：将模型的所有三角形都判断是否和光线相交即可判断光线与哪些三角面片相交，问题是计算量巨大，所以不能简单这么做

##### 2.2.4.1 Bounding Volumes(包围盒)

先判断光线是否和包围盒相交，再判断是否和里面的物体相交

<img src="Images\image-20240217183955308.png" alt="image-20240217183955308" style="zoom:67%;" />

<img src="Images\image-20240217185257199.png" alt="image-20240217185257199" style="zoom:67%;" />

****

**Ray Intersection with Axis-Aligned Box(光线和包围盒求交)：**

<img src="Images\image-20240217185953111.png" alt="image-20240217185953111" style="zoom:67%;" />
$$
t_{enter} = \color{#00F}{max} \color{#000}\{t_{min}\}, \quad t_{exit} = \color{#F00}{min}\color{#000}\{t_{max}\}
$$
<img src="Images\image-20240217190822030.png" alt="image-20240217190822030" style="zoom:67%;" />

**特殊情况：**

- 当 $t_{exit} < 0$ 时，说明包围盒在光线后面；

- 当 $t_{exit} \geq 0 \;\; \&\& \;\; t_{enter} < 0 $时，说明光源在包围盒里；

<img src="Images\image-20240217191114320.png" alt="image-20240217191114320" style="zoom: 67%;" />

**问：为什么使用Axis-Aligned的包围盒？**

**答：**这样计算交点更简单，只需要使用每个轴的分量进行计算即可

<img src="Images\image-20240217191516230.png" alt="image-20240217191516230" style="zoom:67%;" />

##### 2.2.4.2 Uniform Spatial Partitions(Grids)

- **Preprocess - Build Acceleration Grid**

<img src="Images\image-20240217194941475.png" alt="image-20240217194941475" style="zoom:67%;" />

- **Ray-Scene Intersection**

<img src="Images\image-20240217195155216.png" alt="image-20240217195155216" style="zoom:67%;" />

画直线用到了**Bresenham直线算法**，有时间可以了解一下

**缺陷**：该方法对于很空旷的场景效果不是很好并且做了大量无意义的计算

##### 2.2.4.3 Spatial Partitions(空间划分)

- **Oct-Tree**: 八叉树；

- **KD-Tree**：水平和竖直划分交替进行；
- **BSP-Tree**：每次随机划分一条没有穿过模型的线；

<img src="Images\image-20240217200348046.png" alt="image-20240217200348046" style="zoom:67%;" />

****

**KD-Tree Pre-Processing**

<img src="Images\image-20240217201519606.png" alt="image-20240217201519606" style="zoom:67%;" />

物体只存在叶子节点上

<img src="Images\image-20240217203203429.png" alt="image-20240217203203429" style="zoom:67%;" />

该方法存在很多问题，比如某些物体被多个包围盒划分等等，由此诞生了物体划分BVH方法



##### 2.2.4.4 Object Partition & Bounding Volume Hierarchy(BVH)

先把所有物体分为两堆，面向这两堆来计算包围盒，不断递归下去，当叶子节点三角形数量足够少时停止递归，由此**保证了物体不会被多个包围盒划分**

<img src="Images\image-20240217203807177.png" alt="image-20240217203807177" style="zoom:67%;" />

BVH引起的包围盒相交问题不会有太大的问题

****

**如何划分**

X,Y,Z三个维度按顺序划分

**启发方法1：**每次划分按X,Y,Z中最长的那个轴划分

**启发方法2：**取沿划分轴看过去中间的三角形划分，保证划分后两部分三角形的数量差不多，使得树更平衡，保证深度不会爆炸（**快速选择算法 O(n)时间复杂度**）



物体发生移动或添加了新的物体则需要计算新的BVH

****

**数据结构：**

<img src="Images\image-20240217211743773.png" alt="image-20240217211743773" style="zoom:67%;" />

**伪代码：**

<img src="Images\image-20240217211816701.png" alt="image-20240217211816701" style="zoom:67%;" />

****

**Spatial vs Object Partitions: **

<img src="Images\image-20240217212055745.png" alt="image-20240217212055745" style="zoom:67%;" />



## 3. Basic Radiometry(辐射度量学)

**为什么会用到辐射度量学？**

冯氏反射模型中提到的光强 $I$ 是一种没有单位的经验值，它代表着什么并不知道，Whitted-Style Ray Tracing没有给出贴近现实的结果，而后续的光线追踪研究中，随着对于拟真要求的越来越高，有必要涉及辐射度量学，因为**辐射度量学就是一种能够精准给出光的物理量的方法**

****

**Introduction**

辐射度量学描述了如何测量光照，是一种在物理上准确定义光照的方法。定义了光的很多在空间中的属性，但是**仍然假设光沿直线传播**

- **Radiant flux**：辐射通量
- **Intensity**：辐射强度
- **Irradiance**： 辐射照度
- **Radiance**：辐射亮度

****

### 3.1 Radiant Energy and Flux(Power)

**Radiant Energy定义：**

​	电磁辐射的能量，单位是焦耳（$J$）

<img src="Images\image-20240217213725465.png" alt="image-20240217213725465" style="zoom:67%;" />

**Radiant Flux（辐射通量）定义：**

​	单位时间释放的能量，单位是功率（$W$）或流明（$lm$），比如60W的灯要比40W的灯亮

<img src="C:\Users\Yan Liu\AppData\Roaming\Typora\typora-user-images\image-20240218020522923.png" alt="image-20240218020522923" style="zoom:67%;" />

 ### 3.2 Important Light Measurements of Interest

#### 3.2.1 Radiant Intensity（辐射强度）：

一个点光源每单位立体角释放的能量：

<img src="Images\image-20240218021335437.png" alt="image-20240218021335437" style="zoom:67%;" />

- **Solid Angle（立体角）**

<img src="Images\image-20240218021645418.png" alt="image-20240218021645418" style="zoom:67%;" />

**单位立体角：**

<img src="Images\image-20240218022039694.png" alt="image-20240218022039694" style="zoom:67%;" />

<img src="Images\image-20240218022123091.png" alt="image-20240218022123091" style="zoom:67%;" />

对于均匀向外发散能量的点光源，其辐射强度是
$$
I = \frac {\Phi}{4\pi}
$$
