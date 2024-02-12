# Unreal 2D 游戏开发笔记

[TOC]



## 1.保证角色永远不会被背景遮挡

- 调整角色sprite component的半透明排序优先级高于背景，数值越大优先级越高

<img src="image\1.png" alt="1" style="zoom:80%;" />



- 将角色使用的材质改为TranslucentUnlitSpriteMaterial

​                                                       <img src="image\2.png" alt="1" style="zoom:80%;" /> 



## 2.修改Project Setting

- 关闭动态模糊

<img src="image\3.png" alt="image-20240104222321453" style="zoom: 80%;" />



- 关闭自动曝光

<img src="image\4.png" alt="image-20240104222432899" style="zoom:80%;" />



- 切换抗锯齿选项为FXAA或无

<img src="image\5.png" alt="image-20240104222659998" style="zoom: 67%;" />



## 3.修改纹理格式

- 选择纹理图片，右键 -> Sprite操作 -> **应用Paper2D纹理设置**

<img src="image\6.png" alt="image-20240105014447942" style="zoom:80%;" />



- 将纹理的过滤方式从**默认**改为**最近**

<img src="image\7.png" alt="image-20240105014749728" style="zoom:80%;" />

## 4.GameMode设置

将默认Pawn类设为None

<img src="image\8.png" alt="image-20240105015302628" style="zoom: 67%;" />



## 5.Sprite Sheet处理

1. 选中要处理的Sprite Sheet -> 右键 -> Sprite操作 -> 提取Sprite

<img src="image\9.png" alt="image-20240105130940789" style="zoom: 67%;" />



2. 提取模式选择网格(grid)

<img src="image\10.png" alt="image-20240105131233747" style="zoom: 67%;" />



3. 按照Sprite Sheet的row和column调整单元宽度和高度，提取Sprite

<img src="image\11.png" alt="image-20240105131451860" style="zoom:67%;" />



## 6.创建纸片图像序列

多选要使用的Sprite，右键选择**创建图像序列**，创建后打开调整参数

<img src="image\12.png" alt="image-20240105131944130" style="zoom:67%;" />



## 7. 修改Sprite大小

​                                                       <img src="image\13.png" alt="image-20240106143907070" style="zoom:67%;" />  

在项目设置中，将默认逐虚幻单位像素修改为合适的数值，数值越小Sprite越大，**（只会影响之后导入的资产！！）**



## 8. 瓦片集

<img src="image\14.png" alt="image-20240106212710981" style="zoom:67%;" />

如上图所示创建瓦片集（Tile Set），在瓦片集中设置碰撞体积，设置完成后创建瓦片贴图（Tile Map）

<img src="image\15.png" alt="image-20240106213412145" style="zoom: 80%;" />

**为了避免出现瓦片重叠**，可以选择瓦片集 -> 右键 -> 限定瓦片薄片纹理

![image-20240106221617918](image\16.png)