import metatron;

namespace mtt {
    struct Integrate {
        math::Transform transform;
        photo::Film film;
        accel::Acceleration_Union accel;
        photo::Lens_Union lens;
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        iv3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        let pixel = global_idx.xy;
        var size: uv2;
        in.image.GetDimensions(size.x, size.y);
        let fixel = in.film(0, pixel, size, 0);

        let camera: photo::Camera;
        let s_opt = camera.sample(in.lens.get(), fixel.position, fixel.dxdy, fv2(0));
        if (s_opt == none) {
            in.image.Store(pixel, 0);
            return;
        }

        var s = s_opt.value;
        s.ray_differential = in.transform / s.ray_differential;
        s.default_differential = in.transform / s.default_differential;
        let a_intr_opt = in.accel.get().trace(s.ray_differential.r, 0);
        if (a_intr_opt == none || a_intr_opt.value.intr_opt == none) {
            in.image.Store(pixel, 0);
            return;
        }
        let a_intr = a_intr_opt.value;
        let s_intr = a_intr.intr_opt.value;
        let x = sizeof(Integrate);
        in.image.Store(pixel, fv4(s_intr.p, 1));
    }
}
