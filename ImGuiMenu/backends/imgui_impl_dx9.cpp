// dear imgui: Renderer for DirectX9
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as ImTextureID. Read the FAQ about ImTextureID!
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bits indices.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2019-05-29: DirectX9: Added support for large mesh (64K+ vertices), enable ImGuiBackendFlags_RendererHasVtxOffset flag.
//  2019-04-30: DirectX9: Added support for special ImDrawCallback_ResetRenderState callback to reset render state.
//  2019-03-29: Misc: Fixed erroneous assert in ImGui_ImplDX9_InvalidateDeviceObjects().
//  2019-01-16: Misc: Disabled fog before drawing UI's. Fixes issue #2288.
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-06-08: Misc: Extracted imgui_impl_dx9.cpp/.h away from the old combined DX9+Win32 example.
//  2018-06-08: DirectX9: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-05-07: Render: Saving/restoring Transform because they don't seem to be included in the StateBlock. Setting shading mode to Gouraud.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed ImGui_ImplDX9_RenderDrawData() in the .h file so you can call it yourself.
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <memory>

#include "imgui.h"
#include "imgui_impl_dx9.h"

const BYTE default_vs[] =
{
	  0,   2, 254, 255, 254, 255,
	155,   0,  68,  66,  85,  71,
	 40,   0,   0,   0,  64,   2,
	  0,   0,   0,   0,   0,   0,
	  1,   0,   0,   0, 116,   0,
	  0,   0,  16,   0,   0,   0,
	120,   0,   0,   0,   3,   0,
	  0,   0,   4,   2,   0,   0,
	248,   0,   0,   0,  67,  58,
	 92,  85, 115, 101, 114, 115,
	 92, 108, 111, 108, 111, 107,
	 92,  68, 101, 115, 107, 116,
	111, 112,  92,  71,  79,  69,
	 83,  80,  45, 109,  97, 115,
	116, 101, 114,  92,  71,  79,
	 69,  83,  80,  92,  82, 101,
	115, 111, 117, 114,  99, 101,
	115,  92,  83, 104,  97, 100,
	101, 114, 115,  92, 100, 101,
	102,  97, 117, 108, 116,  95,
	118, 115,  46, 104, 108, 115,
	108,   0,  40,   0,   0,   0,
	  0,   0, 255, 255,   0,   3,
	  0,   0,   0,   0, 255, 255,
	 24,   3,   0,   0,   0,   0,
	255, 255,  36,   3,   0,   0,
	  0,   0, 255, 255,  48,   3,
	  0,   0,  20,   0,   0,   0,
	 60,   3,   0,   0,  20,   0,
	  0,   0,  76,   3,   0,   0,
	 20,   0,   0,   0,  92,   3,
	  0,   0,  20,   0,   0,   0,
	108,   3,   0,   0,  20,   0,
	  0,   0, 124,   3,   0,   0,
	 20,   0,   0,   0, 136,   3,
	  0,   0,  20,   0,   0,   0,
	152,   3,   0,   0,  21,   0,
	  0,   0, 168,   3,   0,   0,
	 22,   0,   0,   0, 180,   3,
	  0,   0,  23,   0,   0,   0,
	192,   3,   0,   0,  23,   0,
	  0,   0, 204,   3,   0,   0,
	 23,   0,   0,   0, 216,   3,
	  0,   0, 109,  97, 105, 110,
	  0, 105, 110, 112, 117, 116,
	  0, 112, 111, 115,   0, 171,
	  1,   0,   3,   0,   1,   0,
	  2,   0,   1,   0,   0,   0,
	  0,   0,   0,   0,  99, 111,
	108,   0,   1,   0,   3,   0,
	  1,   0,   4,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	117, 118,   0, 171,   3,   1,
	  0,   0,   8,   1,   0,   0,
	 24,   1,   0,   0,  28,   1,
	  0,   0,  44,   1,   0,   0,
	  8,   1,   0,   0,   5,   0,
	  0,   0,   1,   0,   8,   0,
	  1,   0,   3,   0,  48,   1,
	  0,   0,   1,   0,   0,   0,
	  0,   0,   1,   0, 255, 255,
	255, 255,   2,   0,   0,   0,
	  2,   0,   3,   0,   4,   0,
	  5,   0,   3,   0,   0,   0,
	  6,   0,   7,   0, 255, 255,
	255, 255,   3,   1,   0,   0,
	 28,   1,   0,   0,  24,   1,
	  0,   0,  28,   1,   0,   0,
	 44,   1,   0,   0,   8,   1,
	  0,   0,   5,   0,   0,   0,
	  1,   0,  10,   0,   1,   0,
	  3,   0, 124,   1,   0,   0,
	 13,   0,   0,   0,   0,   0,
	  1,   0,   2,   0,   3,   0,
	 14,   0,   0,   0,   4,   0,
	  5,   0,   6,   0,   7,   0,
	 15,   0,   0,   0,   8,   0,
	  9,   0, 255, 255, 255, 255,
	111, 117, 116, 112, 117, 116,
	  0, 171,   5,   0,   0,   0,
	  1,   0,  10,   0,   1,   0,
	  3,   0, 124,   1,   0,   0,
	 10,   0,   0,   0,   0,   0,
	  1,   0,   2,   0,   3,   0,
	 11,   0,   0,   0,   4,   0,
	  5,   0,   6,   0,   7,   0,
	 12,   0,   0,   0,   8,   0,
	  9,   0, 255, 255, 255, 255,
	248,   0,   0,   0, 253,   0,
	  0,   0,  72,   1,   0,   0,
	  3,   0,   0,   0,  88,   1,
	  0,   0,   0,   0,   0,   0,
	248,   0,   0,   0, 148,   1,
	  0,   0,   3,   0,   0,   0,
	164,   1,   0,   0,   0,   0,
	  0,   0, 200,   1,   0,   0,
	208,   1,   0,   0,   3,   0,
	  0,   0, 224,   1,   0,   0,
	 77, 105,  99, 114, 111, 115,
	111, 102, 116,  32,  40,  82,
	 41,  32,  72,  76,  83,  76,
	 32,  83, 104,  97, 100, 101,
	114,  32,  67, 111, 109, 112,
	105, 108, 101, 114,  32,  49,
	 48,  46,  49,   0, 254, 255,
	 34,   0,  67,  84,  65,  66,
	 28,   0,   0,   0,  91,   0,
	  0,   0,   0,   2, 254, 255,
	  1,   0,   0,   0,  28,   0,
	  0,   0,   5,   1,   0,   0,
	 84,   0,   0,   0,  48,   0,
	  0,   0,   2,   0,   0,   0,
	  4,   0,   0,   0,  68,   0,
	  0,   0,   0,   0,   0,   0,
	112, 114, 111, 106, 101,  99,
	116, 105, 111, 110,  77,  97,
	116, 114, 105, 120,   0, 171,
	171, 171,   3,   0,   3,   0,
	  4,   0,   4,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	118, 115,  95,  50,  95,  48,
	  0,  77, 105,  99, 114, 111,
	115, 111, 102, 116,  32,  40,
	 82,  41,  32,  72,  76,  83,
	 76,  32,  83, 104,  97, 100,
	101, 114,  32,  67, 111, 109,
	112, 105, 108, 101, 114,  32,
	 49,  48,  46,  49,   0, 171,
	 81,   0,   0,   5,   4,   0,
	 15, 160,   0,   0,   0,   0,
	  0,   0, 128,  63,   0,   0,
	  0,   0,   0,   0,   0,   0,
	 31,   0,   0,   2,   0,   0,
	  0, 128,   0,   0,  15, 144,
	 31,   0,   0,   2,  10,   0,
	  0, 128,   1,   0,  15, 144,
	 31,   0,   0,   2,   5,   0,
	  0, 128,   2,   0,  15, 144,
	  5,   0,   0,   3,   0,   0,
	 15, 128,   0,   0,   0, 144,
	  0,   0, 228, 160,   5,   0,
	  0,   3,   1,   0,  15, 128,
	  0,   0,  85, 144,   1,   0,
	228, 160,   2,   0,   0,   3,
	  0,   0,  15, 128,   0,   0,
	228, 128,   1,   0, 228, 128,
	  2,   0,   0,   3,   0,   0,
	 15, 128,   0,   0, 228, 128,
	  4,   0,   0, 160,   1,   0,
	  0,   2,   1,   0,   2, 128,
	  4,   0,  85, 160,   5,   0,
	  0,   3,   1,   0,  15, 128,
	  1,   0,  85, 128,   3,   0,
	228, 160,   2,   0,   0,   3,
	  0,   0,  15, 128,   0,   0,
	228, 128,   1,   0, 228, 128,
	  1,   0,   0,   2,   1,   0,
	 15, 128,   1,   0, 228, 144,
	  1,   0,   0,   2,   2,   0,
	  3, 128,   2,   0, 228, 144,
	  1,   0,   0,   2,   0,   0,
	 15, 192,   0,   0, 228, 128,
	  1,   0,   0,   2,   0,   0,
	 15, 208,   1,   0, 228, 128,
	  1,   0,   0,   2,   0,   0,
	  3, 224,   2,   0, 228, 128,
	255, 255,   0,   0
};

