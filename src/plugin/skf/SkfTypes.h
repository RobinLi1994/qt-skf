/**
 * @file SkfTypes.h
 * @brief SKF API 类型定义
 *
 * 定义 SKF (智能密码钥匙) C API 使用的基础类型和结构体
 */

#pragma once

#include <cstdint>

// 确保结构体按 1 字节对齐
#pragma pack(push, 1)

namespace wekey {
namespace skf {

//=== 基础类型定义 ===

using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using ULONG = uint32_t;
using BOOL = int32_t;
using CHAR = char;
using LPSTR = char*;
using LPCSTR = const char*;
using PULONG = ULONG*;

//=== 句柄类型 ===

using DEVHANDLE = void*;      ///< 设备句柄
using HAPPLICATION = void*;   ///< 应用句柄
using HCONTAINER = void*;     ///< 容器句柄

///=== 常量定义 ===

constexpr int MAX_IV_LEN = 32;           ///< 最大 IV 长度

//=== 结构体定义 ===

/**
 * @brief 版本信息
 */
struct VERSION {
    BYTE major;  ///< 主版本号
    BYTE minor;  ///< 次版本号
};

/**
 * @brief 设备信息
 */
struct DEVINFO {
    VERSION version;                ///< 版本号
    CHAR manufacturer[64];          ///< 制造商名称
    CHAR issuer[64];                ///< 发行商名称
    CHAR label[32];                 ///< 设备标签
    CHAR serialNumber[32];          ///< 设备序列号
    VERSION hwVersion;              ///< 硬件版本
    VERSION firmwareVersion;        ///< 固件版本
    ULONG algSymCap;                ///< 对称算法能力
    ULONG algAsymCap;               ///< 非对称算法能力
    ULONG algHashCap;               ///< 哈希算法能力
    ULONG devAuthAlgId;             ///< 设备认证算法标识
    ULONG totalSpace;               ///< 总空间
    ULONG freeSpace;                ///< 剩余空间
    ULONG maxECCBufferSize;         ///< 最大 ECC 缓冲区大小
    ULONG maxBufferSize;            ///< 最大缓冲区大小
    BYTE reserved[64];              ///< 保留字段
};

/**
 * @brief ECC 公钥结构
 */
struct ECCPUBLICKEYBLOB {
    ULONG bitLen;       ///< 位长度
    BYTE xCoordinate[64];  ///< X 坐标
    BYTE yCoordinate[64];  ///< Y 坐标
};

/**
 * @brief ECC 签名结构
 */
struct ECCSIGNATUREBLOB {
    BYTE r[64];  ///< r 值
    BYTE s[64];  ///< s 值
};

/**
 * @brief RSA 公钥结构
 */
struct RSAPUBLICKEYBLOB {
    ULONG algID;        ///< 算法标识
    ULONG bitLen;       ///< 位长度
    BYTE modulus[256];  ///< 模数
    BYTE publicExponent[4];  ///< 公钥指数
};

/**
 * @brief 块密码参数
 */
struct BLOCKCIPHERPARAM {
    BYTE iv[MAX_IV_LEN];  ///< 初始化向量
    ULONG ivLen;          ///< IV 长度
    ULONG paddingType;    ///< 填充类型
    ULONG feedBitLen;     ///< 反馈位长度
};

/**
 * @brief ECC 密文结构
 */
struct ECCCIPHERBLOB {
    BYTE xCoordinate[64];   ///< X 坐标
    BYTE yCoordinate[64];   ///< Y 坐标
    BYTE hash[32];          ///< 哈希值
    ULONG cipherLen;        ///< 密文长度
    BYTE cipherData[1];     ///< 密文数据（变长，实际长度由 cipherLen 决定）
};

/**
 * @brief ECC 加密私钥信封结构 (GMT-0016)
 *
 * 用于 SKF_ImportECCKeyPair，包含加密的私钥、加密公钥和对称密钥密文
 */
struct ENVELOPEDKEYBLOB {
    ULONG version;                  ///< 版本号（固定为 1）
    ULONG ulSymAlgId;               ///< 对称算法标识（如 SGD_SM4_ECB）
    ULONG ulBits;                   ///< 密钥位长度（如 256）
    BYTE cbEncryptedPriKey[64];     ///< 加密的私钥密文
    ECCPUBLICKEYBLOB pubKey;        ///< 加密公钥
    ECCCIPHERBLOB eccCipherBlob;    ///< 对称密钥密文
};

/**
 * @brief 文件属性
 */
struct FILEATTRIBUTE {
    CHAR fileName[32];   ///< 文件名
    ULONG fileSize;      ///< 文件大小
    ULONG readRights;    ///< 读权限
    ULONG writeRights;   ///< 写权限
};

//=== 算法标识常量 ===

constexpr ULONG SGD_SM1_ECB = 0x00000101;  ///< SM1 ECB
constexpr ULONG SGD_SM1_CBC = 0x00000102;  ///< SM1 CBC
constexpr ULONG SGD_SM4_ECB = 0x00000401;  ///< SM4 ECB
constexpr ULONG SGD_SM4_CBC = 0x00000402;  ///< SM4 CBC
constexpr ULONG SGD_RSA = 0x00010000;      ///< RSA
constexpr ULONG SGD_SM2_1 = 0x00020100;    ///< SM2-1 签名
constexpr ULONG SGD_SM2_2 = 0x00020200;    ///< SM2-2 密钥交换
constexpr ULONG SGD_SM2_3 = 0x00020400;    ///< SM2-3 加密
constexpr ULONG SGD_SM3 = 0x00000001;      ///< SM3 哈希
constexpr ULONG SGD_SHA1 = 0x00000002;     ///< SHA-1 哈希
constexpr ULONG SGD_SHA256 = 0x00000004;   ///< SHA-256 哈希

//=== 调用约定 ===

#if defined(_WIN32) || defined(_WIN64)
#define SKF_API __stdcall
#else
#define SKF_API
#endif

}  // namespace skf
}  // namespace wekey

#pragma pack(pop)
