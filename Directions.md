# 常用命令指引

## CMake
- 常规指令

  ```bash
  # 1. 设置编译目录与编译选项, 并进行配置
  cmake -B build-dir -DCMAKE_BUILD_TYPE=Release 
  # 2. 编译目标文件夹, 并设置并行数量
  cmake --build build-dir --parallel 8
  # [3. 安装/测试编译后的程序]
  [sudo] cmake --build build-dir --target install/test [--prefix=...]
  ```
  
- 其他常见选项

  - `-DCMAKE_INSTALL_PREFIX=/opt/xxx`
  - `-G 'Unix Makefiles'` or `Ninja`

- 缓存文件: `build-dir/CMakeCache.txt`

