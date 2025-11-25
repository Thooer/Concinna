# ✅ 最终架构要素（精简版）

**“跨平台音频后端抽象 + 低延迟混音 Graph（Bus/Submix/Effects）+ 实时空间化（HRTF/距离/occlusion）+ 流式解码与声源管理（Voice Pool/优先级/Stealing）——与 Scene/Physics/Resource 无缝联动。”**

1. **Audio Backend Abstraction**

   * 统一接口（Device, OutputStream, InputStream, LatencyHint, SampleRate），实现多平台后端：WASAPI/DS/ASIO（Windows）、CoreAudio（macOS/iOS）、AAudio/OpenSL（Android）、ALSA/Pulse（Linux）、WebAudio（Web）。
   * 低延迟音频线程（Realtime mixing thread）与非实时任务线程池（IO/decoder/effects upload）。

2. **Mixing / Audio Graph**

   * 节点：Source → Voice → Bus/Submix → Master → Output。
   * 每节点支持 Effects 插件（EQ, Compressor, ConvolutionReverb, Delay）。
   * 支持 Send/Return、sidechain、per-bus volume/ducking。

3. **Spatialization / 3D Audio**

   * Distance model + Doppler + HRTF（ binaural ）支持（软体或调用平台/third-party，如 Google's Resonance, Steam Audio, Microsoft Spatial Sound）。
   * Per-voice spatial params：position, velocity, orientation, cone, radius, LOD（近用高质量 HRTF，远用简单 panning）。

4. **Occlusion / Obstruction**

   * 用 Physics（Scene）做快速射线/多点采样或体积采样，计算传输损失和低通滤波参数（实时低通/LPF 滤波 + gain）。
   * 异步查询以避免阻塞音频线程，结果由 audio thread 平滑插值。

5. **Asset / Streaming Pipeline（与 Resource 对接）**

   * 支持短音效全解码入内存（PCM），长媒体/音乐/语音用流式容器（OGG/Opus/MP3）逐块解码并异步 upload 到 buffer。
   * Metadata: length, channels, sampleRate, loopPoints, memoryHint, priority。
   * 支持预解码、GPU 上传（如果使用 audio GPU 加速）或 zero-copy 框架。

6. **Voice Management（Pool & Priority）**

   * 固定大小 voice pool（避免频繁 alloc），支持优先级/stealing 策略、group-based concurrency limits（max voices per bus）。
   * Voice lifecycle: request → prepare decode/stream → start (ramp) → stop (ramp) → recycle。

7. **Effects & DSP**

   * 插件化 DSP：实现链式处理、可在线编辑（hot-reload），支持 convolution reverb（IR cache）、param automation（曲线）。
   * 可选 SIMD / NEON 优化，或将部分处理移到 compute shaders（实验性）。

8. **Tools / Editor 支持**

   * SoundCue / Event 编辑器、Mixer 覆盖、Profiler（CPU/ms per voice、peak/RMS、memory）、WaveView（波形/loop 编辑）。
   * Live-edit（参数/patch 热更新）与录制/播放调试。

9. **Networking / Voice**（可选）

   * 语音聊天模块独立，使用 Opus 或自定义编解码，低延迟 packetization 与 jitter buffer，不与主 mixer 争用实时线程资源。

10. **Integration Points**

    * **Scene**：AudioComponent 挂 Node（读 Transform），Scene 的 Spatial Layer 提供 sector/region LOD。
    * **Physics**：用于 occlusion/obstruction、反射路径（可用于早期反射估算）。
    * **Resource**：AssetID、streaming、依赖图用于加载音频资产。
    * **Gameplay**：事件/参数（RTPC）驱动 AudioGraph（如状态机触发 SFX）。

---

# 🔧 最小可行版本（快速达成且未来不返工）

* 后端抽象 + 一个桌面后端（WASAPI 或 CoreAudio）+ WebAudio（如果要 Web）。
* 基本 Mixing Thread（支持多个声音混合到 stereo/ambisonic target）。
* 简单 Voice Pool、异步文件 IO + OGG 解码流。
* AudioComponent（绑定 SceneNode） + 基本距离衰减与 Doppler。
* Bus/Submix（至少 Master + SFX + Music）与一个简单 Reverb（plate 模拟或 convolution with small IR）。
* Profiler hooks（实时 voice count、latency meter）。

这个最小实现已能支撑大多数游戏；后续扩展（HRTF、convolution、occlusion、network voice、复杂 effects）都可在该基础上逐步加入，不会有架构冲突。

---

# 性能与实时性要点（必须从一开始就考虑）

* **音频线程必须无阻塞**（所有 IO/decoding 在后台线程，主混音线程只消费 decode 输出与小量元数据）。
* **固定时间步/帧与样本块处理**，避免 frame drops，提供 adaptive buffer strategy（低延迟模式 vs 稳定模式）。
* **优先级管理**：限制每帧解码量与播放声源数以保证帧内 CPU 预算。
* **安全与浮点**：内部用 float32 运算，最终输出再量化；避免锁争用，使用 lock-free queues 传输数据到 audio thread。

---

# 平台建议（首发支持策略）

* 桌面/主机：WASAPI/CoreAudio（低延迟）
* 移动：AAudio（Android），CoreAudio（iOS）
* Web：WebAudio（注意 sampleRate/latency 不可控）
* 跨平台抽象层从一开始要到位（和 RHI 一样做 Backend 插件）。

---

# 结尾（简短路线图）

1. 实现 Backend Abstraction + 单后端（桌面）
2. 实现 Mixing Thread、Voice Pool、Bus、资源流式加载（OGG）
3. 接入 Scene 的 AudioComponent（位置、velocity）与简单 attenuation/doppler
4. 加入 Submix Effects、Profiler、Editor 工具
5. 后续加入 HRTF、Convolution Reverb、Occlusion（Physics 联动）、Web/移动后端与 voice chat

