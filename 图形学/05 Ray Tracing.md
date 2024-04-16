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

#### 3.2.1 Radiant Intensity（辐射强度）

一个点光源每单位立体角释放的能量：（**不管离光源多远，单位立体角释放的能量都不变**）

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

#### 3.2.2 Radiant Irradiance（辐射照度）

一个点光源每单位面积释放的能量：（**离光源越远，单位面积能量越小，因为离光源越远球面变大而整个球面的能量守恒，平均到单位面积上能力就变小了**）

![image-20240414000324276](Images\image-20240414000324276.png)

![image-20240413235705835](Images\image-20240413235705835.png)

![image-20240413235848651](Images\image-20240413235848651.png)

吸收的能量是投影到和入射光线垂直的平面所接收的能量

![image-20240414000052726](Images\image-20240414000052726.png)

四季的变换也可以用辐射照度解释，当某地与光线越垂直吸收的能量越多温度也就越高



#### 3.2.3 Radiant Radiance（辐射亮度）

指的是一条光线在传播过程中会有什么属性，理解为**某单位投影面积发出的某方向单位立体角的能量**或**某单位投影面积从某个单位立体角接收到的能量**

![image-20240414003636917](Images\image-20240414003636917.png)

****

> **Irradiance VS Radiance**

如下图所示，Irradiance（辐射照度）指的是单位面积接收到的**所有能量**，Radiance（辐射亮度）指的是单位面积接收到的**某一单位立体角的能量**或单位面积往某一单位立体角发出的能量



**两者的关系：**辐射照度等于辐射亮度在所有方向上的积分

![image-20240414004418849](Images\image-20240414004418849.png)



## 4. Bidirectional Reflectance Distribution Function（BRDF 双向反射分布函数）

### 4.1 BRDF介绍

BRDF研究的是光从某个方向进来并且反射到某个方向去，这部分能量应该是多少，它告诉的是如果能量从某个方向进来会往不同的方向去分散多少的能量，也就是说已知入射光能量和角度，射到物体表面会往各个方向辐射，辐射出去的能量和对应的角度是什么

整个过程可以理解为光线打到了某个物体表面被吸收了，吸收后再从物体表面把这部分能量发射出去，所以可以用Irradiance和Radiance的概念来解释反射。

**BRDF定义如下：**

​	对于某个出射方向，算出它的Radiance L，将其和单位面积收到的入射方向的Radiance对应的Irradiance E **相除**

​	它会告诉我们如何把这个表面从某个方向收集到的能量反射到另一个方向去

![image-20240414010347918](Images\image-20240414010347918.png)

![image-20240414010918637](Images\image-20240414010918637.png)

> **Tip：** $[\frac{1}{sr}]$ 是这个比值的单位

​	根据BRDF的定义，如果是镜面反射，BRDF就会表明反射出去的方向分布了所有能量，其他的方向没有任何能量；如果是漫反射，BRDF就会表面进来的这些能量会被均等的分布在各个出去的方向上

​	概念上，**BRDF描述了光线和物体是如何作用的**。正是因为这一概念，它会**决定物体不同的材质物理层面是如何工作的**，也可以说对于拥有特定的材质属性（粗糙度，金属度，反射率等）的材质，有唯一的BRDF



### 4.2 反射方程

![image-20240414014755357](Images\image-20240414014755357.png)

既然BRDF考虑的是从某一个单位立体角入射的能量所产生的irradiance在各个不同的出射方向上的能量分布情况，那么将**所有入射方向的能量对于某一方向的BRDF做积分**就可以求出某一单位面积从各个方向所接收到的能量所产生的irradiance在各个不同出射方向的radiance分布情况，如上图公式所示

>  $L_i(p, \omega_i)cos\theta_id\omega_i$ 指的是单位立体角入射的radiance产生的irradiance垂直于该单位面积的分量，因为只有这部分才是被该单位面积有效吸收的能量



