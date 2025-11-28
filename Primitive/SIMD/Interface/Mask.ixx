module;
export module SIMD:Mask;
import Language;
import <immintrin.h>;
import :Packet;

export namespace SIMD {
    template<int W>
    struct PacketMask;
    template<> struct PacketMask<8> { __m256 reg; };
    template<> struct PacketMask<4> { __m128 reg; };
    template<> struct PacketMask<2> { __m128 reg; };
    template<> struct PacketMask<1> { int reg; };

    inline PacketMask<8> GreaterThan(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_GT_OQ) }; }
    inline Packet<float, 8> Select(PacketMask<8> m, Packet<float, 8> t, Packet<float, 8> f) noexcept { return { _mm256_blendv_ps(f.reg, t.reg, m.reg) }; }
    inline PacketMask<8> Equal(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_EQ_OQ) }; }
    inline PacketMask<8> NotEqual(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_NEQ_OQ) }; }
    inline PacketMask<8> Less(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_LT_OQ) }; }
    inline PacketMask<8> LessEqual(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_LE_OQ) }; }
    inline PacketMask<8> GreaterEqual(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_GE_OQ) }; }

    inline PacketMask<4> GreaterThan(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_GT_OQ) }; }
    inline Packet<float, 4> Select(PacketMask<4> m, Packet<float, 4> t, Packet<float, 4> f) noexcept { return { _mm_blendv_ps(f.reg, t.reg, m.reg) }; }
    inline PacketMask<4> Equal(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_EQ_OQ) }; }
    inline PacketMask<4> NotEqual(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_NEQ_OQ) }; }
    inline PacketMask<4> Less(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_LT_OQ) }; }
    inline PacketMask<4> LessEqual(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_LE_OQ) }; }
    inline PacketMask<4> GreaterEqual(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_GE_OQ) }; }

    inline PacketMask<2> GreaterThan(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_GT_OQ) }; }
    inline Packet<float, 2> Select(PacketMask<2> m, Packet<float, 2> t, Packet<float, 2> f) noexcept { return { _mm_blendv_ps(f.reg, t.reg, m.reg) }; }
    inline PacketMask<2> Equal(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_EQ_OQ) }; }
    inline PacketMask<2> NotEqual(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_NEQ_OQ) }; }
    inline PacketMask<2> Less(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_LT_OQ) }; }
    inline PacketMask<2> LessEqual(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_LE_OQ) }; }
    inline PacketMask<2> GreaterEqual(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_GE_OQ) }; }

    inline PacketMask<1> GreaterThan(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg > b.reg ? 1 : 0 }; }
    inline Packet<float, 1> Select(PacketMask<1> m, Packet<float, 1> t, Packet<float, 1> f) noexcept { return { m.reg ? t.reg : f.reg }; }
    inline PacketMask<1> Equal(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg == b.reg ? 1 : 0 }; }
    inline PacketMask<1> NotEqual(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg != b.reg ? 1 : 0 }; }
    inline PacketMask<1> Less(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg < b.reg ? 1 : 0 }; }
    inline PacketMask<1> LessEqual(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg <= b.reg ? 1 : 0 }; }
    inline PacketMask<1> GreaterEqual(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg >= b.reg ? 1 : 0 }; }

    inline bool All(PacketMask<8> m) noexcept {
        int bits = _mm256_movemask_ps(m.reg);
        return bits == 0xFF;
    }
    inline bool Any(PacketMask<8> m) noexcept {
        int bits = _mm256_movemask_ps(m.reg);
        return bits != 0x00;
    }
    inline bool All(PacketMask<4> m) noexcept {
        int bits = _mm_movemask_ps(m.reg);
        return (bits & 0xF) == 0xF;
    }
    inline bool Any(PacketMask<4> m) noexcept {
        int bits = _mm_movemask_ps(m.reg);
        return (bits & 0xF) != 0x0;
    }
    inline bool All(PacketMask<2> m) noexcept {
        int bits = _mm_movemask_ps(m.reg);
        return (bits & 0x3) == 0x3;
    }
    inline bool Any(PacketMask<2> m) noexcept {
        int bits = _mm_movemask_ps(m.reg);
        return (bits & 0x3) != 0x0;
    }
    inline bool All(PacketMask<1> m) noexcept { return m.reg != 0; }
    inline bool Any(PacketMask<1> m) noexcept { return m.reg != 0; }

    inline UInt8 EqMask8(const UInt8* p, UInt8 b) noexcept {
    #if defined(__AVX2__) || defined(__SSE2__) || defined(_M_X64) || defined(_M_IX86)
        __m128i vec = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p));
        __m128i hb  = _mm_set1_epi8(static_cast<char>(b));
        __m128i cmp = _mm_cmpeq_epi8(vec, hb);
        int m       = _mm_movemask_epi8(cmp);
        return static_cast<UInt8>(m & 0xFF);
    #else
        UInt64 word = *reinterpret_cast<const UInt64*>(p);
        UInt64 rpt = static_cast<UInt64>(b) * 0x0101010101010101ull;
        UInt64 x = word ^ rpt;
        UInt64 t = (x - 0x0101010101010101ull) & (~x) & 0x8080808080808080ull;
        UInt64 z = (t >> 7) & 0x0101010101010101ull;
        UInt64 packed = (z * 0x0102040810204080ull) >> 56;
        return static_cast<UInt8>(packed);
    #endif
    }
}
