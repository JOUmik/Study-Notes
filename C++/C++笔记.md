# C++笔记

[TOC]



## 1. stack，queue等使用push和emplace的区别

**push**：push必须为其先创造存储对象再复制变量

**emplace**：emplace可以传入存储对象需要的变量，它会自行调用构造函数

~~~c++
//假设栈内的数据类型是data
class data {
  int a;
  int b;
public:
  data(int x, int y):a(x), b(y) {}
};

data d(1,2);

S.push(d) 
S.emplace(d);

//在传入时候构造对象
S.push(data(1,2));
S.emplce(data(1,2));

//emplace可以直接传入构造对象需要的元素，然后自己调用其构造函数
S.emplace(1,2)
~~~



## 2. 优先队列

[c++优先队列(priority_queue)用法详解_c++ 优先队列-CSDN博客](https://blog.csdn.net/weixin_36888577/article/details/79937886)



## 3. Lambda表达式

[C++ Lambda表达式的完整介绍 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/384314474)

海王计划16



## 4. decltype

[C++11之decltype类型推导（使用场景、推导四规则、cv限定符）_c++ decltype 如何判断相等-CSDN博客](https://blog.csdn.net/qq_45254369/article/details/127372898)



## 5. Sort

[std::sort()的用法 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/615321693)

[C++ sort()排序详解-CSDN博客](https://blog.csdn.net/qq_41575507/article/details/105936466)



## 6. int和string相互转化

[C/C++中string和int相互转换的常用方法_string转int-CSDN博客](https://blog.csdn.net/albertsh/article/details/113765130)



## 7. 求最大公约数

辗转相除法

~~~c++
int gcd(int a, int b) {
    return !b ? a : gcd(b, a % b);
}
~~~



## 8. rand() 和 srand()

[C语言随机数：rand()和srand(time(NULL))的使用-CSDN博客](https://blog.csdn.net/billhao_/article/details/86660588)



## 9. unordered_multimap

[unordered_multimap的介绍及使用-CSDN博客](https://blog.csdn.net/gp1330782530/article/details/106433122)



## 10.拓扑排序

[图文详解面试常考算法 —— 拓扑排序 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/135094687)



## 11. multiset

[multiset用法总结-CSDN博客](https://blog.csdn.net/sodacoco/article/details/84798621)



## 12. map

[C++的map排序_c++ map排序-CSDN博客](https://blog.csdn.net/chengqiuming/article/details/89816566)



## 13. 前置声明

[【C++】C++中前置声明的应用与陷阱_前置生命如何使用-CSDN博客](https://blog.csdn.net/qingzhuyuxian/article/details/92157301)



## 14. 二分查找模板

[二分查找模板总结-CSDN博客](https://blog.csdn.net/charlsonzhao/article/details/124063879)

[二分查找——三种模板（C++版本）_c++二分查找模板-CSDN博客](https://blog.csdn.net/qq_34732729/article/details/100666158)



## 15. Substr

