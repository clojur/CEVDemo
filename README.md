# CryEngine5.5 调库测试

## 它是什么？

这个工程是把CryEngigne当第三方库调用的小例子，是一个纯粹的Visual Studio工程，通过它你可以明白CryEngine大体架构和内部运行逻辑。相信通过大家不断努力后可以实现CryEngine像OSG、OGRE等类似的第三方库来使用。

## 如何使用？

1. 程序入口WinMain所在源文件为mainTest.cpp源文件内；
2. 把VS工程项目包含的CryEngine头文件路径改了指向自己的CryEngigne头文件路径；
3. 生成的可执行文件（exe）要放到CryEngine的Bin目录里面，直接在VS项目配置里面设置输出目录即可；
4. VS工程项目的调试参数设置为 -project "your game project path"， 例如我的为:
    -project "E:\3DEngine\CryEngine\CEProject\MyBlank\Game.cryproject"
 
 ## 目前存在缺点
 1. 渲染循环起来会造成内存泄露
 2. 只是简单的刷了一下背景并未尝试太丰富的场景
    
 ## 运行效果：
![Image text](https://github.com/clojur/CEVDemo/blob/master/image/CEVDemoRun.png)
