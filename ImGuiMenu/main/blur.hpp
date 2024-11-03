#pragma once
#include "imgui.h"

struct ImDrawList;
struct IDirect3DDevice9;

namespace blur
{
    extern void set_device(IDirect3DDevice9* device) noexcept;
    extern void clear_blur_textures() noexcept;
    extern void device_reset() noexcept;
    extern void new_frame() noexcept;
    extern void add_blur(ImDrawList* drawList, ImVec2 min, ImVec2 max, float alpha = 1.f) noexcept;
}
