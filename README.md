# WebServer_C
基于C语言实现一个简易的Web服务器，支持Http和Https（基于OpenSSL的TLS协议）

## 开发环境：
* 开发语言：`C （部分测试网页涉及HTML+CSS内容，不影响功能）`
* 开发环境：`Ubuntu 20.04， gcc V9.3.0, OpenSSL 1.1.1`

## 功能说明： 
* 对HTTP、HTTPS的支持
* Centent-Type的支持（对应Chunked模式）
* 对分块传输的支持
* 文件传输
* 注：在HTTP层，仅完成部分头部协议识别，传输层与SSL层完成了封装，自适应HTTPS与HTTP数据报的传送

## 环境复现
```Bash
sudo apt-get update
sudo apt-get install openssl
sudo apt-get install libssl-dev
sudo apt-get install gcc
```

## 二进制构建
```Bash
make

# 清理项目
make clean
```

## 附言
关于如何创建测试用SSL证书，可参考我的[CSDN博客](https://blog.csdn.net/qq_36290650/article/details/109364017)
