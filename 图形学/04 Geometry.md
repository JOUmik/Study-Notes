# Geometry(几何)

[TOC]

 ## 1. “Implicit” Representations of Geometry(几何的隐式表示)

不会直接告诉每一个点的坐标，但是会告诉点与点之间的几何关系

### 1.1 公式表达

<img src="Images\image-20240215111924074.png" alt="image-20240215111924074" style="zoom:80%;" />

**缺点**：采样很困难（找到满足几何关系的所有点很难）

<img src="Images\image-20240215112052576.png" alt="image-20240215112052576" style="zoom:80%;" />

**优点**：判断某个点是否在这个面上很容易，是在面的外部还是内部

<img src="Images\image-20240215112229155.png" alt="image-20240215112229155" style="zoom:80%;" />



### 1.2 布尔运算

<img src="Images\image-20240215115719485.png" alt="image-20240215115719485" style="zoom:80%;" />

### 1.3 距离函数

<img src="Images\image-20240215120105399.png" alt="image-20240215120105399" style="zoom:80%;" />

![image-20240216104607576](Images\image-20240216104607576.png)

## 2. “Explicit” Representations of Geometry(几何的显式表示)

<img src="Images\image-20240215112907369.png" alt="image-20240215112907369" style="zoom:80%;" />

**优点**：采样很简单（ 获得表面的所有点很简单）

<img src="Images\image-20240215113050570.png" alt="image-20240215113050570" style="zoom:80%;" />

**缺点**：判断点在几何体的内外很困难

<img src="Images\image-20240215113224274.png" alt="image-20240215113224274" style="zoom:80%;" />

**Tip:** 隐式几何表示和显式几何表示的优缺点相反，需要根据需求选取合适的几何表达方法

 

### 2.1 Point Cloud(点云)

$x,y,z$ 的列表，当点足够密时就组成了模型，三维扫描的输出一般就是点云

![image-20240216113227138](Images\image-20240216113227138.png)



### 2.2 Polygon Mesh(多边型面片)

在图形学中最广泛应用的一种几何表示方法

![image-20240216113452571](Images\image-20240216113452571.png)

.obj文件中存储的就是顶点坐标、纹理坐标、法线向量以及它们与三角面片或四边形面片的对应关系

![image-20240216113711496](Images\image-20240216113711496.png)



### 2.3 Bezier Curves(贝塞尔曲线)

用一系列控制点定义某个曲线

<img src="Images\image-20240216115537354.png" alt="image-20240216115537354" style="zoom:67%;" />

<img src="Images\image-20240216115818187.png" alt="image-20240216115818187" style="zoom:67%;" />

<img src="Images\image-20240216123517590.png" alt="image-20240216123517590" style="zoom:67%;" />

<img src="Images\image-20240216123701154.png" alt="image-20240216123701154" style="zoom:67%;" />

****

**博恩斯坦多项式（Bernstein polynomials）：**
$$
B_i^n(t) = C_n^it^i(1-t)^{n-i}
$$
****

<img src="Images\image-20240216123856448.png" alt="image-20240216123856448" style="zoom:67%;" />

贝塞尔曲线具有凸包性质，即整个贝塞尔曲线在几个控制点形成的凸包内

#### 2.3.1 Continuity(连续性)

- **$C^0$ 连续**：两端贝塞尔曲线 $a$ 和 $b$ ，$a$ 的终止点 $a_n$ 和 $b$ 的起始点 $b_0$ 重合；
-  **$C^1$ 连续**：拥有 $C^0$ 连续的所有性质且 $a_{n-1}, a_n, b_0, b_1$ 共线，同时保证$Distance(a_{n-1}, a_n) == Distance(b_0, b_1)$​（可导）;



### 2.4 Spline(样条曲线)

由 $n$​ 个点控制的连续曲线，曲线经过这些点

<img src="Images\image-20240216193717349.png" alt="image-20240216193717349" style="zoom:50%;" />

#### 2.4.1 B-spline(基函数样条)

博恩斯坦多项式在时间t上几个不同的项对不同的控制点做加权平均

贝塞尔曲线的扩展，能力更强，贝塞尔曲线的问题是当n很大时，动一个点整个曲线都会发生变化，而基函数样条具有局部性，改动一点只会改动这个点的周围



