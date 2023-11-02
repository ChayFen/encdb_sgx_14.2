# Enc-db

## 1. 安装

### 1.1 依赖项

```shell
yum -y install make automake libtool cmake
yum -y install openssl-devel boost-devel

# 安装gmp依赖
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz && tar -xf gmp-6.2.1.tar.lz && cd gmp-6.2.1 && CFLAGS="-fPIC" ./configure && make -j4 && make install 

# 安装protobuf依赖
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git submodule update --init --recursive
./autogen.sh
./configure
 make
 make check
 sudo make install
 sudo ldconfig # refresh shared library cache.
```

### 1.2 PostgreSQL安装

#### 1.2.1 本地部分

目前仅支持PostgreSQL 9.2.4

+ 下载本项目源代码到`postgresql-9.2.4/src/interfaces/libpq/`下

  ```shell
  git clone https://gitee.com/lihuixidian/enc-db.git postgresql-9.2.4/src/interfaces/libpq/encryptsql
  ```

+ 运行安装脚本

  ```shell
  cd postgresql-9.2.4/src/interfaces/libpq/encryptsql && sh setup.sh
  ```

+ 切换到```postgresql```主目录下，执行编译命令

  ```shell
  ./configure && make -j && sudo make install
  ```

#### 1.2.2 服务器部分

将目录`postgresql/src/interfaces/libpq/encryptsql/build/lib`下的`libudf.so`复制到服务器`pg_config --pkglibdir`中，即psql的lib文件夹，并执行`createudf.sql`在服务器上创建UDF函数。

### 1.3 OpenGauss安装

#### 1.3.1 本地部分

+ 下载本项目源代码到`postgresql-9.2.4/src/interfaces/libpq/`下

  ```shell
  git clone https://gitee.com/lihuixidian/enc-db.git postgresql-9.2.4/src/interfaces/libpq/encryptsql
  ```

+ 运行安装脚本

  ```shell
  cd postgresql-9.2.4/src/interfaces/libpq/encryptsql && sh setup_gs.sh
  ```

+ 切换到```opengauss```主目录下，执行编译命令

  ```shell
  sh build.sh
  ```

  具体opengauss的编译请参考: <https://gitee.com/opengauss/openGauss-server>

#### 1.3.2 服务器部分

将目录`postgresql/src/interfaces/libpq/encryptsql/build/lib`下的`libudf.so`复制到服务器OpenGauss的lib文件夹，并执行`createudf.sql`在服务器上创建UDF函数。

### 1.3 编译参数说明

编译的主要选项在`encryptsql/CMakeLists.txt`中，按照需求以及注释内容进行相应的配置即可。

+ `USE_ENCRYPT` 是否开启加密功能
+ `TIME_PORTRAIT` 是否开启运行时间采集功能
+ `USE_LRU` 是否开启缓存功能
+ `USE_SGX` 是否开启SGX功能
+ `SGX_MODE` 配置SGX的运行模式，ON为硬件模式，OFF为模拟模式
+ `SGX_ORE` 是否将所有的ORE操作用SGX计算
+ `SGX_SDK`变量为SGX SDK的路径

## 2. 配置

目前配置文件默认为`/etc/encryptsql/config.json`,可根据需要自行配置。

+ `common.EnclavePath` 为Enclave共享库所在的路径。
+ `common.PlainMasterKeyPath` 为明文主密钥的路径，只在客户端使用。
+ `common.SealedMasterKeyPath` 为Enclave Seal的主密钥路径，只在服务端使用。
+ `common.LogPath` 为日志路径。 
+ `RA`字段中的选项为SGX远程认证相关的配置。

## 3. 使用

目前编译的`libpq.so`为加密版本，可通过`ldd libpq.so`查看其是否链接了`libencryptsql.so`。将应用程序链接的`libpq.so`替换即可实现加密功能。
加密库自带了一些管理命令，其中有：
+ `enc`命令
  + `enc status` 查看是否开启加密，1 表示开启，0 表示关闭。
  + `enc on` 设置开启加密功能，如果编译时未开启加密，则此方法无效。
  + `enc off` 暂时关闭加密功能。
+ SQL过滤
  
  在map.json中会设置一系列需要跳过加密的SQL语句，可根据需要自行添加。
