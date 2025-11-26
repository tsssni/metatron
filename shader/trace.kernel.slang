import metatron;

namespace mtt {
    struct Constant_Buffer {
        Ptr<float3> position;
        Ptr<float3> normal;
    };

    [shader("compute")]
    func trace(uniform Constant_Buffer cb) -> void {
        cb.position[1] = cb.normal[1];
    }

    [shader("compute")]
    func postprocess(uniform Constant_Buffer cb) -> void {
        cb.position[0] = cb.normal[0];
    }
}