考虑到每个单位面积的出射radiance可能是其他位置单位面积的入射radiance，所以反射方程**存在一个递归的问题**

![image-20240414015752556](Images\image-20240414015752556.png)



### 4.3 渲染方程

BRDF只考虑了接收到的能量的发射，但是物体有可能自发光，将物体自发光产生的能量加上就变成**渲染方程**了

![image-20240414015951157](Images\image-20240414015951157.png)

也就是说物体发出的radiance分为两类：

- **自己发光产生的；**
- **吸收得到的再发射出去；**



![image-20240414022305027](Images\image-20240414022305027.png)

如上图所示，将所有光源和反射面都当作光源做积分



![image-20240414022915101](Images\image-20240414022915101.png)

通过公式可以看到，虽然无法确定入射的radiance是多少，但是其他的诸如自发光产生的radiance，该单位面积的BRDF以及入射radiance的方向是确定的



![image-20240414034748648](Images\image-20240414034748648.png)

通过代码的简化可以得到：
$$
L = E\space + \space KL
$$
**注意这里要和最初的公式进行区分**，简化后的公式可以理解为
$$
反射出来的能量 = 自发光产生的能量 + 反射操作\times反射出来的能量
$$
而原公式是：
$$
反射出来的能量 = 自发光产生的能量 + \int（BRDF \times单位立体角入射的能量的irradiance）
$$
这点一定要注意区分



****

> **光线追踪和展开**

![image-20240414035712130](Images\image-20240414035712130.png)

![image-20240414040511723](Images\image-20240414040511723.png)

这里 $E$ 可以理解为自发光的能量， $KE$ 可以理解为之前没有反射过的光线的能量，也就是**直接光照**，**光栅化主要就完成了这两个部分**，$K^nE$​ 可以理解为弹射了 $n-1$ 次的光线的能量，也就是**间接光照**



这里就引出了一个重要概念——**全局光照**：
$$
全局光照\space = \space直接光照\space+\space 间接光照
$$
![image-20240414041101327](Images\image-20240414041101327.png)

上面六张图从不采用间接光照到间接光照弹射次数增加的结果，可以看到**整体越来越亮且慢慢收敛**



## 5. Probability Density Function（概率密度函数）

![image-20240414033950574](Images\image-20240414033950574.png)

## 6. Monte Carlo Integration（蒙特卡洛积分）

> **为什么使用蒙特卡洛积分：**

​	当想要求解积分但是很难通过解析式的方法求解时使用蒙特卡罗积分

<img src="Images\image-20240414074038463.png" alt="image-20240414074038463" style="zoom: 50%;" />

> **该怎么做**

​	通过平均随机采样的函数值来估计积分结果

![image-20240414075338863](Images\01 Transformation.md)



> 举例：均匀采样

![image-20240414075712983](Images\image-20240414075712983.png)

![image-20240414075737408](Images\image-20240414075737408.png)



****

![image-20240414081455619](Images\image-20240414081455619.png)



## 7. Path Tracing（路径追踪）

> 目的：用于解决Whitted-Style Ray Tracing中的一些问题

![image-20240414081931059](Images\image-20240414081931059.png)

> Whitted-Style Ray Tracing 问题1

![image-20240414082315307](Images\image-20240414082315307.png)

对于glossy的材质，光线打到上面不应该发生像镜子一样的镜面反射，而是应该在镜面反射向量周围的区域都发生反射来使其与相比镜面反射更模糊，但是Whitted-Style做不到这点

> Whitted-Style Ray Tracing 问题2

![image-20240414082553454](Images\image-20240414082553454.png)

Whitted-Style 光线追踪当光线打到漫反射物体上时不在反射，而这是不对的，现实中漫反射物体吸收光后应该也能反射出去



> **渲染方程是基于物理的，正确的**


$$
L_o(p, \omega_o)= L_e(p,\omega_o)+\int_{\Omega+}L_i(p, \omega_i)f_r(p, \omega_i, \omega_o)(n·\omega_i)d\omega_i
$$


