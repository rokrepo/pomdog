// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "Pomdog/Experimental/GUI/ToggleSwitch.hpp"
#include "Pomdog/Experimental/GUI/DrawingContext.hpp"
#include "Pomdog/Experimental/GUI/PointerPoint.hpp"
#include "Pomdog/Experimental/GUI/UIEventDispatcher.hpp"
#include "Pomdog/Experimental/GUI/UIHelper.hpp"
#include "Pomdog/Experimental/Tween/EasingHelper.hpp"

namespace Pomdog::GUI {
namespace {

const Color toggleOnInnerColor = {162, 122, 16, 255};
const Color toggleOnThumbColor = {252, 252, 250, 255};
const Color toggleOffInnerColor = {160, 160, 160, 255};
const Color toggleOffThumbColor = {81, 81, 81, 255};
const Color disabledInnerColor = {81, 81, 81, 255};
const Color disabledThumbColor = {137, 137, 137, 255};

constexpr float thumbRadius = 7.0f;
constexpr float thumbPadding = 2.0f;
constexpr int segments = 12;

} // unnamed namespace

ToggleSwitch::ToggleSwitch(const std::shared_ptr<UIEventDispatcher>& dispatcher)
    : Widget(dispatcher)
    , toggleAnimation(ToggleAnimation::Stopped)
    , animationDuration(0.0f)
    , isOn(true)
    , isEnabled(true)
{
    SetSize(48, static_cast<int>(thumbRadius) * 2);
    SetCursor(MouseCursor::PointingHand);
}

bool ToggleSwitch::IsOn() const
{
    return isOn;
}

void ToggleSwitch::SetOn(bool isOnIn)
{
    this->isOn = isOnIn;
}

bool ToggleSwitch::IsEnabled() const
{
    return isEnabled;
}

void ToggleSwitch::SetEnabled(bool isEnabledIn)
{
    this->isEnabled = isEnabledIn;
    if (isEnabled) {
        SetCursor(MouseCursor::PointingHand);
    }
    else {
        ResetCursor();
    }
}

HorizontalAlignment ToggleSwitch::GetHorizontalAlignment() const noexcept
{
    return HorizontalAlignment::Right;
}

VerticalAlignment ToggleSwitch::GetVerticalAlignment() const noexcept
{
    return VerticalAlignment::Top;
}

void ToggleSwitch::OnEnter()
{
}

void ToggleSwitch::OnPointerPressed([[maybe_unused]] const PointerPoint& pointerPoint)
{
}

void ToggleSwitch::OnPointerReleased([[maybe_unused]] const PointerPoint& pointerPoint)
{
    if (!isEnabled) {
        return;
    }

    isOn = !isOn;

    if (isOn) {
        toggleAnimation = ToggleAnimation::OffToOn;
    }
    else {
        toggleAnimation = ToggleAnimation::OnToOff;
    }
    animationDuration = 0.0f;

    Toggled(isOn);
}

void ToggleSwitch::UpdateAnimation(const Duration& frameDuration)
{
    if (toggleAnimation == ToggleAnimation::Stopped) {
        return;
    }

    constexpr double animationSpeed = 7.0;
    animationDuration += static_cast<float>(frameDuration.count() * animationSpeed);
    if (animationDuration > 1.0f) {
        toggleAnimation = ToggleAnimation::Stopped;
        animationDuration = 0.0f;
    }
}

void ToggleSwitch::Draw(DrawingContext& drawingContext)
{
    auto globalPos = UIHelper::ProjectToWorldSpace(GetPosition(), drawingContext.GetCurrentTransform());
    auto primitiveBatch = drawingContext.GetPrimitiveBatch();

    auto innerColor = Color{109, 109, 109, 255};
    auto thumbColor = Color::White;

    if (!isEnabled) {
        innerColor = disabledInnerColor;
        thumbColor = disabledThumbColor;
    }

    if (isEnabled) {
        if (isOn) {
            innerColor = toggleOnInnerColor;
            thumbColor = toggleOnThumbColor;
        }
        else {
            innerColor = toggleOffInnerColor;
            thumbColor = toggleOffThumbColor;
        }
    }

    const auto transformOffset = MathHelper::ToVector2(globalPos);

    primitiveBatch->DrawArc(
        transformOffset + Vector2{
                              static_cast<float>(GetWidth()) - (thumbRadius + thumbPadding),
                              (thumbRadius + thumbPadding)},
        thumbRadius + thumbPadding, MathHelper::ToRadians(270.0f), MathHelper::ToRadians(180.0f), segments, innerColor);

    primitiveBatch->DrawArc(
        transformOffset + Vector2{
                              (thumbRadius + thumbPadding),
                              (thumbRadius + thumbPadding)},
        thumbRadius + thumbPadding, MathHelper::ToRadians(90.0f), MathHelper::ToRadians(180.0f), segments, innerColor);

    primitiveBatch->DrawRectangle(
        Matrix3x2::Identity,
        Vector2{(thumbRadius + thumbPadding), 0.0f} + MathHelper::ToVector2(globalPos),
        GetWidth() - (thumbRadius + thumbPadding) * 2.0f,
        (thumbRadius + thumbPadding) * 2.0f,
        innerColor);

    const auto onPos = Vector2{
        static_cast<float>(GetWidth()) - (thumbRadius + thumbPadding),
        (thumbRadius + thumbPadding)};
    const auto offPos = Vector2{
        (thumbRadius + thumbPadding),
        (thumbRadius + thumbPadding)};

    Vector2 circlePos;
    if (isOn) {
        circlePos = onPos;
    }
    else {
        circlePos = offPos;
    }
    switch (toggleAnimation) {
    case ToggleAnimation::OnToOff:
        circlePos = Vector2::Lerp(offPos, onPos, 1.0f - Easings::EaseSine::InOut(animationDuration));
        break;
    case ToggleAnimation::OffToOn:
        circlePos = Vector2::Lerp(offPos, onPos, Easings::EaseSine::InOut(animationDuration));
        break;
    case ToggleAnimation::Stopped:
        break;
    }

    primitiveBatch->DrawCircle(
        transformOffset + circlePos,
        thumbRadius,
        segments,
        thumbColor);
    primitiveBatch->Flush();

    auto spriteBatch = drawingContext.GetSpriteBatch();
    auto spriteFont = drawingContext.GetFont(FontWeight::Bold, FontSize::Small);

    if (isOn) {
        auto textPosition = MathHelper::ToVector2(globalPos) + Vector2{thumbPadding, thumbPadding} + Vector2{9.0f, 3.0f};
        spriteFont->Draw(*spriteBatch, "ON", textPosition, toggleOnThumbColor);
    }
    else {
        auto textPosition = MathHelper::ToVector2(globalPos) + Vector2{static_cast<float>(GetWidth()) - (thumbRadius + thumbPadding), thumbPadding} + Vector2{-19.0f, 3.0f};
        spriteFont->Draw(*spriteBatch, "OFF", textPosition, Color{250, 250, 250, 255});
    }

    spriteBatch->Flush();
}

} // namespace Pomdog::GUI
