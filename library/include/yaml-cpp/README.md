# yaml-cpp

此文件夹内是从 [yaml-cpp](https://github.com/jbeder/yaml-cpp) 的Release 0.8.0 版本中获得的代码。

`yaml-cpp`原项目使用CMAKE构建，但为了兼容本项目，Yayin Story Studio另行将其转换为了VS2022项目并另外进行构建。由于无需对该库进行更改，因此该库的项目未出现在解决方案中，仅在开发者本机编译一次即可。编译后产生的二进制文件附带在本项目的仓库中。

Yayin Story Studio 仅对编译配置进行了修改，其他源码均按原样呈现，但不保证提供与原项目完全相同的功能或性能。

该项目采用MIT许可证授权。以下是该项目的许可证信息：

# MIT License

```
Copyright (c) 2008-2015 Jesse Beder.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```