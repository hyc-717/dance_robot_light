/**
 * @file    ws2812_water_drop.c
 * @brief   水滴落入水面并扩散多层涟漪动画
 * @note    共180帧，每39ms调用WS2812_WaterDropChange一次
 */

#include "ws2812.h"

#define WATER_DROP_FRAME_COUNT        180U
#define WATER_DROP_FALL_FRAMES        24U
#define WATER_DROP_IMPACT_FRAME       24U
#define WATER_DROP_RIPPLE_START       32U
#define WATER_DROP_RIPPLE_END         140U
#define WATER_DROP_SCREEN_OFFSET      18U

static uint8_t WS2812_WaterDropTriangle(uint8_t phase)
{
    if(phase < 128U)
    {
        return (uint8_t)(phase * 2U);
    }

    return (uint8_t)((255U - phase) * 2U);
}

static uint8_t WS2812_WaterDropScale(uint8_t value, uint8_t fade)
{
    return (uint8_t)((uint16_t)value * fade / 255U);
}

static uint16_t WS2812_WaterDropRingError(uint16_t distance_squared,
                                          uint8_t radius)
{
    uint16_t radius_squared = (uint16_t)radius * radius;

    return (distance_squared >= radius_squared) ?
           (uint16_t)(distance_squared - radius_squared) :
           (uint16_t)(radius_squared - distance_squared);
}

static uint16_t WS2812_WaterDropAbsDifference(int16_t left, int16_t right)
{
    return (left >= right) ?
           (uint16_t)(left - right) :
           (uint16_t)(right - left);
}

static void WS2812_WaterDropPixel(WS2812_Device device,
                                  uint8_t screen,
                                  uint8_t row,
                                  uint8_t col,
                                  uint8_t frame)
{
    uint8_t shimmer_phase;
    uint8_t shimmer;
    uint8_t drop_row;
    uint8_t splash_age;
    uint8_t arm;
    uint8_t bubble_row;
    uint8_t age;
    uint8_t radius;
    uint8_t inner_radius;
    uint8_t faint_radius;
    uint8_t fade;
    uint8_t r = 0U;
    uint8_t g;
    uint8_t b;
    int16_t dx;
    int16_t dy;
    uint16_t distance_squared;
    uint16_t band_width;
    uint16_t error;
    uint16_t inner_width;
    uint16_t faint_width;

    shimmer_phase = (uint8_t)(col * 17U + row * 23U
                              + frame * 3U + screen * 47U);
    shimmer = WS2812_WaterDropTriangle(shimmer_phase);
    g = (uint8_t)(7U + shimmer / 28U);
    b = (uint8_t)(24U + shimmer / 12U);

    dx = (int16_t)col * 2 - 15;
    dy = (int16_t)row * 2 - 15;
    distance_squared = (uint16_t)(dx * dx + dy * dy);

    if((frame >= WATER_DROP_RIPPLE_START) &&
       (frame < WATER_DROP_RIPPLE_END))
    {
        age = (uint8_t)(frame - WATER_DROP_RIPPLE_START);
        radius = (uint8_t)(1U + age / 8U);
        inner_radius = (radius > 3U) ? (uint8_t)(radius - 3U) : 0U;
        faint_radius = (radius > 6U) ? (uint8_t)(radius - 6U) : 0U;
        fade = (age > 40U) ?
               (uint8_t)((255U - (age - 40U) * 3U) > 72U ?
                         (255U - (age - 40U) * 3U) : 72U) :
               255U;
        band_width = (uint16_t)radius * 4U + 2U;
        error = WS2812_WaterDropRingError(distance_squared, radius);

        if(error <= band_width)
        {
            g = WS2812_WaterDropScale(135U, fade);
            b = WS2812_WaterDropScale(225U, fade);
        }
        else if(inner_radius > 0U)
        {
            inner_width = (uint16_t)inner_radius * 4U + 2U;
            error = WS2812_WaterDropRingError(distance_squared, inner_radius);
            if(error <= inner_width)
            {
                g = WS2812_WaterDropScale(52U, fade);
                b = WS2812_WaterDropScale(115U, fade);
            }
            else if(faint_radius > 0U)
            {
                faint_width = (uint16_t)faint_radius * 4U + 2U;
                error = WS2812_WaterDropRingError(distance_squared, faint_radius);
                if(error <= faint_width)
                {
                    g = WS2812_WaterDropScale(24U, fade);
                    b = WS2812_WaterDropScale(65U, fade);
                }
            }
        }
        else if(faint_radius > 0U)
        {
            faint_width = (uint16_t)faint_radius * 4U + 2U;
            error = WS2812_WaterDropRingError(distance_squared, faint_radius);
            if(error <= faint_width)
            {
                g = WS2812_WaterDropScale(24U, fade);
                b = WS2812_WaterDropScale(65U, fade);
            }
        }
    }

    if(frame < WATER_DROP_FALL_FRAMES)
    {
        drop_row = (uint8_t)(1U + frame / 4U);
        if((row == drop_row) && ((col == 7U) || (col == 8U)))
        {
            r = 185U;
            g = 250U;
            b = 255U;
        }
        else if((row == (uint8_t)(drop_row - 1U)) &&
                ((col == 7U) || (col == 8U)))
        {
            r = 0U;
            g = 92U;
            b = 175U;
        }
    }
    else if((frame >= WATER_DROP_IMPACT_FRAME) &&
            (frame < WATER_DROP_RIPPLE_START))
    {
        splash_age = (uint8_t)(frame - WATER_DROP_IMPACT_FRAME);
        arm = (uint8_t)(1U + splash_age / 2U);

        if(((row == 7U) || (row == 8U)) &&
           ((col == (uint8_t)(7U - arm)) ||
            (col == (uint8_t)(8U + arm))))
        {
            r = 90U;
            g = 230U;
            b = 255U;
        }
        else if((splash_age < 4U) && (row == 6U) &&
                ((col == 7U) || (col == 8U)))
        {
            r = 185U;
            g = 255U;
            b = 255U;
        }
        else if((splash_age < 4U) && (row == 9U) &&
                ((col == 7U) || (col == 8U)))
        {
            r = 30U;
            g = 150U;
            b = 230U;
        }
    }
    else if((frame >= WATER_DROP_RIPPLE_START) && (frame < 44U))
    {
        bubble_row = (uint8_t)(8U - (frame - WATER_DROP_RIPPLE_START) / 4U);
        if((row == bubble_row) && ((col == 7U) || (col == 8U)))
        {
            r = 115U;
            g = 230U;
            b = 255U;
        }
    }

    WS2812_ScreenSetPixel(device, row, col, r, g, b);
}

