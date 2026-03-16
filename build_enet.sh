#!/bin/sh

# 仅回滚 enet 目录，避免影响其他未提交的修改（如果需要全量回滚可调整）
svn revert -R ./InGame/source/skynet/3rd/enet

# 更新 enet 代码
svn up ./InGame/source/skynet/3rd/enet

# 进入 skynet 目录
cd ./InGame/source/skynet

# 执行增量编译 (make linux)
# enet 的源码直接链接在 egate.so 中，这会重新生成 egate.so
echo "Start building skynet (egate.so with enet)..."

# 强制删除 egate.so 以触发重新编译
rm -f ../../bin/cservice/egate.so

make linux

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "Build success!"
    # 如果需要自动提交，可以取消下面行的注释
     svn commit ../../bin/cservice/egate.so -m "update egate.so (enet)"
else
    echo "Build failed!"
    exit 1
fi
