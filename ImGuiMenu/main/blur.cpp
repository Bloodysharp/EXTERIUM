#include <d3d9.h>
#include <wrl/client.h>

#include "blur.hpp"

#include "blur_x.h"
#include "blur_y.h"

using Microsoft::WRL::ComPtr;

static int backbufferWidth = 0;
static int backbufferHeight = 0;

float blur_animation = 4.f;

static IDirect3DDevice9* device;

[[nodiscard]] static IDirect3DTexture9* createTexture(int width, int height) noexcept
{
    if (!device)
        return nullptr;

    IDirect3DTexture9* texture;
    device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr);
    return texture;
}

static void copyBackbufferToTexture(IDirect3DTexture9* texture, D3DTEXTUREFILTERTYPE filtering) noexcept
{
    if (!device)
        return;

    ComPtr<IDirect3DSurface9> backBuffer;
    if (device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, backBuffer.GetAddressOf()) == D3D_OK) {
        ComPtr<IDirect3DSurface9> surface;
        if (texture->GetSurfaceLevel(0, surface.GetAddressOf()) == D3D_OK)
            device->StretchRect(backBuffer.Get(), nullptr, surface.Get(), nullptr, filtering);
    }
}

static void setRenderTarget(IDirect3DTexture9* rtTexture) noexcept
{
    if (!device)
        return;

    ComPtr<IDirect3DSurface9> surface;
    if (rtTexture->GetSurfaceLevel(0, surface.GetAddressOf()) == D3D_OK)
        device->SetRenderTarget(0, surface.Get());
}



class ShaderProgram {
public:
    ~ShaderProgram()
    {
    }

    void use(float uniform, int location) const noexcept
    {
        device->SetPixelShader(pixelShader.Get());
        const float params[4] = { uniform };
        device->SetPixelShaderConstantF(location, params, 1);
    }

    void init(const BYTE* pixelShaderSrc) noexcept
    {
        if (initialized)
            return;
        initialized = true;

        if (!device)
            return;

        device->CreatePixelShader(reinterpret_cast<const DWORD*>(pixelShaderSrc), pixelShader.GetAddressOf());
    }

private:
    ComPtr<IDirect3DPixelShader9> pixelShader;
    bool initialized = false;
};

class BlurEffect {
public:
    static void draw(ImDrawList* drawList, ImVec2 min, ImVec2 max, float alpha) noexcept
    {
        if (!device || !drawList)
            return;

        instance()._draw(drawList, min, max, alpha);
    }

    static void clearTextures() noexcept
    {
        if (instance().blurTexture1) {
            instance().blurTexture1->Release();
            instance().blurTexture1 = nullptr;
        }
        if (instance().blurTexture2) {
            instance().blurTexture2->Release();
            instance().blurTexture2 = nullptr;
        }
    }
private:
    IDirect3DSurface9* rtBackup = nullptr;
    IDirect3DTexture9* blurTexture1 = nullptr;
    IDirect3DTexture9* blurTexture2 = nullptr;
    ShaderProgram blurShaderX;
    ShaderProgram blurShaderY;

    BlurEffect() = default;
    BlurEffect(const BlurEffect&) = delete;

    ~BlurEffect()
    {
        if (rtBackup)
            rtBackup->Release();
        if (blurTexture1)
            blurTexture1->Release();
        if (blurTexture2)
            blurTexture2->Release();
    }

    static BlurEffect& instance() noexcept
    {
        static BlurEffect blurEffect;
        return blurEffect;
    }

    static void begin(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._begin(); }
    static void firstPass(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._firstPass(); }
    static void secondPass(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._secondPass(); }
    static void end(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._end(); }

    void createTextures() noexcept
    {
        if (!blurTexture1)
            blurTexture1 = createTexture(backbufferWidth / blur_animation, backbufferHeight / blur_animation);
        if (!blurTexture2)
            blurTexture2 = createTexture(backbufferWidth / blur_animation, backbufferHeight / blur_animation);
    }

    void createShaders() noexcept
    {
        blurShaderX.init(blur_x);
        blurShaderY.init(blur_y);
    }

    void _begin() noexcept
    {
        device->GetRenderTarget(0, &rtBackup);

        copyBackbufferToTexture(blurTexture1, D3DTEXF_LINEAR);

        device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

        const D3DMATRIX projection{ {{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f / (backbufferWidth / blur_animation), 1.0f / (backbufferHeight / blur_animation), 0.0f, 1.0f
        }} };
        device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
    }

    void _firstPass() noexcept
    {
        blurShaderX.use(1.0f / (backbufferWidth / blur_animation), 0);
        setRenderTarget(blurTexture2);
    }

    void _secondPass() noexcept
    {
        blurShaderY.use(1.0f / (backbufferHeight / blur_animation), 0);
        setRenderTarget(blurTexture1);
    }

    void _end() noexcept
    {
        device->SetRenderTarget(0, rtBackup);
        rtBackup->Release();

        device->SetPixelShader(nullptr);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    }

    void _draw(ImDrawList* drawList, ImVec2 min, ImVec2 max, float alpha) noexcept
    {


        createTextures();
        createShaders();

        if (!blurTexture1 || !blurTexture2)
            return;

        drawList->AddCallback(&begin, nullptr);
        for (int i = 0; i < 8; ++i) {
            drawList->AddCallback(&firstPass, nullptr);
            drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture1), { -1.0f, -1.0f }, { 1.0f, 1.0f });
            drawList->AddCallback(&secondPass, nullptr);
            drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture2), { -1.0f, -1.0f }, { 1.0f, 1.0f });
        }
        drawList->AddCallback(&end, nullptr);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

        drawList->AddImageRounded(reinterpret_cast<ImTextureID>(blurTexture1), min, max, { min.x / backbufferWidth, min.y / backbufferHeight }, { max.x / backbufferWidth, max.y / backbufferHeight }, IM_COL32(255, 255, 255, 255 * alpha), ImGui::GetStyle().WindowRounding);
    }
};


void blur::set_device(IDirect3DDevice9* device) noexcept
{
    ::device = device;
}

void blur::clear_blur_textures() noexcept
{
    BlurEffect::clearTextures();
}

void blur::device_reset() noexcept
{
    BlurEffect::clearTextures();
}


void blur::new_frame() noexcept
{
    const int width = ImGui::GetIO().DisplaySize.x;
    const int height = ImGui::GetIO().DisplaySize.y;
    if (backbufferWidth != static_cast<int>(width) || backbufferHeight != static_cast<int>(height)) {
        BlurEffect::clearTextures();
        backbufferWidth = static_cast<int>(width);
        backbufferHeight = static_cast<int>(height);
    }
}

void blur::add_blur(ImDrawList* drawList, ImVec2 min, ImVec2 max, float alpha) noexcept
{
    BlurEffect::draw(drawList, min, max, alpha);
}
