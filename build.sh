#!/bin/bash

# 获取项目根目录
PROJECT_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
echo "项目根目录为: $PROJECT_ROOT"

# 创建并进入 build 目录
if [ ! -d "$PROJECT_ROOT/build" ]; then
  echo "创建 build 目录..."
  mkdir "$PROJECT_ROOT/build"
else
  echo "build 目录已存在"
fi

# 清理 build 目录
echo "清理 build 目录..."
rm -rf "$PROJECT_ROOT/build/*"

# 进入 build 目录
cd "$PROJECT_ROOT/build"

# 检查编译模式并配置 CMake
if [ "$1" == "O3" ]; then
  echo "启用 Release 模式和 O3 优化..."
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="-O3" ..
else
  echo "启用 Debug 模式..."
  cmake -DCMAKE_BUILD_TYPE=Debug ..
fi

# 运行 make
echo "运行 make ..."
make -j$(nproc)

# 运行测试
echo "运行测试..."
ctest --output-on-failure
