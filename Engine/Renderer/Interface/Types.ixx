module;
export module Eng.Renderer:Types;

import Lang;
import Cap.Math;
import SIMD;
 

export namespace Eng {
    constexpr UInt32 kMaxTris = static_cast<UInt32>(65536);
    constexpr UInt32 kMaxLines = static_cast<UInt32>(128);
    constexpr UInt32 kMaxRects = static_cast<UInt32>(64);
    constexpr UInt32 kMaxGlyphs = static_cast<UInt32>(256);

    struct Tri2D { float x0{}, y0{}, z0{}, x1{}, y1{}, z1{}, x2{}, y2{}, z2{}, r{}, g{}, b{}, a{}; };
    struct Line2D { float x0{}, y0{}, x1{}, y1{}, w{}, r{}, g{}, b{}, a{}; };
    struct Rect2D { float x{}, y{}, w{}, h{}, r{}, g{}, b{}, a{}; };
    struct GlyphQuad { float x{}, y{}, w{}, h{}, r{}, g{}, b{}, a{}; };
    struct RGBA { float r{}, g{}, b{}, a{}; };

    struct SimpleFrame {
        float clear[4]{0.0f,0.0f,0.0f,1.0f};
        float screenW{0.0f};
        float screenH{0.0f};
        Tri2D tris[kMaxTris]{}; UInt32 triCount{0};
        Line2D lines[kMaxLines]{}; UInt32 lineCount{0};
        Rect2D rects[kMaxRects]{}; UInt32 rectCount{0};
        GlyphQuad glyphs[kMaxGlyphs]{}; UInt32 glyphCount{0};
    };

    export inline void SetClear(SimpleFrame& fr, float r, float g, float b, float a) noexcept { fr.clear[0]=r; fr.clear[1]=g; fr.clear[2]=b; fr.clear[3]=a; }
    export inline void PushTri(SimpleFrame& fr, float x0,float y0,float z0,float x1,float y1,float z1,float x2,float y2,float z2,float r,float g,float b,float a) noexcept {
        if (fr.triCount < kMaxTris) { auto& t = fr.tris[fr.triCount++]; t.x0=x0; t.y0=y0; t.z0=z0; t.x1=x1; t.y1=y1; t.z1=z1; t.x2=x2; t.y2=y2; t.z2=z2; t.r=r; t.g=g; t.b=b; t.a=a; }
    }
    export inline void PushRect(SimpleFrame& fr, float x,float y,float w,float h,float r,float g,float b,float a) noexcept {
        if (fr.rectCount < kMaxRects) { auto& rc = fr.rects[fr.rectCount++]; rc.x=x; rc.y=y; rc.w=w; rc.h=h; rc.r=r; rc.g=g; rc.b=b; rc.a=a; }
    }
    export inline void PushLine(SimpleFrame& fr, float x0,float y0,float x1,float y1,float thickness,float r,float g,float b,float a) noexcept {
        if (fr.lineCount < kMaxLines) { auto& ln = fr.lines[fr.lineCount++]; ln.x0=x0; ln.y0=y0; ln.x1=x1; ln.y1=y1; ln.w=thickness; ln.r=r; ln.g=g; ln.b=b; ln.a=a; }
    }
    export inline void PushGlyph(SimpleFrame& fr, float x,float y,float w,float h,float r,float g,float b,float a) noexcept {
        if (fr.glyphCount < kMaxGlyphs) { auto& gq = fr.glyphs[fr.glyphCount++]; gq.x=x; gq.y=y; gq.w=w; gq.h=h; gq.r=r; gq.g=g; gq.b=b; gq.a=a; }
    }

