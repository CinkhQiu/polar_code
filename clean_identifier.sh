#!/bin/bash

# 切换到当前目录
cd "$(dirname "$0")"

# 查找并删除 include/Eigen 目录下所有以 .Identifier 结尾的文件
find ./include/Eigen -type f -name '*.Identifier' -exec rm -f {} +

echo "All .Identifier files under include/Eigen have been deleted."