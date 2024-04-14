# Unreal 5 操作笔记

[TOC]



## 1. 基本操作

**打开Content Drawer**：Ctrl + 空格键

**选择场景物体对应的文件**：在场景中选择（多选）物体后按Ctrl + B

**测量大小**：在非透视视图中按住鼠标中键可以测量物体大小， 100 unreal units = 1m

**preview mesh**：按住 "\\" 键查看preview mesh来判断场景物体大小是否合适，可以在“编辑器偏好设置”中修改preview mesh使用的模型 

**轮廓**：G，显示或隐藏所有好用的工具（轮廓，网格，枢轴，图标等）

**玩家模式**：运行后点击F8开关该模式



## 2. 材质

**迁移节点**：按住Ctrl点击节点进行拖拽，可以将该节点连的所有线移到其他节点

**改变材质预览光照位置**：按住L + 鼠标左键

**公共节点**：选择想要放在一起的节点，点击C键



## 3. 网格体绘制模式

**改变绘制大小**：参数栏调整或使用” [ “、 ” ] “键来缩小或放大

**擦除颜色**：按住shift移动鼠标进行擦除

**切换绘制/擦除颜色**：X键



## 4. 设置控制视角的ITEM

![](images\1.png)

选择用来控制视角的ITEM，搜索Auto Possess Player，将自动控制玩家设置为玩家0，本地多人的游戏有另外的设置方法，注意所有item里只能有一个被设置为玩家0



## 5. 使用AddMovementInput()注意事项

![](images/2.png)

对于**Pawn类**的蓝图，必须手动添加FloatingPawnMovement组件后AddMovementInput才会生效；

对于**Character类**的蓝图，创建时自带UCharacterMovementComponent，所以不需要手动添加；



## 6. 为AI Controller启用Movement Controller

![image-20240105230849757](images\3)

务必打开MoveComponent里的“**在无控制器的情况下运行物理**”，否则重力，运动等功能将失效



## 7. Groom毛发不显示（启动groom绑定后 或者导入第三方角色带头发 头发不显示）

UE5 Groom毛发设置绑定后 消失不见问题 或者导入第三方角色 涉及到毛发不显示

支持 Groom 插件要事先开启

第一，需要确定 项目设置-SupportCompute Skin Cache 为**开启**状态

![image-20240127214019703](images\4.png)

第二，确定skeltal mesh 属性页 Skin Cache Uage 为 **Enable** 的状态

![image-20240127214122388](images\5.png)

修改以上设置后需要**重启引擎**



## 8. Multithreading

动画蓝图可以使用绑定了相同骨骼的其他动画蓝图，当每个动画蓝图都在Update Animation节点中做了大量的操作时，很可能会像Github一样出现冲突，为了解决这个问题，可以使用UE5自带的multithreading功能，该功能对应的函数叫做 **Blueprint Thread Safe Update Animation**, 如下图所示：

![image-20240301150637672](images\image-20240301150637672.png)

将需要赋值的变量或者需要使用的函数通过 **Property Access** 获得：

![image-20240301150800933](images\image-20240301150800933.png)



假设所有的变量在更新前称为状态A，更新后称为状态B，通过这种更新方式，所有的动画蓝图同时更新，每次更新都使用状态A的变量，当所有动画蓝图更新一次后，再将所有变量都更新为状态B，以此来解决冲突问题



## 9. Nav Mesh Bounds Volume

按P显示Nav区域，所有内置的Nav功能只有在该Volume内才会生效



## 10. 行为树

**黑板：**给NPC的行为规定一个具体的指导的目标

使用行为树的方法：将行为树放在**AI Controller**里，具体方法是在begin play中调用”运行行为树“再绑定需要的行为树

三种不同的选择节点：

**Selector：**从左到右执行第一个可以成功执行的节点**一直执行**

**Sequence：**强制性的按照从左到右的顺序来执行，当执行某节点失败时再重新从左到右执行

**Simple Parallel：**分为主任务和在后台运行的后台任务，二者同时运行 
