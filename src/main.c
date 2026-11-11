/*
 * Lufia II: Rise of the Sinistrals — desktop host shim.
 *
 * Identity, widescreen FOV, aspect handling and hooks.
 * The host itself is the framework's (snesrecomp/runner/src/desktop/host_main.h).
 */

#include "host_main.h"
#include "game_rtl.h"
#include "snesrecomp_rom_identity.h"
#include "desktop/config.h"
#include "snes/ppu.h"
#include "snes/ws_shadow.h"
#include "widescreen.h"

#ifndef __ANDROID__
#define SDL_MAIN_HANDLED 1
#endif
#include "desktop/sdl_compat.h"
#ifdef __ANDROID__
#define main SDL_main
#endif

#ifndef SNES_GAME_VERSION
#define SNES_GAME_VERSION "1.0.0"
#endif

extern Ppu *g_ppu;
extern uint8 g_ram[0x20000];

static const char *const kLufia2AspectLabels[] = {
    "4:3 (Original Cl\xc3\xa1sico)",
    "16:9 Panor\xc3\xa1mico (Hack FOV)",
    "16:10 Panor\xc3\xa1mico (Steam Deck)",
    "Pantalla Completa (Estirado)",
};

static void Lufia2PrepareFrame(int dw, int dh, int *w, int *h)
{
    (void)dw; (void)dh;
    if (g_config.display_aspect == 1) {
        *w = 384; /* 16:9 widescreen canvas: 256 + 2 * 64 */
    } else if (g_config.display_aspect == 2) {
        *w = 352; /* 16:10 widescreen canvas: 256 + 2 * 48 (Steam Deck 1280x800) */
    } else {
        *w = 256;
    }
    *h = 224;
}

static void Lufia2ComputeViewport(int frame_w, int frame_h, int drawable_w, int drawable_h,
                                     SnesDisplayViewport *viewport)
{
    (void)frame_w; (void)frame_h;
    if (!viewport) return;
    if (g_config.display_aspect == 0) {
        /* 4:3 Original Clásico con barras laterales en pantallas panorámicas */
        *viewport = SnesDisplayAspect_FitViewport(4.0 / 3.0, drawable_w, drawable_h);
    } else if (g_config.display_aspect == 1) {
        /* 16:9 Panorámico con lienzo extendido a resolución nativa */
        *viewport = SnesDisplayAspect_FitViewport(16.0 / 9.0, drawable_w, drawable_h);
    } else if (g_config.display_aspect == 2) {
        /* 16:10 Panorámico nativo para Steam Deck (1280x800) y pantallas 16:10 */
        *viewport = SnesDisplayAspect_FitViewport(16.0 / 10.0, drawable_w, drawable_h);
    } else {
        /* Pantalla Completa estirada sin franjas */
        viewport->x = 0;
        viewport->y = 0;
        viewport->width = drawable_w > 0 ? drawable_w : 1;
        viewport->height = drawable_h > 0 ? drawable_h : 1;
    }
}

static void Lufia2BeginFrame(unsigned number)
{
    (void)number;
    if (!g_ppu) return;
    PpuSetPal(g_ppu, false);

    /* If mode is 4:3 or Stretched, disable extra wide PPU rasterization */
    if (g_config.ignore_aspect_ratio || (g_config.display_aspect != 1 && g_config.display_aspect != 2)) {
        PpuSetExtraSpace(g_ppu, 0);
        return;
    }

    int budget = (g_config.display_aspect == 2) ? 48 : 64;
    PpuSetExtraSpaceCentered(g_ppu, budget);
}

static const SnesDesktopHostGame kGameHost = {
    .display_name        = "Lufia II: Rise of the Sinistrals",
    .window_title        = "Lufia II: Rise of the Sinistrals Recompiled",
    .region              = SNESRECOMP_ROM_REGION,
    .rom_file            = SNESRECOMP_ROM_FILE,
    .expected_sha256_hex = SNESRECOMP_ROM_EXPECTED_SHA256,
    .expected_crc32_hex  = SNESRECOMP_ROM_EXPECTED_CRC32,
    .game_id             = SNESRECOMP_ROM_GAME_ID,
    .build_version       = SNES_GAME_VERSION,
    .game_info           = &kGameInfo,
    .num_players         = 1,
    .sram_path           = "saves/save.srm",
    .widescreen_supported = 1,
    .native_widescreen   = 1,
    .display_aspect_supported = 1,
    .shader_supported    = 1,
    .aspect_labels       = kLufia2AspectLabels,
    .num_aspect_labels   = 4,
    .aspect_setting_label = "Modo de Pantalla",
    .aspect_setting_help  = "4:3 clasico original, 16:9 Panoramico, 16:10 Steam Deck o Pantalla Completa estirada.",
    .prepare_frame       = Lufia2PrepareFrame,
    .compute_viewport    = Lufia2ComputeViewport,
    .begin_sim_frame     = Lufia2BeginFrame,
};

#ifndef __ANDROID__
#undef main
#endif
int main(int argc, char **argv)
{
    return snesrecomp_desktop_main(&kGameHost, argc, argv);
}
