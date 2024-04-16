# Materials and Appearances

[TOC]



材质和光线是紧密联系起来的，当说到材质也就是说光线应该如何作用



**渲染方程中的BRDF就和物体的材质息息相关：**
$$
Material == BRDF
$$


## 1. Diffuse/Lambertian Material（漫反射材质）

![image-20240414213003390](Images\image-20240414213003390.png)

对于一个将所有能量反射回去的纯漫反射材质，它的BRDF为
$$
f_r= \frac{1}{\pi}
$$
对于无法将全部能量反射出去的漫反射材质，引入 **反射率 $\rho$（albedo）**的概念，范围在 $[0, 1]$ 之间，此时的BRDF更新为
$$
f_r= \frac{\rho}{\pi}
$$

## 2. Gloosy Material

![image-20240414214041707](Images\image-20240414214041707.png)



## 3. Ideal reflective / refractive material

![image-20240414214221737](Images\image-20240414214221737.png)



## 4. Perfect Specular Relfection（镜面反射）

<img src="Images\image-20240414214456698.png" alt="image-20240414214456698" style="zoom:50%;" />
$$
入射角=反射角
$$
![image-20240415053136678](Images\image-20240415053136678.png)



## 5. Specular Refraction（折射）

![image-20240415053352597](Images\image-20240415053352597.png)

![image-20240415055018280](Images\image-20240415055018280.png)

![image-20240415055111442](Images\image-20240415055111442.png)

当入射介质的折射率 $\eta_i$ 大于入射介质的折射率 $\eta_t$ 就**有可能导致没有折射的发生**，这就是**全反射现象**

![image-20240415055422106](Images\image-20240415055422106.png)

折射的模式使用的是**BTDF（双向透射分布函数）**，而BRDF和BTDF可以统称为**BSDF（双向散射分布函数）**

## 6. Fresnel Reflection / Term（菲涅尔项）

![image-20240415055952782](Images\image-20240415055952782.png)

**菲涅尔项：绝缘体**

![image-20240415060400639](Images\image-20240415060400639.png)

**菲涅尔项：导体**

![image-20240415063645963](Images\image-20240415063645963.png)

![image-20240415093106933](Images\image-20240415093106933.png)



## 7. Microfacet Material（微表面模型）

<img src="Images\image-20240415093351637.png" alt="image-20240415093351637" style="zoom: 50%;" />

当我离得足够远时，微小的东西就看不到了，看到的是这些微小的东西整体对光的一个作用

**微表面模型的假设：**

​	物体表面是粗糙凹凸不平的，但是从远处看到的是一整块平坦的表面，只是整体来看材质是粗糙的。**即从远处看到的是呈现的材质效果，从近处看到的是导致这些效果的几何**。每一个微表面都是一个微小的镜面发生镜面反射

![image-20240415093938906](Images\image-20240415093938906.png)

![image-20240415094044337](Images\image-20240415094044337.png)

![image-20240415094727807](Images\image-20240415094727807.png)

## 8. PBR

[猴子都能看懂的PBR - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/33464301)



## 9. Isotropic/Anisotropic Materials 各向同性/各向异性材质

![image-20240415095856204](Images\image-20240415095856204.png)

各向同性：微表面不存在方向性；

各向异性：微表面存在明显的方向性；

![image-20240415100147748](Images\image-20240415100147748.png)

从BRDF角度来看，如果入射角和出射角相对位置不变，整体旋转得到的BRDF值相等则是各向同性的，如果不相等即**BRDF值与绝对位置有关则是各项异性的**



## 10. BRDF的性质

![image-20240415110717308](Images\image-20240415110717308.png)

![image-20240415110856833](Images\image-20240415110856833.png)

![image-20240415111056561](Images\image-20240415111056561.png)