    export inline void Mul4x4(const float a[16], const float b[16], float out[16]) noexcept {
        for (int i=0;i<4;++i) { for (int j=0;j<4;++j) { out[i*4+j] = a[i*4+0]*b[0*4+j] + a[i*4+1]*b[1*4+j] + a[i*4+2]*b[2*4+j] + a[i*4+3]*b[3*4+j]; } }
    }
    export inline void MakeIdentity(float out[16]) noexcept { for(int i=0;i<16;++i) out[i]=0.0f; out[0]=1.0f; out[5]=1.0f; out[10]=1.0f; out[15]=1.0f; }
    export inline float FastAbs(float x) noexcept { return x>=0.0f?x:-x; }
    export inline float FastSqrt(float x) noexcept { if (x<=0.0f) return 0.0f; float g = x>1.0f ? x*0.5f : 1.0f; for (int i=0;i<6;++i) g = 0.5f*(g + x/g); return g; }
    export inline float FastWrap(float x, float period) noexcept { int n = (int)(x/period); x -= n*period; if (x<0.0f) x += period; return x; }
    export inline float FastSin(float x) noexcept {
        float in = x; auto p = SIMD::Packet<float,1>::LoadAligned(&in);
        auto r = Cap::Transcendental::Sin<1>(p);
        float out; SIMD::Packet<float,1>::StoreAligned(&out, r);
        return out;
    }
    export inline float FastCos(float x) noexcept {
        float in = x; auto p = SIMD::Packet<float,1>::LoadAligned(&in);
        auto r = Cap::Transcendental::Cos<1>(p);
        float out; SIMD::Packet<float,1>::StoreAligned(&out, r);
        return out;
    }
    export inline float FastTan(float x) noexcept { float s = FastSin(x), c = FastCos(x); if (c==0.0f) return 1e9f; return s/c; }
    export inline void MakeRotationY(float angle, float out[16]) noexcept { float c = FastCos(angle); float s = FastSin(angle); out[0]=c; out[1]=0.0f; out[2]=-s; out[3]=0.0f; out[4]=0.0f; out[5]=1.0f; out[6]=0.0f; out[7]=0.0f; out[8]=s; out[9]=0.0f; out[10]=c; out[11]=0.0f; out[12]=0.0f; out[13]=0.0f; out[14]=0.0f; out[15]=1.0f; }
    export inline void MakeTranslation(float tx, float ty, float tz, float out[16]) noexcept { MakeIdentity(out); out[3]=tx; out[7]=ty; out[11]=tz; }
    export inline void MakeScale(float sx, float sy, float sz, float out[16]) noexcept { for(int i=0;i<16;++i) out[i]=0.0f; out[0]=sx; out[5]=sy; out[10]=sz; out[15]=1.0f; }
    export inline void MakePerspective(float fovY, float aspect, float nearZ, float farZ, float out[16]) noexcept { float f = 1.0f/FastTan(fovY*0.5f); out[0]=f/aspect; out[1]=0.0f; out[2]=0.0f; out[3]=0.0f; out[4]=0.0f; out[5]=f; out[6]=0.0f; out[7]=0.0f; out[8]=0.0f; out[9]=0.0f; out[10]=(farZ+nearZ)/(nearZ-farZ); out[11]=(2.0f*farZ*nearZ)/(nearZ-farZ); out[12]=0.0f; out[13]=0.0f; out[14]=-1.0f; out[15]=0.0f; }
    export inline void MakeLookAt(float ex,float ey,float ez,
                                  float cx,float cy,float cz,
                                  float ux,float uy,float uz,
                                  float out[16]) noexcept {
        float fx = cx - ex, fy = cy - ey, fz = cz - ez;
        float fl = FastSqrt(fx*fx + fy*fy + fz*fz); if (fl==0.0f) fl=1.0f; fx/=fl; fy/=fl; fz/=fl;
        float slx = fy*uz - fz*uy; float sly = fz*ux - fx*uz; float slz = fx*uy - fy*ux;
        float sll = FastSqrt(slx*slx + sly*sly + slz*slz); if (sll==0.0f) sll=1.0f; slx/=sll; sly/=sll; slz/=sll;
        float upx = sly*fz - slz*fy; float upy = slz*fx - slx*fz; float upz = slx*fy - sly*fx;
        out[0]=slx; out[1]=sly; out[2]=slz; out[3]=-(slx*ex + sly*ey + slz*ez);
        out[4]=upx; out[5]=upy; out[6]=upz; out[7]=-(upx*ex + upy*ey + upz*ez);
        out[8]=-fx; out[9]=-fy; out[10]=-fz; out[11]=(fx*ex + fy*ey + fz*ez);
        out[12]=0.0f; out[13]=0.0f; out[14]=0.0f; out[15]=1.0f;
    }
    export inline void ProjectToScreen(const float m[16], float x, float y, float z, const SimpleFrame& fr, float& sx, float& sy, float& sz) noexcept {
        float X = x*m[0] + y*m[1] + z*m[2] + 1.0f*m[3];
        float Y = x*m[4] + y*m[5] + z*m[6] + 1.0f*m[7];
        float Z = x*m[8] + y*m[9] + z*m[10] + 1.0f*m[11];
        float W = x*m[12] + y*m[13] + z*m[14] + 1.0f*m[15];
        if (W == 0.0f) W = 1.0f;
        float nx = X/W; float ny = Y/W; float nz = Z/W;
        sx = (nx*0.5f+0.5f)*fr.screenW; sy = (1.0f-(ny*0.5f+0.5f))*fr.screenH;
        sz = nz*0.5f + 0.5f;
    }
    export inline bool IsBackface(float x0,float y0,float x1,float y1,float x2,float y2) noexcept {
        float cx = (x1 - x0)*(y2 - y0) - (y1 - y0)*(x2 - x0);
        return cx < 0.0f;
    }
    export inline bool TriOutsideViewport(const SimpleFrame& fr, float x0,float y0,float x1,float y1,float x2,float y2) noexcept {
        float minx = x0; if (x1 < minx) minx = x1; if (x2 < minx) minx = x2;
        float maxx = x0; if (x1 > maxx) maxx = x1; if (x2 > maxx) maxx = x2;
        float miny = y0; if (y1 < miny) miny = y1; if (y2 < miny) miny = y2;
        float maxy = y0; if (y1 > maxy) maxy = y1; if (y2 > maxy) maxy = y2;
        return (maxx < 0.0f) || (maxy < 0.0f) || (minx > fr.screenW) || (miny > fr.screenH);
    }
    export inline void ClipTriToViewportAndPush(SimpleFrame& fr, float x0,float y0,float z0, float x1,float y1,float z1, float x2,float y2,float z2, float r,float g,float b,float a) noexcept {
        struct P { float x; float y; float z; };
        P poly[8]; P out[8]; unsigned pc = 3;
        poly[0] = {x0,y0,z0}; poly[1] = {x1,y1,z1}; poly[2] = {x2,y2,z2};
        auto clipEdge = [&](float ex, float ey, float nx, float ny, unsigned inCount) noexcept -> unsigned {
            unsigned outCount = 0;
            for (unsigned i=0;i<inCount;++i) {
                P cur = poly[i]; P prev = poly[(i+inCount-1)%inCount];
                float dc = (cur.x-ex)*nx + (cur.y-ey)*ny;
                float dp = (prev.x-ex)*nx + (prev.y-ey)*ny;
                bool cin = dc >= 0.0f; bool pin = dp >= 0.0f;
                if (pin && cin) { out[outCount++] = cur; }
                else if (pin && !cin) {
                    float tnum = (ex - prev.x)*nx + (ey - prev.y)*ny;
                    float tden = ((cur.x - prev.x)*nx + (cur.y - prev.y)*ny);
                    float t = (tden==0.0f) ? 0.0f : (tnum/tden);
                    out[outCount++] = { prev.x + (cur.x - prev.x)*t, prev.y + (cur.y - prev.y)*t, prev.z + (cur.z - prev.z)*t };
                }
                else if (!pin && cin) {
                    float tnum = (ex - prev.x)*nx + (ey - prev.y)*ny;
                    float tden = ((cur.x - prev.x)*nx + (cur.y - prev.y)*ny);
                    float t = (tden==0.0f) ? 0.0f : (tnum/tden);
                    out[outCount++] = { prev.x + (cur.x - prev.x)*t, prev.y + (cur.y - prev.y)*t, prev.z + (cur.z - prev.z)*t };
                    out[outCount++] = cur;
                }
            }
            for (unsigned i=0;i<outCount;++i) poly[i] = out[i];
            return outCount;
        };
        pc = clipEdge(0.0f, 0.0f, 1.0f, 0.0f, pc); if (pc==0) return;
        pc = clipEdge(fr.screenW, 0.0f, -1.0f, 0.0f, pc); if (pc==0) return;
        pc = clipEdge(0.0f, 0.0f, 0.0f, 1.0f, pc); if (pc==0) return;
        pc = clipEdge(0.0f, fr.screenH, 0.0f, -1.0f, pc); if (pc==0) return;
        for (unsigned i=1;i+1<pc;++i) { PushTri(fr, poly[0].x, poly[0].y, poly[0].z, poly[i].x, poly[i].y, poly[i].z, poly[i+1].x, poly[i+1].y, poly[i+1].z, r,g,b,a); }
    }
    export inline void PushUnitCube(SimpleFrame& fr, const float mvp[16], float r, float g, float b, float a) noexcept {
        const float v[8][3] = { {-1.0f,-1.0f,-1.0f}, {1.0f,-1.0f,-1.0f}, {1.0f,1.0f,-1.0f}, {-1.0f,1.0f,-1.0f}, {-1.0f,-1.0f,1.0f}, {1.0f,-1.0f,1.0f}, {1.0f,1.0f,1.0f}, {-1.0f,1.0f,1.0f} };
        const UInt32 idx[12][3] = { {0,1,2},{0,2,3}, {4,6,5},{4,7,6}, {0,4,5},{0,5,1}, {1,5,6},{1,6,2}, {2,6,7},{2,7,3}, {3,7,4},{3,4,0} };
        for (int i=0;i<12;++i) {
            float x0,y0,z0,x1,y1,z1,x2,y2,z2;
            ProjectToScreen(mvp, v[idx[i][0]][0], v[idx[i][0]][1], v[idx[i][0]][2], fr, x0,y0,z0);
            ProjectToScreen(mvp, v[idx[i][1]][0], v[idx[i][1]][1], v[idx[i][1]][2], fr, x1,y1,z1);
            ProjectToScreen(mvp, v[idx[i][2]][0], v[idx[i][2]][1], v[idx[i][2]][2], fr, x2,y2,z2);
            if (IsBackface(x0,y0,x1,y1,x2,y2)) continue;
            if (TriOutsideViewport(fr, x0,y0,x1,y1,x2,y2)) continue;
            ClipTriToViewportAndPush(fr, x0,y0,z0, x1,y1,z1, x2,y2,z2, r,g,b,a);
        }
    }
    export inline void PushUnitCubeWire(SimpleFrame& fr, const float mvp[16], float thickness, float r, float g, float b, float a) noexcept {
        const float v[8][3] = { {-1.0f,-1.0f,-1.0f}, {1.0f,-1.0f,-1.0f}, {1.0f,1.0f,-1.0f}, {-1.0f,1.0f,-1.0f}, {-1.0f,-1.0f,1.0f}, {1.0f,-1.0f,1.0f}, {1.0f,1.0f,1.0f}, {-1.0f,1.0f,1.0f} };
        auto proj = [&](int i, float& sx, float& sy) noexcept { float sz; ProjectToScreen(mvp, v[i][0], v[i][1], v[i][2], fr, sx, sy, sz); };
        float p[8][2]; for (int i=0;i<8;++i) proj(i, p[i][0], p[i][1]);
        int edges[12][2] = { {0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7} };
        for (int e=0;e<12;++e) {
            int s = edges[e][0];
            int t = edges[e][1];
            PushLine(fr, p[s][0], p[s][1], p[t][0], p[t][1], thickness, r, g, b, a);
        }
    }
    export inline void PushUnitCubeFaceColors(SimpleFrame& fr, const float mvp[16], const RGBA* faceColors) noexcept {
        const float v[8][3] = { {-1.0f,-1.0f,-1.0f}, {1.0f,-1.0f,-1.0f}, {1.0f,1.0f,-1.0f}, {-1.0f,1.0f,-1.0f}, {-1.0f,-1.0f,1.0f}, {1.0f,-1.0f,1.0f}, {1.0f,1.0f,1.0f}, {-1.0f,1.0f,1.0f} };
        const UInt32 idx[12][3] = { {0,1,2},{0,2,3}, {4,6,5},{4,7,6}, {0,4,5},{0,5,1}, {1,5,6},{1,6,2}, {2,6,7},{2,7,3}, {3,7,4},{3,4,0} };
        const int triFace[12] = { 0,0, 1,1, 2,2, 3,3, 4,4, 5,5 };
        for (int i=0;i<12;++i) {
            float x0,y0,z0,x1,y1,z1,x2,y2,z2;
            ProjectToScreen(mvp, v[idx[i][0]][0], v[idx[i][0]][1], v[idx[i][0]][2], fr, x0,y0,z0);
            ProjectToScreen(mvp, v[idx[i][1]][0], v[idx[i][1]][1], v[idx[i][1]][2], fr, x1,y1,z1);
            ProjectToScreen(mvp, v[idx[i][2]][0], v[idx[i][2]][1], v[idx[i][2]][2], fr, x2,y2,z2);
            if (IsBackface(x0,y0,x1,y1,x2,y2)) continue;
            if (TriOutsideViewport(fr, x0,y0,x1,y1,x2,y2)) continue;
            const RGBA& c = faceColors[triFace[i]];
            ClipTriToViewportAndPush(fr, x0,y0,z0, x1,y1,z1, x2,y2,z2, c.r,c.g,c.b,c.a);
        }
    }
}
