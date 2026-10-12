/**
 * @file    ws2812_lotus_bloom.c
 * @brief   多层莲花花瓣缓开、停留与合拢动画
 * @note    共240帧，每39ms调用WS2812_LotusBloomChange一次
 */

#include "ws2812.h"

#define LOTUS_BLOOM_FRAME_COUNT       240U
#define LOTUS_BLOOM_RAMP_FRAMES       96U
#define LOTUS_BLOOM_HOLD_END          144U
#define LOTUS_BLOOM_MAX_OPEN          16U
#define LOTUS_BLOOM_SCREEN_OFFSET     30U

static uint8_t WS2812_LotusBloomOpen(uint8_t frame)
{
    if(frame < LOTUS_BLOOM_RAMP_FRAMES)
    {
        return (uint8_t)(frame / 6U);
    }
    else if(frame < LOTUS_BLOOM_HOLD_END)
    {
        return LOTUS_BLOOM_MAX_OPEN;
    }

    return (uint8_t)((LOTUS_BLOOM_FRAME_COUNT - frame) / 6U);
}

static uint8_t WS2812_LotusTriangle(uint8_t phase)
{
    if(phase < 128U)
    {
        return (uint8_t)(phase * 2U);
    }

    return (uint8_t)((255U - phase) * 2U);
}

static void WS2812_LotusBloomSpan(WS2812_Device device,
                                  uint8_t row,
                                  int16_t left,
                                  int16_t right,
                                  uint8_t r,
                                  uint8_t g,
                                  uint8_t b)
{
    uint8_t col;

    if(left < 0)
    {
        left = 0;
    }
    if(right >= (int16_t)WS2812_SCREEN_WIDTH)
    {
        right = (int16_t)(WS2812_SCREEN_WIDTH - 1U);
    }

    for(col = (uint8_t)left; (int16_t)col <= right; col++)
    {
        WS2812_ScreenSetPixel(device, row, col, r, g, b);
    }
}

static void WS2812_LotusBloomScreen(WS2812_Device device,
                                    uint8_t local_frame)
{
    static const uint8_t outer_left[8] = {7U, 7U, 6U, 6U, 5U, 4U, 4U, 5U};
    static const uint8_t outer_right[8] = {8U, 8U, 9U, 9U, 10U, 11U, 11U, 10U};
    static const uint8_t outer_spread[8] = {0U, 1U, 2U, 3U, 4U, 4U, 3U, 1U};
    static const uint8_t outer_width[8] = {0U, 1U, 1U, 1U, 2U, 1U, 1U, 0U};
    static const uint8_t middle_left[6] = {7U, 7U, 6U, 6U, 7U, 7U};
    static const uint8_t middle_right[6] = {8U, 8U, 9U, 9U, 8U, 8U};
    static const uint8_t middle_spread[6] = {0U, 1U, 2U, 2U, 1U, 0U};
    static const uint8_t middle_width[6] = {0U, 0U, 1U, 1U, 0U, 0U};
    static const uint8_t inner_left[6] = {7U, 6U, 6U, 6U, 7U, 7U};
    static const uint8_t inner_right[6] = {8U, 9U, 9U, 9U, 8U, 8U};
    static const uint8_t inner_spread[6] = {0U, 0U, 1U, 1U, 0U, 0U};
    static const uint8_t inner_width[6] = {0U, 0U, 1U, 1U, 0U, 0U};

    uint8_t open;
    uint8_t middle_open;
    uint8_t inner_open;
    uint8_t leaf_green;
    uint8_t row;
    uint8_t i;
    uint8_t width;
    uint8_t shift;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    int16_t left;
    int16_t right;

    open = WS2812_LotusBloomOpen(local_frame);
    middle_open = (open > 3U) ? (uint8_t)(open - 3U) : 0U;
    inner_open = (open > 5U) ? (uint8_t)(open - 5U) : 0U;
    leaf_green = (uint8_t)(70U + open * 3U);

    WS2812_Fill(device, 0U, 0U, 0U);

    WS2812_LotusBloomSpan(device, 11U, 1, 6, 8U, (uint8_t)(leaf_green + 25U), 32U);
    WS2812_LotusBloomSpan(device, 11U, 9, 14, 8U, (uint8_t)(leaf_green + 25U), 32U);
    WS2812_LotusBloomSpan(device, 12U, 3, 12, 5U, (uint8_t)(leaf_green + 12U), 24U);
    WS2812_LotusBloomSpan(device, 13U, 5, 10, 4U, leaf_green, 20U);
    WS2812_LotusBloomSpan(device, 14U, 6, 9, 3U, (uint8_t)(leaf_green - 5U), 18U);
    WS2812_LotusBloomSpan(device, 15U, 7, 8, 3U, (uint8_t)(leaf_green - 8U), 16U);

    for(row = 3U; row <= 10U; row++)
    {
        i = (uint8_t)(row - 3U);
        shift = (uint8_t)(open * outer_spread[i] / LOTUS_BLOOM_MAX_OPEN);
        width = (uint8_t)(outer_width[i] + open / 8U);
        r = (uint8_t)(120U + open * 4U);
        g = (uint8_t)(18U + open * 2U);
        b = (uint8_t)(52U + open * 4U);

        left = (int16_t)outer_left[i] - shift - width;
        right = (int16_t)outer_left[i] - shift + width;
        WS2812_LotusBloomSpan(device, row, left, right, r, g, b);

        left = (int16_t)outer_right[i] + shift - width;
        right = (int16_t)outer_right[i] + shift + width;
        WS2812_LotusBloomSpan(device, row, left, right, r, g, b);
    }

    for(row = 5U; row <= 10U; row++)
    {
        i = (uint8_t)(row - 5U);
        shift = (uint8_t)(middle_open * middle_spread[i] / 13U);
        width = (uint8_t)(middle_width[i] + middle_open / 7U);
        r = (uint8_t)(190U + open * 3U);
        g = (uint8_t)(30U + open * 5U);
        b = (uint8_t)(78U + open * 5U);

        left = (int16_t)middle_left[i] - shift - width;
        right = (int16_t)middle_left[i] - shift + width;
        WS2812_LotusBloomSpan(device, row, left, right, r, g, b);

        left = (int16_t)middle_right[i] + shift - width;
        right = (int16_t)middle_right[i] + shift + width;
        WS2812_LotusBloomSpan(device, row, left, right, r, g, b);
    }

    for(row = 4U; row <= 9U; row++)
    {
        i = (uint8_t)(row - 4U);
        shift = (uint8_t)(inner_open * inner_spread[i] / 11U);
        width = (uint8_t)(inner_width[i] + inner_open / 6U);
        r = 235U;
        g = (uint8_t)(92U + open * 4U);
        b = (uint8_t)(122U + open * 3U);

        left = (int16_t)inner_left[i] - shift - width;
        right = (int16_t)inner_left[i] - shift + width;
        WS2812_LotusBloomSpan(device, row, left, right, r, g, b);

        left = (int16_t)inner_right[i] + shift - width;
        right = (int16_t)inner_right[i] + shift + width;
        WS2812_LotusBloomSpan(device, row, left, right, r, g, b);
    }

    if(open >= 8U)
    {
        if((local_frame % 12U) < 6U)
        {
            r = 255U;
            g = 198U;
            b = 48U;
        }
        else
        {
            r = 245U;
            g = 95U;
            b = 22U;
        }

        WS2812_ScreenSetPixel(device, 7U, 7U, r, g, b);
        WS2812_ScreenSetPixel(device, 7U, 8U, r, g, b);
        WS2812_ScreenSetPixel(device, 8U, 7U, r, g, b);
        WS2812_ScreenSetPixel(device, 8U, 8U, r, g, b);
    }
}

