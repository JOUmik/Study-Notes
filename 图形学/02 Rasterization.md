# Rasterization(光栅化)

[TOC]

## 1. Introduction

在经过MVP后物体和摄像机被摆放在了正确的位置，下一步就是把物体画出来，这就是rasterization要做的事情， 屏幕就是光栅成像设备，所以光栅化和pixel有关



## 2. Canonical Cube to Screen

Games101中Pixel的原点在左下角（UE设定是左上角为原点），而Canonical Cube是[-1, 1]范围的，要把它映射到对应Pixel上

![image-20240211223353833](Images\image-20240211223353833.png)

![image-20240211223629128](Images\image-20240211223629128.png)



### 2.1 Viewport transformation(视口变换)

为了将Canonical Cube映射到屏幕空间上，要使用视口变换

![image-20240211223736910](Images\image-20240211223736910.png)

## 3. Triangles - Fundamental Shape Primitives

![image-20240211230155822](Images\image-20240211230155822.png)



## 4. Sample(采样)

在图形学中，采样是一种关键思想，在光栅化这部分，采样用来确定屏幕空间中某个像素对应的RGB应该是多少

![image-20240212105753273](Images\image-20240212105753273.png)

在不同的像素中心，确认其是 1 还是 0 ， 1 代表在三角形内， 0 代表在三角形外

![image-20240212110012966](Images\image-20240212110012966.png)

判断一点是否在三角形内的方法是叉乘，使用右手螺旋定则，如果方向一致则在三角形内，否则在三角形外

![image-20240212110417486](Images\image-20240212110417486.png)

**Tip:**

当某一像素点处于三角形1和三角形2的共边上，那么该像素点的规则自己定义（OpenGL是**记上不记下，记左不记右**）



### 4.1 Bounding Box(包围盒)

如果到此结束，那么每个三角形都要遍历一遍整个屏幕空间，为了降低这种巨大的时间和性能开销，包围盒这一概念随即出现

#### 4.1.1 Axis-aligned Bounding Box(AABB)

![image-20240212111824443](Images\image-20240212111824443.png)

 

## 4. Antialiasing(抗锯齿)(反走样)

直接采样后图像会出现如图所示的锯齿，解决锯齿问题是图形学非常重要的一大课题

![image-20240212112603383](Images\image-20240212112603383.png)
