#include <metatron/resource/light/sunsky.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <ranges>
#include <fstream>

namespace mtt::light {
	std::vector<f32> Sunsky_Light::sky_params;
	std::vector<f32> Sunsky_Light::sky_radiance;
	std::vector<f32> Sunsky_Light::sun_radiance;
	std::vector<f32> Sunsky_Light::sun_limb;

	auto Sunsky_Light::init() noexcept -> void {
		auto read = []
		<typename T, typename U>
		(std::vector<T>& storage, std::vector<U>&& intermediate, std::string const& file) -> void {
			auto& fs = stl::filesystem::instance();
			auto prefix = std::string{"sunsky/"};
			auto postfix = std::string{".bin"};
			auto path = prefix + file + postfix;
			MTT_OPT_OR_CALLBACK(data, fs.find(path), {
				std::println("{} not found", path);
				std::abort();
			});

			auto f = std::ifstream{data, std::ios::binary};
			if (!f.is_open()) {
				std::println("{} not open", path);
				std::abort();
			}

			auto header = std::string(3, '\0');
			auto version = 0u;
			if (false
			|| !f.read(header.data(), 3)
			|| (true
			&& header != "SKY"
			&& header != "SUN")) {
				std::println("{} has wrong header {}", path, header);
				std::abort();
			}
			f.read((char*)(&version), sizeof(version));

			auto dims = 0uz;
			f.read((char*)(&dims), sizeof(dims));
			auto elems = 1uz;
			auto shape = std::vector<usize>(dims);
			for (auto& s: shape) {
				f.read((char*)(&s), sizeof(s));
				if (!s) {
					std::println("{} has zero dimension", path);
					std::abort();
				}
				elems *= s;
			}

			data.resize(elems);
			intermediate.resize(elems);
			f.read((char*)(intermediate.data()), intermediate.size() * sizeof(U));
			f.close();
			std::ranges::transform(intermediate, data.begin(), [](U x){return T(x);});
		};

		read(sky_params, std::vector<f64>{}, "sky-params");
		read(sky_radiance, std::vector<f64>{}, "sky-radiance");
		read(sun_radiance, std::vector<f64>{}, "sun-radiance");
		read(sun_limb, std::vector<f64>{}, "sun-limb");
	}

	auto Sunsky_Light::operator()(
		eval::Context const& ctx
	) const noexcept -> std::optional<Interaction> {
		return {};
	}

	auto Sunsky_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const noexcept -> std::optional<Interaction> {
		return {};
	}

	auto Sunsky_Light::flags() const noexcept -> Flags {
		return Flags::inf;
	}
}
