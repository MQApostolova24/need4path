#include "ui/widgets.h"

#include "ui/theme.h"

#include <cmath>

namespace
{
void drawPyramid(float centerX, float baseY, float halfWidth, float height)
{
    const Vector2 apex = {centerX, baseY - height};
    const Vector2 left = {centerX - halfWidth, baseY};
    const Vector2 middle = {centerX + halfWidth * 0.25f, baseY};
    const Vector2 right = {centerX + halfWidth, baseY};

    DrawTriangle(apex, left, middle, theme::SAND);         // sunlit side
    DrawTriangle(apex, middle, right, theme::SAND_DARK);   // shadow side
}
}  // namespace

void drawCenteredText(const char* text, int centerX, int y, int fontSize, Color color)
{
    const int width = MeasureText(text, fontSize);
    DrawText(text, centerX - width / 2, y, fontSize, color);
}

void drawDesertBackground(float time)
{
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, theme::NIGHT_SKY, theme::DUSK);

    // Twinkling stars
    for (int i = 0; i < 70; ++i)
    {
        const int x = (i * 197 + 53) % SCREEN_WIDTH;
        const int y = (i * 89 + 17) % (SCREEN_HEIGHT / 2);
        const float twinkle = 0.5f + 0.5f * std::sin(time * 1.7f + static_cast<float>(i));
        DrawCircle(x, y, 1.5f, Fade(theme::PAPYRUS, 0.25f + 0.6f * twinkle));
    }

    // Moon
    DrawCircle(SCREEN_WIDTH - 170, 130, 62.0f, Fade(theme::BRIGHT_GOLD, 0.12f));
    DrawCircle(SCREEN_WIDTH - 170, 130, 44.0f, theme::PAPYRUS);

    // Pyramids and dunes
    const float horizon = static_cast<float>(SCREEN_HEIGHT) * 0.72f;
    drawPyramid(250.0f, horizon, 230.0f, 210.0f);
    drawPyramid(520.0f, horizon, 140.0f, 125.0f);
    drawPyramid(static_cast<float>(SCREEN_WIDTH) - 290.0f, horizon, 200.0f, 180.0f);

    const int horizonY = static_cast<int>(horizon);
    DrawRectangle(0, horizonY, SCREEN_WIDTH, SCREEN_HEIGHT - horizonY, theme::SAND_DARK);
    DrawEllipse(SCREEN_WIDTH / 4, horizonY + 50, 560.0f, 70.0f, theme::SAND);
    DrawEllipse(SCREEN_WIDTH * 3 / 4, horizonY + 90, 600.0f, 90.0f, theme::SAND_LIGHT);
}

void drawTitle(const char* title, const char* subtitle, int y)
{
    const int centerX = SCREEN_WIDTH / 2;
    drawCenteredText(title, centerX + 3, y + 3, 52, Fade(BLACK, 0.6f));
    drawCenteredText(title, centerX, y, 52, theme::BRIGHT_GOLD);

    const float lineY = static_cast<float>(y + 62);
    const float half = 240.0f;
    const float cx = static_cast<float>(centerX);
    DrawLineEx({cx - half, lineY}, {cx + half, lineY}, 2.0f, theme::DARK_GOLD);
    DrawPoly({cx, lineY}, 4, 7.0f, 0.0f, theme::BRIGHT_GOLD);
    DrawPoly({cx - half, lineY}, 4, 5.0f, 0.0f, theme::BRIGHT_GOLD);
    DrawPoly({cx + half, lineY}, 4, 5.0f, 0.0f, theme::BRIGHT_GOLD);

    if (subtitle != nullptr)
    {
        drawCenteredText(subtitle, centerX, y + 74, 22, theme::PAPYRUS);
    }
}

