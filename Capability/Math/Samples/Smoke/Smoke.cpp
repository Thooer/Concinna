import Cap.Math;
import SIMD;
import <cstdio>;

void TestMatrixMultiply() {
    std::printf("Testing Matrix Multiply...\n");
    constexpr int W = 4;
    auto one = SIMD::Set1<W>(1.0f);
    auto zero = SIMD::Set1<W>(0.0f);
    
    Cap::MatrixPacket<W> id;
    id.c0x = one; id.c0y = zero; id.c0z = zero; id.c0w = zero;
    id.c1x = zero; id.c1y = one; id.c1z = zero; id.c1w = zero;
    id.c2x = zero; id.c2y = zero; id.c2z = one; id.c2w = zero;
    id.c3x = zero; id.c3y = zero; id.c3z = zero; id.c3w = one;
    
    auto res = Cap::Multiply(id, id);
    
    float buf[4];
    SIMD::Packet<float, W>::StoreUnaligned(buf, res.c0x);
    if (buf[0] != 1.0f) std::printf("Error: Id*Id c0x != 1\n");
    else std::printf("Id*Id passed basic check.\n");
}

void TestMatrixInverse() {
    std::printf("Testing Matrix Inverse...\n");
    constexpr int W = 4;
    auto two = SIMD::Set1<W>(2.0f);
    auto zero = SIMD::Set1<W>(0.0f);
    auto one = SIMD::Set1<W>(1.0f);
    
    Cap::MatrixPacket<W> m;
    m.c0x = two; m.c0y = zero; m.c0z = zero; m.c0w = zero;
    m.c1x = zero; m.c1y = two; m.c1z = zero; m.c1w = zero;
    m.c2x = zero; m.c2y = zero; m.c2z = two; m.c2w = zero;
    m.c3x = zero; m.c3y = zero; m.c3z = zero; m.c3w = one;
    
    auto inv = Cap::Inverse(m);
    
    float buf[4];
    SIMD::Packet<float, W>::StoreUnaligned(buf, inv.c0x);
    if (buf[0] != 0.5f) std::printf("Error: Inv Scale c0x != 0.5, got %f\n", buf[0]);
    else std::printf("Inv Scale passed.\n");
}

int main() {
    TestMatrixMultiply();
    TestMatrixInverse();
    return 0;
}
