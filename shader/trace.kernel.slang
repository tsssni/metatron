import metatron;

namespace mtt {
    struct Buffer {
        float3 x;
        float3 y;
        Ptr<float3> z;
    }

    [shader("compute")]
    func trace(uniform Buffer cb) -> void {
        cb.z[0] = cb.x[0] + cb.y[0];
    }

    [shader("compute")]
    func postprocess(uniform Buffer cb) -> void {
        cb.z[1] = cb.x[1] + cb.y[1];
    }
}