void drawPanel(Rectangle bounds)
{
    DrawRectangleRounded(bounds, 0.05f, 8, theme::PANEL);
    DrawRectangleRoundedLinesEx(bounds, 0.05f, 8, 3.0f, theme::DARK_GOLD);
    const Rectangle inner = {bounds.x + 7.0f, bounds.y + 7.0f, bounds.width - 14.0f, bounds.height - 14.0f};
    DrawRectangleRoundedLinesEx(inner, 0.05f, 8, 1.0f, Fade(theme::DARK_GOLD, 0.5f));
}

bool drawButton(Rectangle bounds, const char* text, bool selected)
{
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    const bool highlighted = hovered || selected;

    const Rectangle shadow = {bounds.x + 3.0f, bounds.y + 4.0f, bounds.width, bounds.height};
    DrawRectangleRounded(shadow, 0.3f, 8, Fade(BLACK, 0.5f));
    DrawRectangleRounded(bounds, 0.3f, 8, highlighted ? theme::BRIGHT_GOLD : theme::SAND);
    DrawRectangleRoundedLinesEx(bounds, 0.3f, 8, highlighted ? 3.0f : 2.0f, theme::INK);

    const int fontSize = bounds.height >= 46.0f ? 24 : 20;
    const int textWidth = MeasureText(text, fontSize);
    const int textX = static_cast<int>(bounds.x + bounds.width / 2.0f) - textWidth / 2;
    const int textY = static_cast<int>(bounds.y + bounds.height / 2.0f) - fontSize / 2;
    DrawText(text, textX, textY, fontSize, theme::INK);

    if (selected)
    {
        // Small ankh markers next to the keyboard-selected button.
        const float iconY = bounds.y + bounds.height / 2.0f;
        drawAnkhIcon({bounds.x - 22.0f, iconY}, 26.0f, theme::BRIGHT_GOLD);
        drawAnkhIcon({bounds.x + bounds.width + 22.0f, iconY}, 26.0f, theme::BRIGHT_GOLD);
    }

    return hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

int handleMenuNavigation(int& selection, int count)
{
    if (count <= 0)
    {
        return -1;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_LEFT))
    {
        selection = (selection - 1 + count) % count;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT))
    {
        selection = (selection + 1) % count;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        return selection;
    }
    return -1;
}

void updateTextFields(TextField* fields[], const Rectangle boxes[], int count)
{
    // Mouse click focuses the field under the cursor.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const Vector2 mouse = GetMousePosition();
        bool clickedField = false;
        for (int i = 0; i < count; ++i)
        {
            clickedField = clickedField || CheckCollisionPointRec(mouse, boxes[i]);
        }
        if (clickedField)
        {
            for (int i = 0; i < count; ++i)
            {
                fields[i]->active = CheckCollisionPointRec(mouse, boxes[i]);
            }
        }
    }

    // Tab / Shift+Tab moves between fields.
    if (IsKeyPressed(KEY_TAB))
    {
        int activeIndex = -1;
        for (int i = 0; i < count; ++i)
        {
            if (fields[i]->active)
            {
                activeIndex = i;
            }
        }
        const bool backwards = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        const int next = backwards ? (activeIndex - 1 + count) % count : (activeIndex + 1) % count;
        for (int i = 0; i < count; ++i)
        {
            fields[i]->active = (i == next);
        }
    }

    // Typing goes into the active field.
    for (int i = 0; i < count; ++i)
    {
        TextField& field = *fields[i];
        if (!field.active)
        {
            continue;
        }

        int key = GetCharPressed();
        while (key > 0)
        {
            bool accepted = key >= 32 && key <= 126 && field.text.size() < field.maxLength;
            if (field.digitsOnly && (key < '0' || key > '9'))
            {
                accepted = false;
            }
            if (accepted)
            {
                field.text.push_back(static_cast<char>(key));
            }
            key = GetCharPressed();
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !field.text.empty())
        {
            field.text.pop_back();
        }
    }
}