// DirectX data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9  g_pVB = NULL;
static LPDIRECT3DINDEXBUFFER9   g_pIB = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

static IDirect3DVertexDeclaration9* vertexDeclaration = nullptr;
static IDirect3DVertexShader9* vertexShader = nullptr;

static void ImGui_ImplDX9_SetupRenderState(const ImDrawData* draw_data)
{
	// Setup viewport
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = (DWORD)draw_data->DisplaySize.x;
	vp.Height = (DWORD)draw_data->DisplaySize.y;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	g_pd3dDevice->SetViewport(&vp);

	// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient)
	g_pd3dDevice->SetPixelShader(nullptr);
	g_pd3dDevice->SetVertexShader(vertexShader);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	g_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, false);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// Setup orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	// Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
	{
		float L = draw_data->DisplayPos.x + 0.5f;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
		float T = draw_data->DisplayPos.y + 0.5f;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
		D3DMATRIX mat_projection =
		{ { {
			2.0f / (R - L),   0.0f,         0.0f,  0.0f,
			0.0f,         2.0f / (T - B),   0.0f,  0.0f,
			0.0f,         0.0f,         0.5f,  0.0f,
			(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
		} } };
		g_pd3dDevice->SetVertexShaderConstantF(0, &mat_projection.m[0][0], 4);
	}
}

// Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
void ImGui_ImplDX9_RenderDrawData(const ImDrawData* draw_data)
{
	// Avoid rendering when minimized
	if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
		return;

	// Create and grow buffers if needed
	if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
	{
		if (g_pVB) { g_pVB->Release(); g_pVB = nullptr; }
		g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		if (g_pd3dDevice->CreateVertexBuffer(g_VertexBufferSize * sizeof(ImDrawVert), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &g_pVB, nullptr) < 0)
			return;
	}
	if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
	{
		if (g_pIB) { g_pIB->Release(); g_pIB = nullptr; }
		g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		if (g_pd3dDevice->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, nullptr) < 0)
			return;
	}

	// Backup the DX9 state
	IDirect3DStateBlock9* d3d9_state_block = nullptr;
	if (g_pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
		return;

	if (d3d9_state_block->Capture() != D3D_OK)
		return;

	ImDrawVert* vtx_dst;
	ImDrawIdx* idx_dst;
	if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(ImDrawVert)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
		return;
	if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
		return;

	for (int n = 0; n < draw_data->CmdListsCount; ++n) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		vtx_dst += cmd_list->VtxBuffer.Size;
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		idx_dst += cmd_list->IdxBuffer.Size;
	}

	g_pVB->Unlock();
	g_pIB->Unlock();

	g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(ImDrawVert));
	g_pd3dDevice->SetIndices(g_pIB);
	g_pd3dDevice->SetVertexDeclaration(vertexDeclaration);

	// Setup desired DX state
	ImGui_ImplDX9_SetupRenderState(draw_data);

	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	ImVec2 clip_off = draw_data->DisplayPos;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != nullptr)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					ImGui_ImplDX9_SetupRenderState(draw_data);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				const RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
				const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->TextureId;
				g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, texture == g_FontTexture ? D3DTOP_SELECTARG2 : D3DTOP_MODULATE);
				g_pd3dDevice->SetTexture(0, texture);
				g_pd3dDevice->SetScissorRect(&r);
				g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}



	// Restore the DX9 state
	d3d9_state_block->Apply();
	d3d9_state_block->Release();
}