基函数样条极其复杂，有兴趣可以**自学**



## 3. Surfaces(曲面)

### 3.1 Bezier Surfaces(贝塞尔曲面)

将贝塞尔曲线扩展到贝塞尔曲面

<img src="Images\image-20240216210513073.png" alt="image-20240216210513073" style="zoom:67%;" />

<img src="Images\image-20240216210807308.png" alt="image-20240216210807308" style="zoom:80%;" />



### 3.2 Mesh Operations: Geometry Processing(曲面处理)

#### 3.2.1 Mesh Subdivision(曲面细分)(上采样)

<img src="Images\image-20240216213215613.png" alt="image-20240216213215613" style="zoom:67%;" />

##### 3.2.1.1 Loop Subdivision(Loop细分)

**Tip: **和循环没有任何关系，是因为发明者名字是Loop



先细分再调整

<img src="Images\image-20240216213820727.png" alt="image-20240216213820727" style="zoom:67%;" />

关键是细分出更多三角形后调整它们的位置，使模型形状越来越接近正确的形状

- **新顶点的更新**

<img src="Images\image-20240216214142682.png" alt="image-20240216214142682" style="zoom:67%;" />

- **旧顶点的更新**

一方面保留自己的位置，另一方面又受周围老顶点位置的影响

<img src="Images\image-20240216214249278.png" alt="image-20240216214249278" style="zoom:67%;" />

$n$​ : 顶点的度
$$
new\_position = (1 - n \times u) \times original\_position\ + \ u \times neighbor\_position\_sum 
$$


##### 3.2.1.2 Catmull-Clark Subdivision(General Mesh)

Loop细分无法处理非三角形网格，而Catmull细分**更具有普适性**，可以处理非三角形网络



定义 度 != 4 的点为**奇异点（极点）**

<img src="Images\image-20240216220057472.png" alt="image-20240216220057472" style="zoom:67%;" />



**引入更多的点再调整位置**：每个面取个中点，面的每个边取一个中点，将面的中点和边的中点连起来生成更多的面，再调整这些点的位置



- **增加点**

<img src="Images\image-20240216220912645.png" alt="image-20240216220912645" style="zoom: 67%;" />

**在非四边形面里经过细分后，会引入更多的奇异点**，而这是因为划分出三角形面（度为3）导致的，所以只要在非四边形面里细分新的点，那么新的点一定是奇异点，而在**细分之后，非四边形面会消失**，后续细分不会再出现新的奇异点

<img src="E:\学习笔记\Study-Notes\图形学\Images\image-20240216221500250.png" alt="image-20240216221500250" style="zoom:67%;" />

- **调整点**

<img src="Images\image-20240216232028906.png" alt="image-20240216232028906" style="zoom:67%;" />

<img src="Images\image-20240216232147923.png" alt="image-20240216232147923" style="zoom:67%;" />

#### 3.2.2 Mesh Simplification(曲面简化)(下采样)

**Introduction：**在减少面片数量的同时保持整体形状

<img src="Images\image-20240216213304005.png" alt="image-20240216213304005" style="zoom:67%;" />

<img src="Images\image-20240216232622818.png" alt="image-20240216232622818" style="zoom:67%;" />

##### 3.2.2.1 Edge Collapse(边坍缩)

<img src="Images\image-20240216234320271.png" alt="image-20240216234320271" style="zoom:67%;" />

**问题**：该坍缩哪些边对模型形状影响最小

- **Quadric Error Metrics(二次度量误差)**

**简单解释：**新产生的点放在哪个位置产生的二次误差最小

<img src="Images\image-20240216235753714.png" alt="image-20240216235753714" style="zoom:67%;" />



**思路（贪心算法）：**想要简化模型，遍历一遍每个边坍缩后的二次度量误差，优先坍缩二次度量误差最小的边，这样对原模型形状的影响最小。

**该思路需要解决的问题：**坍缩一个边后，它周围的边会发生变化，那么它们的二次度量误差也会发生变化，那么就需要坍缩一个边后立刻更新，这种时候需要用到的数据结构是优先队列



- **坍缩结果**

![image-20240217001333813](Images\image-20240217001333813.png)

#### 3.2.3 Mesh Regularization(曲面正规化)

<img src="Images\image-20240216213334426.png" alt="image-20240216213334426" style="zoom:67%;" />