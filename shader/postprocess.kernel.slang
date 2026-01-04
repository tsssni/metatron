import metatron;

namespace mtt {
    struct Constants {
        [vk_binding(0, 0)] RWTexture2D film;
        [vk_binding(1, 0)] RWTexture2D image;
    }
    ParameterBlock<Constants> constants;

    [shader("compute"), numthreads(8, 8, 1)]
    func wave(
        iv3 global_idx: SV_DispatchThreadID,
    ) -> void {
        let pixel = global_idx.xy;
        var size: uv2;
        constants.image.GetDimensions(size.x, size.y);
        if (any(pixel != clamp(pixel, 0, size - 1))) return;
        let film = constants.film.Load(pixel);
        constants.image.Store(pixel, film / max(film.w, math::epsilon));
    }
}
