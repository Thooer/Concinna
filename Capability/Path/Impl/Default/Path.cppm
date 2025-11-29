module Cap.Path;

import Cap.Path;
import Lang;

using namespace Cap;

Expect<USize> Path::Normalize(Span<const Char8, DynamicExtent> inPath, Span<Char8, DynamicExtent> out) noexcept {
    USize oi = 0;
    USize i = 0;
    const USize n = inPath.size();
    while (i < n) {
        Char8 c = inPath[i];
        if (c == '\\' || c == '/') {
            while (i < n && (inPath[i] == '\\' || inPath[i] == '/')) { ++i; }
            if (oi > 0 && out[oi - 1] == '/') { continue; }
            if (oi >= out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            out[oi++] = '/';
            continue;
        }
        if (c == '.') {
            if ((i + 1 >= n) || (inPath[i + 1] == '\\' || inPath[i + 1] == '/')) { ++i; continue; }
            if (i + 1 < n && inPath[i + 1] == '.' && (i + 2 >= n || inPath[i + 2] == '\\' || inPath[i + 2] == '/')) {
                i += 2;
                if (oi > 0 && out[oi - 1] == '/') { if (oi > 1) { --oi; } }
                while (oi > 0 && out[oi - 1] != '/') { --oi; }
                continue;
            }
        }
        if (oi >= out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        out[oi++] = c;
        ++i;
    }
    return Expect<USize>::Ok(oi);
}

Expect<USize> Path::Join(Span<const Char8, DynamicExtent> a, Span<const Char8, DynamicExtent> b, Span<Char8, DynamicExtent> out) noexcept {
    USize oi = 0;
    for (USize i = 0; i < a.size(); ++i) {
        if (oi >= out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        out[oi++] = a[i];
    }
    if (b.size() > 0) {
        bool needSep = (oi > 0 && out[oi - 1] != '/');
        USize bi = 0;
        if (b[0] == '/' || b[0] == '\\') { bi = 1; needSep = false; }
        if (needSep) {
            if (oi >= out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            out[oi++] = '/';
        }
        for (; bi < b.size(); ++bi) {
            if (oi >= out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            Char8 c = b[bi];
            if (c == '\\') c = '/';
            out[oi++] = c;
        }
    }
    return Expect<USize>::Ok(oi);
}

Expect<USize> Path::Basename(Span<const Char8, DynamicExtent> inPath, Span<Char8, DynamicExtent> out) noexcept {
    if (inPath.size() == 0) return Expect<USize>::Ok(0);
    USize end = inPath.size();
    while (end > 0 && (inPath[end - 1] == '/' || inPath[end - 1] == '\\')) { --end; }
    USize start = end;
    while (start > 0) {
        Char8 c = inPath[start - 1];
        if (c == '/' || c == '\\') break;
        --start;
    }
    USize len = (end > start) ? (end - start) : 0;
    if (len > out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
    for (USize i = 0; i < len; ++i) { out[i] = inPath[start + i]; }
    return Expect<USize>::Ok(len);
}

Expect<USize> Path::Extension(Span<const Char8, DynamicExtent> inPath, Span<Char8, DynamicExtent> out) noexcept {
    if (inPath.size() == 0) return Expect<USize>::Ok(0);
    USize end = inPath.size();
    while (end > 0 && (inPath[end - 1] == '/' || inPath[end - 1] == '\\')) { --end; }
    USize sep = 0;
    for (USize i = 0; i < end; ++i) { if (inPath[i] == '/' || inPath[i] == '\\') sep = i + 1; }
    USize dotPos = 0;
    for (USize i = sep; i < end; ++i) { if (inPath[i] == '.') dotPos = i; }
    if (dotPos == 0 || dotPos + 1 >= end) return Expect<USize>::Ok(0);
    USize len = end - (dotPos + 1);
    if (len > out.size()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
    for (USize i = 0; i < len; ++i) { out[i] = inPath[dotPos + 1 + i]; }
    return Expect<USize>::Ok(len);
}
