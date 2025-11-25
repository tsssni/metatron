import metatron;

namespace mtt {
    struct Constant_Buffer {
        Ptr<float3> position;
        Ptr<float3> normal;
    };

    [shader("compute")]
    func main(uniform Constant_Buffer cb) -> void {
        cb.position[0] = cb.normal[0];
    }
}
