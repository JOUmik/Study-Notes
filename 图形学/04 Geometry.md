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

![image-20240215120105399](Images\image-20240215120105399.png)

## 2. “Explicit” Representations of Geometry(几何的显式表示)

<img src="Images\image-20240215112907369.png" alt="image-20240215112907369" style="zoom:80%;" />

**优点**：采样很简单（ 获得表面的所有点很简单）

<img src="Images\image-20240215113050570.png" alt="image-20240215113050570" style="zoom:80%;" />

**缺点**：判断点在几何体的内外很困难

<img src="Images\image-20240215113224274.png" alt="image-20240215113224274" style="zoom:80%;" />

**Tip:** 隐式几何表示和显式几何表示的优缺点相反，需要根据需求选取合适的几何表达方法

