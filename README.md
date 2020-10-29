# WebServer_C
基于C语言实现一个简易的Web服务器，支持Http和Https（基于OpenSSL的TLS协议）

## 开发环境：
开发语言：C （部分测试网页设计HTML+CSS内容，不影响功能）
开发环境：Ubuntu 20.04， gcc V9.3.0, OpenSSL 1.1.1

## 功能说明： 
* 对HTTP、HTTPS的全面支持
* Centent-Type的支持（对应Chunked模式）
* 对分块传输的支持
* 文件传输（断点续传）

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
