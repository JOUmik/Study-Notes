# Transformation

[TOC]

**MVP**分别代指三种变换（Model, View, Projection），变换的最终目的是把三维空间中的物体变为二维空间中的一张照片

## 1. Model transformation(模型变换)

> 如果一个矩阵的转置等于它的逆，这个矩阵为正交矩阵。

对于二维旋转矩阵有这样的性质:

如下是二维旋转矩阵的公式：
$$
R_{\theta} = \begin{pmatrix} cos\theta & -sin\theta \\ sin\theta & cos\theta \\ \end{pmatrix}
$$
如果想要旋转 $-\theta$ 角则将 $-\theta$ 带入得到：
$$
R_{-\theta} = \begin{pmatrix} cos\theta & sin\theta \\ -sin\theta & cos\theta \\ \end{pmatrix}
$$
发现 $R_{-\theta}$ 与 $R_\theta$ 的转置一样，即：
$$
R_{-\theta} == R_{\theta}^T
$$
而旋转 $-\theta$ 角本身相当于旋转 $\theta$ 角的逆，即：
$$
R_{-\theta} == R_{\theta}^{-1}
$$
所以对于二维旋转矩阵，它的转置等于它的逆，所以**二维旋转矩阵是一个正交矩阵**



> **模型变换是相对于自己坐标系的变换**

### 1.1 Scale(缩放)

$$
S_{(s_x, s_y, s_z}) = \begin{pmatrix} s_x & 0 & 0 & 0 
\\ 0 & s_y & 0 & 0 \\ 0 & 0 & s_z & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$



### 1.2 Translation(平移)

$$
T_{(t_x, t_y, t_z}) = \begin{pmatrix} 1 & 0 & 0 & t_x 
\\ 0 & 1 & 0 & t_y \\ 0 & 0 & 1 & t_z\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

### 1.3 Rotation(旋转)

> 旋转要拆成绕x ,y ,z轴旋转三种矩阵

$$
R_{x}(\alpha) = \begin{pmatrix} 1 & 0 & 0 & 0
\\ 0 & cos\alpha & -sin\alpha & 0 \\ 0 & sin\alpha & cos\alpha & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

$$
R_{y}(\alpha) = \begin{pmatrix} cos\alpha & 0 & sin\alpha & 0
\\ 0 & 1 & 0 & 0 \\ -sin\alpha & 0 & cos\alpha & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

$$
R_{z}(\alpha) = \begin{pmatrix} cos\alpha & -sin\alpha & 0 & 0
\\ sin\alpha & cos\alpha & 0 & 0 \\ 0 & 0 & 1 & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

可以发现绕y轴旋转的参数和绕x， z轴不一样，原因如下：

- 假定逆时针旋转为正向旋转；
- x为轴旋转时，y是横坐标，z是纵坐标；
- z为轴旋转时，x是横坐标，y是纵坐标；
- 但是以y为轴旋转时，z是横坐标，x是纵坐标，参照二维旋转矩阵，此时正向旋转对于以x作为横坐标，z作为纵坐标的坐标系是反方向的，所以参数和正向旋转的逆一样

<img src="Images\image-20240413181617016.png" alt="image-20240413181617016" style="zoom:50%;" />

#### 1.3.1 Euler angle(欧拉角)

![image-20240413182255677](Images\image-20240413182255677.png)

任何旋转都可以用绕x轴，绕y轴，绕z轴旋转的组合



****

***万向节死锁***

对于欧拉角，参照公式可以看出自带旋转顺序，即绕着z轴旋转不会影响x， y轴，绕着y轴旋转不会影响x轴，其和平衡环架概念上相同：

<img src="Images\image-20240413184037888.png" alt="image-20240413184037888" style="zoom:50%;" />

图中蓝色的环是绕z轴旋转，红色的环是绕y轴旋转，绿色的轴是绕x轴旋转，与欧拉角公式产生对应。

假设让y轴旋转90度，此时会发现蓝色的环和绿色的环重合在一起了，此时不管绕x轴即绿色的环还是绕z轴即蓝色的环结果都是一样的，这就是万向节死锁，即某一轴的旋转失灵，用同样的方法推导可知**旋转x轴90度也会出现这一问题如下图所示**。出现这一问题的原因是欧拉角公式的设计，因为x轴的变换会影响到y轴，z轴，y轴的变换会影响到z轴

<img src="Images\image-20240413191445981.png" alt="image-20240413191445981" style="zoom:50%;" />

现在再从公式角度验证一下：（以y轴旋转90度为例，x轴旋转90度同理可证）

假设x轴旋转 $\alpha$ 度，y轴旋转90度，z轴旋转 $\beta$ 度
$$
R_{(x,y,z)}(\alpha, \frac{\pi}{2}, \beta) = R_{(x)}(\alpha)R_{(y)}(\frac{\pi}{2})R_{(z)}(\beta)
$$

