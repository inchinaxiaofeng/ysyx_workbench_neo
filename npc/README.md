# New Processing Core

首先, 无论如何我都要换个名字, 太糟糕了.

这个目录下使用 `Kconfig` 完成配置.
但是与 `NEMU` 不同的是, 这个目录依赖 `kconfiglib` 工作.

## 环境依赖

```bash
pip install kconfiglib

menuconfig # 执行生成.config(保持默认)

python3 sync_config.py # 生成autoconfig.h
```

需要注意的是, `menuconfig` 的运行结果会直接生成在当前目录下
