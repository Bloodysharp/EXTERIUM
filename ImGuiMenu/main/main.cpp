#include "main.h"

#include "imgui_freetype.h"

static int slider_int[25];

static bool checkbox[25];

float color_edit[10][4];

int keybind[10];

static int combo[10];

static int iTabs = 0;
static int iSubTabs = 0;

int main(int, char**)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

 
    ImFontConfig config;

    ImFontConfig config2;
    config.MergeMode = false; // каждый шрифт будет представлен отдельно

    config2.FontBuilderFlags = ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_Bold; // использовать более легкое намекание

    config.FontBuilderFlags = ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_LightHinting; // использовать более легкое намекание
    config.OversampleH = 25; // увеличить горизонтальное субпиксельное сглаживание
    config.OversampleV = 25; // увеличить вертикальное субпиксельное сглаживание
    config.PixelSnapH = true; // выровнять глифы по границам пикселей

    io.Fonts->AddFontFromMemoryTTF(&NunitoMedium, sizeof NunitoMedium, 21, &config, io.Fonts->GetGlyphRangesCyrillic());

    medium_font = io.Fonts->AddFontFromMemoryTTF(&NunitoMedium, sizeof NunitoMedium, 18, &config, io.Fonts->GetGlyphRangesCyrillic());

    small_font = io.Fonts->AddFontFromMemoryTTF(&NunitoMedium, sizeof NunitoMedium, 17, &config, io.Fonts->GetGlyphRangesCyrillic());

    icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 18, &config, io.Fonts->GetGlyphRangesCyrillic());

    icon_big_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 23, &config, io.Fonts->GetGlyphRangesCyrillic());

    logo_font = io.Fonts->AddFontFromMemoryTTF(&NunitoMedium, sizeof NunitoMedium, 25, &config2, io.Fonts->GetGlyphRangesCyrillic());

    small_icon_font = io.Fonts->AddFontFromMemoryTTF(&NunitoMedium, sizeof NunitoMedium, 15, &config, io.Fonts->GetGlyphRangesCyrillic());

    arrow_icons = io.Fonts->AddFontFromMemoryTTF(&arrowicon, sizeof arrowicon, 18, &config, io.Fonts->GetGlyphRangesCyrillic());

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    ImVec4 clear_color = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);

    ImGuiStyle& s = ImGui::GetStyle();

    s.FramePadding = ImVec2(10, 10);
    s.ItemSpacing = ImVec2(10, 5);
    s.FrameRounding = 5.f;
    s.WindowRounding = 15.f;
    s.WindowBorderSize = 0.f;
    s.PopupBorderSize = 0.f;
    s.WindowPadding = ImVec2(0, 0);
    s.ChildBorderSize = 0.0001f;
    s.Colors[ImGuiCol_WindowBg] = winbg_color;
    s.Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.f, 0.f);
    s.WindowShadowSize = 0;
    s.PopupRounding = 5.f;
    s.PopupBorderSize = 1.3f;
    s.ScrollbarSize = 4.f;
    s.ScrollbarRounding = 10.f;

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            blur::set_device(g_pd3dDevice);
            blur::new_frame();
            LoadImages();

            ImGui::GetBackgroundDrawList()->AddImage(bg, ImVec2(0, 0), ImVec2(1920, 1080), ImVec2(0, 0), ImVec2(1, 1), main_color);

            ImGui::SetNextWindowSize(ImVec2(827, 604));
            ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
            {
                anim_speed = ImGui::GetIO().DeltaTime * 12.f;

                auto draw = ImGui::GetWindowDrawList();
                const auto& p = ImGui::GetWindowPos();

                blur::add_blur(ImGui::GetBackgroundDrawList(), p, ImVec2(p.x + 827, p.y + 604), 1.f);

                draw->AddRectFilled(p + ImVec2(0, 55), p + ImVec2(827, 604), winbg_color, s.WindowRounding, ImDrawFlags_RoundCornersBottom);

                Trinage_background();

                ImGui::PushClipRect(p + ImVec2(0, 55), p + ImVec2(187, 604), true);
                blur::add_blur(ImGui::GetWindowDrawList(), p, ImVec2(p.x + 827, p.y + 604), 1.f);
                ImGui::PopClipRect();

                ImGui::PushClipRect(p, p + ImVec2(827, 55), true);
                blur::add_blur(ImGui::GetWindowDrawList(), p, ImVec2(p.x + 827, p.y + 604), 1.f);
                ImGui::PopClipRect();

                draw->AddRectFilled(p + ImVec2(0, 55), p + ImVec2(187, 604), ImColor(13, 14, 16, 229), s.WindowRounding, ImDrawFlags_RoundCornersBottomLeft);

                draw->AddRectFilled(p, p + ImVec2(827, 55), ImColor(5, 5, 5, 45), s.WindowRounding, ImDrawFlags_RoundCornersTop);
                draw->AddRectFilled(p + ImVec2(0, 52), p + ImVec2(827, 55), main_color, s.WindowRounding);

                ImGui::PushFont(logo_font);

                draw->AddText(center_text(p, p + ImVec2(827, 52), "Your Menu"), ImColor(1.f, 1.f, 1.f, 1.f), "Yours Menu");

                ImGui::PopFont();

                ImGui::SetCursorPos(ImVec2(10, 70));

                ImGui::BeginGroup();

                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 0.3f), "Aimbot");

                ImGui::Tab("Legitbot", "9", &iTabs, 0);
                ImGui::Tab("Ragebot", "0", &iTabs, 1);

                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 0.3f), "Visuals");

                ImGui::Tab("Wallhack", "8", &iTabs, 2);
                ImGui::Tab("Chams", "5", &iTabs, 3);

                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 0.3f), "Others");

                ImGui::Tab("Misc", "1", &iTabs, 4);
                ImGui::Tab("Lua API", "2", &iTabs, 5);

                ImGui::EndGroup();

                ImGui::SetCursorPos(ImVec2(197, 72));
                ImGui::BeginGroup();
                {
                    ImGui::BeginChild("Frame##1", ImVec2(frame_size.x, 532), true);
                    {

                        ImGui::Checkbox("Enable", &checkbox[50]);
                        ImGui::SliderInt("FOV", &slider_int[50], 0, 100);
                        ImGui::Checkbox("Auto wall", &checkbox[51]);
                        ImGui::Checkbox("Auto fire", &checkbox[52]);
                        ImGui::Checkbox("Silent aim", &checkbox[53]);
                        ImGui::Checkbox("Quick peek", &checkbox[54]);
                        ImGui::ColorEdit4("Glow color", (float*)color_edit[0], ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
                        ImGui::ColorEdit4("Main color", (float*)&main_color.Value, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);

                        const char* bones[] = {
                            "Head",
                            "Neck",
                            "Spine1",
                            "Spine2",
                            "Spine3",
                            "Spine4",
                            "RightClavicle",
                            "RightUpperArm",
                            "RightForearm",
                        };

                        ImGui::Combo("Bones", &combo[0], bones, IM_ARRAYSIZE(bones));
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild("Frame##2", ImVec2(frame_size.x, 532), true);
                    {
                        ImGui::Checkbox("Enable", &checkbox[0]);
                        //Pickerbox("Bullet trace", &checkbox[6], (float*)color_edit);
                        ImGui::Checkbox("Silent Aim", &checkbox[1]);
                        ImGui::Checkbox("Target Dead", &checkbox[2]);
                        ImGui::Checkbox("Magic Bullet", &checkbox[3]);
                        ImGui::Checkbox("Preview Aimbot FOV", &checkbox[4]);
                        ImGui::Checkbox("Filled FOV", &checkbox[5]);

                        Pickerbox("Aimbot FOV", &checkbox[10], (float*)color_edit[1]);

                        Keybindbox("Keybind", &checkbox[11], &keybind[0]);

                        ImGui::SliderInt("Smooth flow", &slider_int[0], 0, 100);

                        const char* bones[] = {
                            "Head",
                            "Neck",
                            "Spine1",
                            "Spine2",
                            "Spine3",
                            "Spine4",
                            "RightClavicle",
                            "RightUpperArm",
                            "RightForearm",
                        };

                        ImGui::Combo("Bones Enemy", &combo[2], bones, IM_ARRAYSIZE(bones));
                    }
                    ImGui::EndChild();
                }
                ImGui::EndGroup();
                foremenu_drawlist = ImGui::GetWindowDrawList();
            }
            ImGui::End();
        }

        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
