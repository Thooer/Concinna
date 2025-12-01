export module Sys.Task:TaskGraph;
import Lang;
import Memory;
import Sys;
import Prm.Ownership:Memory;

export namespace Sys {
    export struct TaskHandle { USize id{0}; };
    export using TaskFunc = void(*)(void*) noexcept;
    struct TaskNode { TaskFunc fn{nullptr}; void* arg{nullptr}; UInt8 qos{0}; };
    struct Edge { USize from; USize to; };

    export struct TaskGraph {
        TaskNode* m_nodes{nullptr}; Edge* m_edges{nullptr}; USize m_nodeCap{0}; USize m_edgeCap{0}; USize m_nodeCount{0}; USize m_edgeCount{0};
        TaskGraph() noexcept {}
        ~TaskGraph() noexcept {
            auto h = Prm::Heap::GetProcessDefault();
            if (m_nodes) { (void)Prm::Heap::FreeRaw(h, m_nodes); m_nodes = nullptr; }
            if (m_edges) { (void)Prm::Heap::FreeRaw(h, m_edges); m_edges = nullptr; }
            m_nodeCap = m_edgeCap = m_nodeCount = m_edgeCount = 0;
        }
        bool Reserve(USize nodes, USize edges) noexcept {
            auto h = Prm::Heap::GetProcessDefault();
            if (nodes) { auto rn = Prm::Heap::AllocRaw(h, sizeof(TaskNode) * nodes); if (!rn.IsOk()) return false; m_nodes = static_cast<TaskNode*>(rn.Value()); m_nodeCap = nodes; }
            if (edges) { auto re = Prm::Heap::AllocRaw(h, sizeof(Edge) * edges); if (!re.IsOk()) return false; m_edges = static_cast<Edge*>(re.Value()); m_edgeCap = edges; }
            return true;
        }
        [[nodiscard]] TaskHandle Add(TaskFunc fn, void* arg, UInt8 qos=0) noexcept {
            if (!fn) return TaskHandle{0};
            if (m_nodeCount >= m_nodeCap) { return TaskHandle{0}; }
            auto idx = m_nodeCount++;
            m_nodes[idx] = TaskNode{ fn, arg, qos };
            return TaskHandle{ idx + 1 };
        }
        [[nodiscard]] bool DependsOn(TaskHandle a, TaskHandle b) noexcept {
            if (a.id == 0 || b.id == 0) return false;
            if (m_edgeCount >= m_edgeCap) return false;
            m_edges[m_edgeCount++] = Edge{ b.id - 1, a.id - 1 };
            return true;
        }
        [[nodiscard]] bool Dispatch() noexcept {
            if (m_nodeCount == 0) return true;
            USize indegCap = m_nodeCount;
            auto h = Prm::Heap::GetProcessDefault();
            auto r = Prm::Heap::AllocRaw(h, sizeof(USize) * indegCap);
            if (!r.IsOk()) return false; auto* indeg = static_cast<USize*>(r.Value());
            for (USize i=0;i<m_nodeCount;++i) indeg[i] = 0;
            for (USize i=0;i<m_edgeCount;++i) { auto e = m_edges[i]; ++indeg[e.to]; }
            // 简单的 Kahn 拓扑排序调度
            for (USize round=0; round<m_nodeCount; ++round) {
                bool progressed=false;
                for (USize i=0;i<m_nodeCount;++i) {
                    if (indeg[i]==0 && m_nodes[i].fn) {
                        if (!Sys::JobSubmitPriority(m_nodes[i].fn, m_nodes[i].arg, m_nodes[i].qos).Ok()) { (void)Prm::Heap::FreeRaw(h, indeg); return false; }
                        indeg[i] = static_cast<USize>(-1);
                        for (USize k=0;k<m_edgeCount;++k) { auto e = m_edges[k]; if (e.from==i) { if (indeg[e.to] > 0) --indeg[e.to]; } }
                        progressed=true;
                    }
                }
                if (!progressed) break;
            }
            (void)Prm::Heap::FreeRaw(h, indeg);
            return true;
        }
    };
}