void drawTextField(const TextField& field, Rectangle bounds)
{
    const int x = static_cast<int>(bounds.x);
    const int y = static_cast<int>(bounds.y);

    if (!field.label.empty())
    {
        DrawText(field.label.c_str(), x, y - 26, 20, theme::SAND_LIGHT);
    }

    DrawRectangleRec(bounds, theme::PAPYRUS);
    DrawRectangleLinesEx(bounds, field.active ? 3.0f : 2.0f, field.active ? theme::BRIGHT_GOLD : theme::STONE);

    const std::string shown = field.masked ? std::string(field.text.size(), '*') : field.text;
    const int fontSize = 24;
    const int textY = y + static_cast<int>(bounds.height / 2.0f) - fontSize / 2;
    DrawText(shown.c_str(), x + 12, textY, fontSize, theme::INK);

    // Blinking cursor
    if (field.active && static_cast<int>(GetTime() * 2.0) % 2 == 0)
    {
        const int cursorX = x + 14 + MeasureText(shown.c_str(), fontSize);
        DrawRectangle(cursorX, textY, 2, fontSize, theme::INK);
    }
}

void drawAnkhIcon(Vector2 center, float size, Color color)
{
    const Vector2 loopCenter = {center.x, center.y - size * 0.25f};
    DrawRing(loopCenter, size * 0.10f, size * 0.18f, 0.0f, 360.0f, 20, color);
    DrawRectangleV({center.x - size * 0.28f, center.y - size * 0.07f}, {size * 0.56f, size * 0.11f}, color);
    DrawRectangleV({center.x - size * 0.06f, center.y - size * 0.07f}, {size * 0.12f, size * 0.55f}, color);
}

void drawHeartIcon(Vector2 center, float size, Color color)
{
    const float radius = size * 0.26f;
    DrawCircleV({center.x - size * 0.22f, center.y - size * 0.1f}, radius, color);
    DrawCircleV({center.x + size * 0.22f, center.y - size * 0.1f}, radius, color);
    DrawTriangle({center.x - size * 0.47f, center.y - size * 0.02f},
                 {center.x, center.y + size * 0.45f},
                 {center.x + size * 0.47f, center.y - size * 0.02f}, color);
}

void drawEyeIcon(Vector2 center, float size, Color color)
{
    const int cx = static_cast<int>(center.x);
    const int cy = static_cast<int>(center.y);
    DrawEllipse(cx, cy, size * 0.40f, size * 0.20f, color);
    DrawEllipse(cx, cy, size * 0.31f, size * 0.12f, theme::STONE_DARK);
    DrawCircleV(center, size * 0.10f, color);
    const float thickness = std::fmax(1.0f, size * 0.06f);
    // The tear-line and brow of the Eye of Horus
    DrawLineEx({center.x - size * 0.05f, center.y + size * 0.18f},
               {center.x - size * 0.16f, center.y + size * 0.38f}, thickness, color);
    DrawLineEx({center.x - size * 0.36f, center.y - size * 0.30f},
               {center.x + size * 0.38f, center.y - size * 0.30f}, thickness, color);
}

void drawScarabIcon(Vector2 center, float size, Color color)
{
    const float thickness = std::fmax(1.0f, size * 0.07f);
    for (int i = -1; i <= 1; ++i)
    {
        const float legY = center.y + size * 0.16f * static_cast<float>(i);
        DrawLineEx({center.x - size * 0.42f, legY + size * 0.08f}, {center.x + size * 0.42f, legY - size * 0.08f},
                   thickness, color);
        DrawLineEx({center.x - size * 0.42f, legY - size * 0.08f}, {center.x + size * 0.42f, legY + size * 0.08f},
                   thickness, color);
    }
    DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y + size * 0.05f), size * 0.25f, size * 0.32f,
                color);
    DrawCircleV({center.x, center.y - size * 0.32f}, size * 0.13f, color);
    DrawLineEx({center.x, center.y - size * 0.18f}, {center.x, center.y + size * 0.35f}, thickness,
               Fade(BLACK, 0.5f));
}
