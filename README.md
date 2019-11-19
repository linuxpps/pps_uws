# pps_uws
基于libuv的uws，支持websocket socket http服务

# Linux安装依赖
sudo yum install -y libuv-devel libuv

sudo yum install -y centos-release-scl

sudo yum install -y devtoolset-8-toolchain

```
# new create bash environment
#scl enable devtoolset-8 bash

# current bash environment
source /opt/rh/devtoolset-8/enable

source scl_source enable devtoolset-8
```