static void WS2812_LotusStripColor(uint8_t level,
                                   uint8_t open,
                                   uint8_t *r,
                                   uint8_t *g,
                                   uint8_t *b)
{
    if((open >= 8U) && (level >= 220U))
    {
        *r = 255U;
        *g = 190U;
        *b = 45U;
    }
    else if(level >= 180U)
    {
        *r = 230U;
        *g = 65U;
        *b = 125U;
    }
    else if(level >= 90U)
    {
        *r = 135U;
        *g = 18U;
        *b = 62U;
    }
    else
    {
        *r = 8U;
        *g = (uint8_t)(8U + open / 3U);
        *b = (uint8_t)(16U + open);
    }
}

void WS2812_LotusBloomChange(void)
{
    static uint8_t frame = 0U;

    uint8_t screen;
    uint8_t index;
    uint8_t local_frame;
    uint8_t open;
    uint8_t phase;
    uint8_t level;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t frame_with_offset;

    for(screen = 0U; screen < 2U; screen++)
    {
        frame_with_offset = (uint16_t)frame
                          + (uint16_t)screen * LOTUS_BLOOM_SCREEN_OFFSET;
        local_frame = (uint8_t)(frame_with_offset % LOTUS_BLOOM_FRAME_COUNT);
        WS2812_LotusBloomScreen(
            (screen == 0U) ? WS2812_SCREEN_1 : WS2812_SCREEN_2,
            local_frame);
    }

    open = WS2812_LotusBloomOpen(frame);
    for(index = 0U; index < WS2812_BODY_LED_COUNT; index++)
    {
        phase = (uint8_t)(index * 17U + frame * 9U);
        level = WS2812_LotusTriangle(phase);
        WS2812_LotusStripColor(level, open, &r, &g, &b);
        WS2812_SetLED(WS2812_BODY, index, r, g, b);
    }

    if(frame >= (LOTUS_BLOOM_FRAME_COUNT - 1U))
    {
        frame = 0U;
    }
    else
    {
        frame++;
    }
}
