[English](BUILD.md) | **简体中文**

# AIR6 开源版构建说明

本仓库为全新历史的公开源码准备版，不含生产私钥、SDK 签名工具、签发器或已签名固件。原私有发行包不能由这个副本直接复现。

安装 OpenWrt 常规构建依赖后执行：

```sh
./scripts/feeds update -a
./scripts/feeds install -a
make defconfig
make -j$(nproc)
```

目标为 mediatek/mt6990/softbank_air6。APK 签名使用构建者自己生成的密钥；原 APK 私钥未随仓库提供。安全启动所需的厂商 boot 签名流程必须由维护者独立提供，本仓库不包含相关工具或密钥。普通构建产物不是可直接用于 AIR6 安全启动的最终发行包。

Web 更新仍强制验签，使用前须配置维护者自己的 update.pub。缺少公钥时会拒绝更新。生产设备信任的原密钥没有公开。

原仓库已有许可文件保持不变；第三方二进制驱动/固件不因本次导出而改变许可。
