# Transformation

[TOC]

**MVP**分别代指三种变换（Model, View, Projection）

## 1. Model transformation(模型变换)



## 2. View/Camera transformation(视图变换)

保证相机和物体的相对位置

## 3. Projection transformation(投影变换)

### 3.1 Orthographic Projection(正交投影)

![image-20240211171434839](Images\1.png)

### 3.2 Perspective Projection(透视投影)

#### 3.2.1 推理

![image-20240211171652071](Images\image-20240211171652071.png)



透视投影需要先将Frustum(截锥体)变成Cuboid(正方体)，再做正交投影

![image-20240211172201639](Images\image-20240211172201639.png)



转换方法是使用相似三角形

![image-20240211204319898](Images\image-20240211204319898.png)



可以得到除z轴外，透视投影矩阵所有的信息

![image-20240211210807646](Images\image-20240211210807646.png)



z轴满足 z = n 或 z = f 时没有任何变化，得到如下关系

![image-20240211211509741](Images\image-20240211211509741.png)

![image-20240211212045014](Images\image-20240211212045014.png)



得到：
$$
M_{persp \rightarrow ortho} = \begin{pmatrix} n & 0 & 0 & 0 
\\ 0 & n & 0 & 0 \\ 0 & 0 & n+f & -nf\\ 0 & 0 & 1 & 0 \\ \end{pmatrix}
$$


最终的透视投影矩阵为：
$$
M_{persp} = M_{ortho} M_{persp \rightarrow ortho}
$$
**Tip:**

经过 $ M_{persp \rightarrow ortho} $ 后 n 与 f 平面之间的点会被**往 n 平面推**，推导如下：

选取中间平面任意一点：
$$
\begin{pmatrix}  x \\ y \\ \frac{n+f}{2} \\ 1 \\ \end{pmatrix}
$$
左乘 $ M_{persp \rightarrow ortho} $ 得到如下结果:
$$
\begin{pmatrix}  nx \\ ny \\ \frac{(n+f)^2}{2} - nf \\ \frac{n+f}{2} \\ \end{pmatrix} == \begin{pmatrix}  \frac{2nx}{n+f} \\ \frac{2ny}{n+f} \\ n+f - \frac{2nf}{n+f} \\ 1 \\ \end{pmatrix}
$$
其中新的Z轴坐标 $n+f - \frac{2nf}{n+f} == \frac{2n^2 + 2f^2}{2(n+f)}$ ， 而原Z轴坐标为 $\frac{n+f}{2} == \frac{n^2 + f^2 + 2nf}{2(n+f)}$， 所以只需要判断 $2n^2 + 2f^2$ 和 $ n^2 + f^2 + 2nf $ 大小即可。

两边同减 $n^2 + f^2 + 2nf$ 得到  $ (n-f)^2$ 和 0， 所以新的Z轴更大，而在右手坐标系中，n > f， 所以**比之前更接近n平面**



#### 3.2.2 Fov and Aspect ratio

FovY(垂直视场角)， Aspect ratio(宽高比)

![image-20240211221402616](Images\image-20240211221402616.png)



![image-20240211222113314](Images\image-20240211222113314.png)
