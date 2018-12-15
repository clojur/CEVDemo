# CEVDemo
CryEngine5.5 调库测试

如何使用？

1. 把VS工程项目包含的CryEngine头文件路径改了指向自己的CryEngigne头文件路径；
2. 生成的可执行文件（exe）要放到CryEngine的Bin目录里面，直接在VS项目配置里面设置输出目录即可；
3. VS工程项目的调试参数设置为 -project "your game project path"， 例如我的为:
    -project "E:\3DEngine\CryEngine\CEProject\MyBlank\Game.cryproject"
