/**
 * @file    ws2812_water_wave.c
 * @brief   16x16灯屏与LED灯带水波流动动画
 * @note    每39ms调用WS2812_WaterWaveChange一次
 */

#include "ws2812.h"

static uint8_t WS2812_WaterTriangle(uint8_t phase)
{
    if(phase < 128U)
    {
        return (uint8_t)(phase * 2U);
    }

    return (uint8_t)((255U - phase) * 2U);
}

static void WS2812_WaterColor(uint8_t level,
                              uint8_t screen,
                              uint8_t *r,
                              uint8_t *g,
                              uint8_t *b)
{
    if(level >= 220U)
    {
        *r = (screen == 0U) ? 140U : 100U;
        *g = (screen == 0U) ? 240U : 255U;
        *b = (screen == 0U) ? 255U : 230U;
    }
    else if(level >= 160U)
    {
        *r = 0U;
        *g = (screen == 0U) ? 150U : 190U;
        *b = (screen == 0U) ? 225U : 180U;
    }
    else if(level >= 90U)
    {
        *r = 0U;
        *g = (screen == 0U) ? 78U : 110U;
        *b = (screen == 0U) ? 150U : 112U;
    }
    else if(screen == 0U)
    {
        *r = 0U;
        *g = (uint8_t)(16U + level / 5U);
        *b = (uint8_t)(50U + level / 3U);
    }
    else
    {
        *r = 0U;
        *g = (uint8_t)(22U + level / 6U);
        *b = (uint8_t)(42U + level / 4U);
    }
}

void WS2812_WaterWaveChange(void)
{
    static uint8_t frame = 0U;

    uint8_t screen;
    uint8_t row;
    uint8_t col;
    uint8_t index;
    uint8_t phase1;
    uint8_t phase2;
    uint8_t pulse1;
    uint8_t pulse2;
    uint8_t level;
    uint8_t r;
    uint8_t g;
    uint8_t b;

    WS2812_ClearAll();

    for(screen = 0U; screen < 2U; screen++)
    {
        for(row = 0U; row < WS2812_SCREEN_HEIGHT; row++)
        {
            for(col = 0U; col < WS2812_SCREEN_WIDTH; col++)
            {
                phase1 = (uint8_t)(col * 21U + row * 11U
                                   + frame * 7U + screen * 36U);
                phase2 = (uint8_t)((WS2812_SCREEN_WIDTH - 1U - col) * 15U
                                   + row * 19U + frame * 4U + screen * 64U);

                pulse1 = WS2812_WaterTriangle(phase1);
                pulse2 = WS2812_WaterTriangle(phase2);
                level = (uint8_t)(((uint16_t)pulse1 * 2U + pulse2) / 3U);

                WS2812_WaterColor(level, screen, &r, &g, &b);
                WS2812_ScreenSetPixel(
                    (screen == 0U) ? WS2812_SCREEN_1 : WS2812_SCREEN_2,
                    row, col, r, g, b);
            }
        }
    }

    for(index = 0U; index < WS2812_BODY_LED_COUNT; index++)
    {
        phase1 = (uint8_t)(index * 13U + frame * 9U);
        level = WS2812_WaterTriangle(phase1);
        WS2812_WaterColor(level, 0U, &r, &g, &b);
        WS2812_SetLED(WS2812_BODY, index, r, g, b);
    }

    frame++;
}
