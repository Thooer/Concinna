export module Cap.Algorithms:Radix;

import Lang;

export namespace Cap {
    inline void RadixSort(Span<UInt32> keys) noexcept {
        USize n = keys.size(); if (n == 0) return;
        UInt32* buf = static_cast<UInt32*>(::operator new[](n * sizeof(UInt32)));
        for (int pass = 0; pass < 4; ++pass) {
            UInt32 count[256]{};
            int shift = pass * 8;
            for (USize i = 0; i < n; ++i) { ++count[(keys[i] >> shift) & 0xFFu]; }
            UInt32 sum = 0; for (int i = 0; i < 256; ++i) { UInt32 c = count[i]; count[i] = sum; sum += c; }
            for (USize i = 0; i < n; ++i) { UInt32 b = (keys[i] >> shift) & 0xFFu; buf[count[b]++] = keys[i]; }
            for (USize i = 0; i < n; ++i) { keys[i] = buf[i]; }
        }
        ::operator delete[](buf);
    }

    inline void RadixSortPairs(Span<UInt32> keys, Span<UInt32> values) noexcept {
        USize n = keys.size(); if (values.size() < n) n = values.size(); if (n == 0) return;
        UInt32* kbuf = static_cast<UInt32*>(::operator new[](n * sizeof(UInt32)));
        UInt32* vbuf = static_cast<UInt32*>(::operator new[](n * sizeof(UInt32)));
        for (int pass = 0; pass < 4; ++pass) {
            UInt32 count[256]{};
            int shift = pass * 8;
            for (USize i = 0; i < n; ++i) { ++count[(keys[i] >> shift) & 0xFFu]; }
            UInt32 sum = 0; for (int i = 0; i < 256; ++i) { UInt32 c = count[i]; count[i] = sum; sum += c; }
            for (USize i = 0; i < n; ++i) {
                UInt32 b = (keys[i] >> shift) & 0xFFu;
                USize idx = count[b]++;
                kbuf[idx] = keys[i]; vbuf[idx] = values[i];
            }
            for (USize i = 0; i < n; ++i) { keys[i] = kbuf[i]; values[i] = vbuf[i]; }
        }
        ::operator delete[](kbuf);
        ::operator delete[](vbuf);
    }
}
