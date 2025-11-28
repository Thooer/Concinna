export module CryptoTest;

import Cap.Crypto;
import Cap.Memory;
import std.io;

using namespace Cap::Crypto;

void PrintHashResult(const HashResult& result, const char* algorithm) {
    std::cout << algorithm << ": ";
    for (size_t i = 0; i < result.Size; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(result.Data[i]);
    }
    std::cout << std::dec << std::endl;
}

int main() {
    std::cout << "Crypto Module Test" << std::endl;
    std::cout << "================" << std::endl;
    
    // Test data
    const char* testData = "Hello, Crypto!";
    size_t testDataSize = strlen(testData);
    
    std::cout << "\nTest Data: \"" << testData << "\"" << std::endl;
    std::cout << "Data Size: " << testDataSize << " bytes" << std::endl;
    
    // Test CRC32
    uint32_t crc32 = CRC32(testData, testDataSize);
    std::cout << "\nCRC32: " << std::hex << std::setw(8) << std::setfill('0') << crc32 << std::dec << std::endl;
    
    // Test CRC64
    uint64_t crc64 = CRC64(testData, testDataSize);
    std::cout << "CRC64: " << std::hex << std::setw(16) << std::setfill('0') << crc64 << std::dec << std::endl;
    
    // Test MD5
    HashResult md5Result = MD5(testData, testDataSize);
    PrintHashResult(md5Result, "MD5");
    
    // Test SHA256
    HashResult sha256Result = SHA256(testData, testDataSize);
    PrintHashResult(sha256Result, "SHA256");
    
    // Test Hasher interface
    std::cout << "\nTesting Hasher Interface:" << std::endl;
    
    // Test MD5 Hasher
    Hasher* md5Hasher = CreateHasher(HashType::MD5);
    if (md5Hasher) {
        md5Hasher->Append(testData, testDataSize);
        HashResult md5HasherResult = md5Hasher->Finalize();
        PrintHashResult(md5HasherResult, "MD5 (Hasher)");
        DestroyHasher(md5Hasher);
    }
    
    // Test SHA256 Hasher
    Hasher* sha256Hasher = CreateHasher(HashType::SHA256);
    if (sha256Hasher) {
        sha256Hasher->Append(testData, testDataSize);
        HashResult sha256HasherResult = sha256Hasher->Finalize();
        PrintHashResult(sha256HasherResult, "SHA256 (Hasher)");
        DestroyHasher(sha256Hasher);
    }
    
    // Test incremental hashing
    std::cout << "\nTesting Incremental Hashing:" << std::endl;
    Hasher* incrementalHasher = CreateHasher(HashType::SHA256);
    if (incrementalHasher) {
        incrementalHasher->Append("Hello, ", 7);
        incrementalHasher->Append("Crypto!", 7);
        HashResult incrementalResult = incrementalHasher->Finalize();
        PrintHashResult(incrementalResult, "SHA256 (Incremental)");
        DestroyHasher(incrementalHasher);
    }
    
    std::cout << "\nAll tests completed!" << std::endl;
    
    return 0;
}
