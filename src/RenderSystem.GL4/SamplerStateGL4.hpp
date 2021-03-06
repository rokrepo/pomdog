// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "OpenGLPrerequisites.hpp"
#include "../RenderSystem/NativeSamplerState.hpp"
#include "../Utility/Tagged.hpp"
#include "Pomdog/Graphics/detail/ForwardDeclarations.hpp"
#include <optional>

namespace Pomdog::Detail::GL4 {

using SamplerObjectGL4 = Tagged<GLuint, SamplerState>;

class SamplerStateGL4 final : public NativeSamplerState {
public:
    explicit SamplerStateGL4(const SamplerDescription& description);

    ~SamplerStateGL4() override;

    void Apply(int index);

private:
    std::optional<SamplerObjectGL4> samplerObject;
};

} // namespace Pomdog::Detail::GL4