void ImGui_ImplDX9_DestroyFontsTexture()
{
	if (g_FontTexture) { g_FontTexture->Release(); g_FontTexture = nullptr; ImGui::GetIO().Fonts->TexID = nullptr; } // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
}

bool ImGui_ImplDX9_Init(IDirect3DDevice9* device)
{
	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "imgui_impl_dx9";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

	LoadLibraryW(L"d3d9"); // increment d3d9.dll reference count

	g_pd3dDevice = device;
	g_pd3dDevice->AddRef();
	return true;
}

void ImGui_ImplDX9_Shutdown()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

static bool ImGui_ImplDX9_CreateFontsTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);


	// Upload texture to graphics system
	g_FontTexture = nullptr;
	if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, &g_FontTexture, nullptr) < 0)
		return false;
	D3DLOCKED_RECT tex_locked_rect;
	if (g_FontTexture->LockRect(0, &tex_locked_rect, nullptr, D3DLOCK_DISCARD) != D3D_OK)
		return false;
	for (int y = 0; y < height; y++)
		memcpy((unsigned char*)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + width * y, width);
	g_FontTexture->UnlockRect(0);

	// Store our identifier
	io.Fonts->TexID = (ImTextureID)g_FontTexture;

	return true;
}

bool ImGui_ImplDX9_CreateDeviceObjects()
{
	if (!g_pd3dDevice)
		return false;

	if (!ImGui_ImplDX9_CreateFontsTexture())
		return false;

	if (!vertexDeclaration) {
		constexpr D3DVERTEXELEMENT9 elements[]{
			{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			D3DDECL_END()
		};
		g_pd3dDevice->CreateVertexDeclaration(elements, &vertexDeclaration);
	}

	if (!vertexShader)
		g_pd3dDevice->CreateVertexShader(reinterpret_cast<const DWORD*>(default_vs), &vertexShader);

	return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects()
{
	if (!g_pd3dDevice)
		return;
	if (g_pVB) { g_pVB->Release(); g_pVB = nullptr; }
	if (g_pIB) { g_pIB->Release(); g_pIB = nullptr; }
	if (vertexDeclaration) { vertexDeclaration->Release(); vertexDeclaration = nullptr; }
	if (vertexShader) { vertexShader->Release(); vertexShader = nullptr; }
	ImGui_ImplDX9_DestroyFontsTexture();
}

void ImGui_ImplDX9_NewFrame()
{
	if (!g_FontTexture)
		ImGui_ImplDX9_CreateDeviceObjects();
}

ImTextureID ImGui_CreateTextureRGBA(int width, int height, const unsigned char* data)
{
	IDirect3DTexture9* texture;
	if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr) != D3D_OK)
		return nullptr;

	D3DLOCKED_RECT lockedRect;
	if (texture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD) != D3D_OK) {
		texture->Release();
		return nullptr;
	}

	for (int y = 0; y < height; ++y) {
		const auto dest = reinterpret_cast<std::uint32_t*>((unsigned char*)lockedRect.pBits + lockedRect.Pitch * y);
		std::memcpy(dest, data + width * 4 * y, width * 4);

		for (int i = 0; i < width; ++i) {
			// RGBA --> BGRA
			auto color = dest[i];
			color = (color & 0xFF00FF00) | ((color & 0xFF0000) >> 16) | ((color & 0xFF) << 16);
			dest[i] = color;
		}
	}

	texture->UnlockRect(0);
	return texture;
}

void ImGui_DestroyTexture(ImTextureID texture)
{
	reinterpret_cast<IDirect3DTexture9*>(texture)->Release();
}