但是渲染方程有两个问题，

- 它是一个在半球上的积分
- 它本身是递归的定义，即入射的radiance可能是从其他地方反射过来的，它本身的反射radiance也可能成为其他地方入射的radiance



如果要使用它就要解决这两个问题。



### 7.1 求积分

> **待解决问题1：**积分——使用蒙特卡洛积分求解

**先只看直接光照**

![image-20240414163442177](Images\image-20240414163442177.png)

![image-20240414163857750](Images\image-20240414163857750.png)

![image-20240414163800573](Images\image-20240414163800573.png)

![image-20240414165731164](Images\image-20240414165731164.png)

![image-20240414165945212](Images\image-20240414165945212.png)



**再来引入间接光照**

![image-20240414170153336](Images\image-20240414170153336.png)

![image-20240414170303037](Images\image-20240414170303037.png)

此时还没有解决，**因为会指数爆炸并且会无限递归下去**

![image-20240414171323381](Images\image-20240414171323381.png)

**指数爆炸解决方法：将光线数量限制在1！！**再追踪多次求平均得到一个近似解

![image-20240414171721725](Images\image-20240414171721725.png)

将**N设为1**得到的就是**path tracing（路径追踪）**，将**N设为大于1**的数被称为**distributed ray tracing（分布式光线追踪）**



### 7.2 解决递归问题



![image-20240414172055366](Images\image-20240414172055366.png)

![image-20240414172141603](Images\image-20240414172141603.png)

还有问题吗？有！**现在递归没有结束条件依旧会无限递归**

![image-20240414172425090](Images\image-20240414172425090.png)

而如果**限制弹射次数又会因为损失能量导致不够真实**

![image-20240414190116464](Images\image-20240414190116464.png)



> **解决方法：俄罗斯轮盘赌（RR）**

![image-20240414190248007](Images\image-20240414190248007.png)

**使用俄罗斯轮盘赌的思想使得路径追踪一定概率上停止后面的计算直接返回**

![image-20240414191039751](Images\image-20240414191039751.png)

**算法逻辑：**

![image-20240414191210108](Images\image-20240414191210108.png)

这就**已经是个可以用来使用的路径追踪算法**了，但是它有一个**小问题就是效率不高**

![image-20240414191417479](Images\image-20240414191417479.png)

原因是**随机方向会有很多方向找不到光源而浪费掉**

![image-20240414191535575](Images\image-20240414191535575.png)



> **改进思想：**不在物体上积分，而是在光源上积分，这样可以确保每个方向上的采样都是有效的，但是渲染方程是对物体的单位面积的入射能量进行积分，怎么把它转换成对光源进行积分呢？**这就需要找到 $d\omega$ 和 $dA$ 的关系进行积分换元**

![image-20240414193755311](Images\image-20240414193755311.png)

![image-20240414194430353](Images\image-20240414194430353.png)

![image-20240414194614348](Images\image-20240414194614348.png)



**此时渲染方程的计算分为两部分：**

- 对于光线的直接光照使用积分换元换成计算光源对该范围面积的贡献，不需要使用俄罗斯轮盘赌即可求解
- 对于物体的间接光照使用之前的公式进行计算，需要使用俄罗斯轮盘赌放置无限递归

**算法逻辑：**

![image-20240414194942416](Images\image-20240414194942416.png)



**最后一个小问题：**如果光源和物体之间被遮挡怎么办？

方法很简单：从单位面积发射一条射线指向光源，判断中间有没有碰到其他物体，如果碰到则不进行计算，如果没碰到则计算该光源产生的直接光照

![image-20240414195508591](Images\image-20240414195508591.png)



**！！！路径追踪结果几乎和现实中的一模一样！！！**

![image-20240414200123139](Images\image-20240414200123139.png)



> **目前仍然没有覆盖的部分：**

![image-20240414201209889](Images\image-20240414201209889.png)

![image-20240414201125491](Images\image-20240414201125491.png)