$$
R_{(x,y,z)}(\alpha, \frac{\pi}{2}, \beta) = \begin{pmatrix} 1 & 0 & 0 & 0
\\ 0 & cos\alpha & -sin\alpha & 0 \\ 0 & sin\alpha & cos\alpha & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}\begin{pmatrix} cos\frac{\pi}{2} & 0 & sin\frac{\pi}{2} & 0
\\ 0 & 1 & 0 & 0 \\ -sin\frac{\pi}{2} & 0 & cos\frac{\pi}{2} & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}\begin{pmatrix} cos\beta & -sin\beta & 0 & 0
\\ sin\beta & cos\beta & 0 & 0 \\ 0 & 0 & 1 & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

$$
R_{(x,y,z)}(\alpha, \frac{\pi}{2}, \beta) = \begin{pmatrix} 0 & 0 & 1 & 0
\\ cos\alpha·sin\beta + sin\alpha·cos\beta& cos\alpha·cos\beta - sin\alpha·sin\beta & 0 & 0 \\ sin\alpha·sin\beta - cos\alpha·cos\beta & cos\alpha·sin\beta + sin\alpha·cos\beta & 0 & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

$$
R_{(x,y,z)}(\alpha, \frac{\pi}{2}, \beta) = \begin{pmatrix} 0 & 0 & 1 & 0
\\ sin(\alpha + \beta)& cos(\alpha + \beta) & 0 & 0 \\ -cos(\alpha + \beta) & sin(\alpha + \beta) & 0 & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$

同时，将y轴旋转90度，z轴旋转 $\alpha+\beta$ 度有公式：
$$
R_{(y)}(\frac{\pi}{2})R_{(z)}(\alpha+\beta) = \begin{pmatrix} 0 & 0 & 1 & 0
\\ sin(\alpha + \beta)& cos(\alpha + \beta) & 0 & 0 \\ -cos(\alpha + \beta) & sin(\alpha + \beta) & 0 & 0\\ 0 & 0 & 0 & 1 \\ \end{pmatrix}
$$
所以此时
$$
R_{(x,y,z)}(\alpha, \frac{\pi}{2}, \beta) == R_{(y)}(\frac{\pi}{2})R_{(z)}(\alpha+\beta)
$$
可以看到x轴的功能失效了



为了避免万向节死锁的问题，就要设计一种新的旋转公式，这种旋转公式应该x，y，z轴的旋转相互独立。这就引出了**四元数（Quaternion）**



#### 1.3.2 四元数（Quaternion）

用四维去考虑三维，可以避免欧拉角的万向节死锁问题并能保证旋转的平滑插值



## 2. View/Camera transformation(视图变换)

保证相机和物体的相对位置

想象一下拍照的过程：

- 找到一个好地方并摆好Pose（该步骤可以类比模型变换，在合适的地方摆好模型并且完成变换）
- 找到一个好的角度来放置相机（该步骤可以类比视图变换）
- 拍照！（该步骤可以类比投影变换，将三维空间投影到二维的照片上去）



思考在UE中放置一个摄像机，我希望最终运行时呈现的是该摄像机看到的画面。在场景中摄像机的坐标是世界坐标，如果希望知道物体与摄像机的关系，最好的方法就是站在摄像机的位置去看，那么此时可以类比把场景中所有物体，包括摄像机**进行平移**，使得摄像机的坐标是（0，0，0），之后再**旋转**场景中所有物体，包括摄像机，使得摄像机朝向Z轴负方向，向上方向为y轴正方向，此时就完成了视图变换



**问：这样不会导致相对位置出现变换吗**

答：不会，因为所有物体视作一个整体一起移动，相对位置不会发生变化

![image-20240413204942031](Images\image-20240413204942031.png)

![image-20240413205342278](Images\image-20240413205342278.png)

旋转矩阵是正交矩阵，它的逆等于它的转置

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



## 4. Viewport transformation(视口变换)

> **Canonical Cube to Screen**

Games101中Pixel的原点在左下角（UE设定是左上角为原点），而Canonical Cube是[-1, 1]范围的，要把它映射到对应Pixel上

<img src="E:/学习笔记/Study-Notes/图形学/Images/image-20240211223353833.png" alt="image-20240211223353833" style="zoom:67%;" />

<img src="E:/学习笔记/Study-Notes/图形学/Images/image-20240211223629128.png" alt="image-20240211223629128" style="zoom:67%;" />



为了将Canonical Cube映射到屏幕空间上，要使用**视口变换**

<img src="Images\image-20240211223736910.png" alt="image-20240211223736910" style="zoom: 67%;" />