static void WS2812_WaterDropScreen(WS2812_Device device,
                                   uint8_t screen,
                                   uint8_t frame)
{
    uint8_t row;
    uint8_t col;

    WS2812_Fill(device, 0U, 0U, 0U);
    for(row = 0U; row < WS2812_SCREEN_HEIGHT; row++)
    {
        for(col = 0U; col < WS2812_SCREEN_WIDTH; col++)
        {
            WS2812_WaterDropPixel(device, screen, row, col, frame);
        }
    }
}

static void WS2812_WaterDropStrip(uint8_t frame)
{
    uint8_t index;
    uint8_t phase;
    uint8_t shimmer;
    uint8_t radius;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    int16_t distance;
    int16_t delta;

    for(index = 0U; index < WS2812_BODY_LED_COUNT; index++)
    {
        phase = (uint8_t)(index * 15U + frame * 3U);
        shimmer = WS2812_WaterDropTriangle(phase);
        color_r = 0U;
        color_g = (uint8_t)(9U + shimmer / 30U);
        color_b = (uint8_t)(25U + shimmer / 12U);

        if((frame >= WATER_DROP_RIPPLE_START) &&
           (frame < WATER_DROP_RIPPLE_END))
        {
            radius = (uint8_t)((frame - WATER_DROP_RIPPLE_START) / 2U);
            distance = (int16_t)index - 29;
            if(distance < 0)
            {
                distance = (int16_t)-distance;
            }
            delta = (int16_t)distance - radius;
            if(delta < 0)
            {
                delta = (int16_t)-delta;
            }

            if(delta <= 1)
            {
                color_r = 20U;
                color_g = 170U;
                color_b = 245U;
            }
            else if(delta <= 3)
            {
                color_r = 0U;
                color_g = 62U;
                color_b = 135U;
            }
        }

        WS2812_SetLED(WS2812_BODY, index, color_r, color_g, color_b);
    }
}

void WS2812_WaterDropChange(void)
{
    static uint8_t frame = 0U;

    uint8_t screen;
    uint8_t local_frame;
    uint16_t frame_with_offset;

    for(screen = 0U; screen < 2U; screen++)
    {
        frame_with_offset = (uint16_t)frame
                          + (uint16_t)screen * WATER_DROP_SCREEN_OFFSET;
        local_frame = (uint8_t)(frame_with_offset % WATER_DROP_FRAME_COUNT);
        WS2812_WaterDropScreen(
            (screen == 0U) ? WS2812_SCREEN_1 : WS2812_SCREEN_2,
            screen,
            local_frame);
    }

    WS2812_WaterDropStrip(frame);

    if(frame >= (WATER_DROP_FRAME_COUNT - 1U))
    {
        frame = 0U;
    }
    else
    {
        frame++;
    }
}
