#include "ui/widgets.h"

#include "ui/draw_utils.h"
#include "ui/theme.h"

#include <cmath>

void drawPanel(Rectangle bounds, Color accent)
{
    const Rectangle shadow = {bounds.x + 6.0f, bounds.y + 8.0f, bounds.width, bounds.height};
    DrawRectangleRounded(shadow, 0.04f, 8, Fade(BLACK, 0.45f));
    DrawRectangleRounded(bounds, 0.04f, 8, theme::PANEL);
    DrawRectangleRoundedLinesEx(bounds, 0.04f, 8, 3.0f, accent);
    const Rectangle inner = {bounds.x + 8.0f, bounds.y + 8.0f, bounds.width - 16.0f, bounds.height - 16.0f};
    DrawRectangleRoundedLinesEx(inner, 0.04f, 8, 1.0f, Fade(accent, 0.45f));

    // Corner diamonds
    const float inset = 8.0f;
    const Vector2 corners[4] = {{bounds.x + inset, bounds.y + inset},
                                {bounds.x + bounds.width - inset, bounds.y + inset},
                                {bounds.x + inset, bounds.y + bounds.height - inset},
                                {bounds.x + bounds.width - inset, bounds.y + bounds.height - inset}};
    for (const Vector2& corner : corners)
    {
        drawDiamond(corner, 5.0f, accent);
    }
}

void drawOrnamentLine(float centerX, float y, float halfWidth, Color color)
{
    DrawLineEx({centerX - halfWidth, y}, {centerX - 14.0f, y}, 2.0f, color);
    DrawLineEx({centerX + 14.0f, y}, {centerX + halfWidth, y}, 2.0f, color);
    drawDiamond({centerX, y}, 8.0f, color);
    drawDiamond({centerX, y}, 4.0f, theme::PANEL);
    drawDiamond({centerX - halfWidth, y}, 5.0f, color);
    drawDiamond({centerX + halfWidth, y}, 5.0f, color);
}

void drawGameTitle(float y, const char* subtitle)
{
    const float centerX = static_cast<float>(SCREEN_WIDTH) / 2.0f;
    drawStringShadowCentered("LOST PYRAMIDS", centerX, y, 72.0f, theme::GOLD_BRIGHT, FontStyle::Title);
    drawOrnamentLine(centerX, y + 86.0f, 260.0f, theme::GOLD_BRIGHT);
    if (subtitle != nullptr)
    {
        drawStringShadowCentered(subtitle, centerX, y + 98.0f, 24.0f, theme::PARCHMENT, FontStyle::Body);
    }
}

void drawHeading(const char* text, float y, float size, Color color, FontStyle style)
{
    const float centerX = static_cast<float>(SCREEN_WIDTH) / 2.0f;
    drawStringShadowCentered(text, centerX, y, size, color, style);
    const float halfWidth = std::fmax(140.0f, measureString(text, size, style) / 2.0f + 30.0f);
    drawOrnamentLine(centerX, y + size * 1.12f, halfWidth, color);
}

bool drawButton(Rectangle bounds, const char* text, bool selected)
{
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    const bool highlighted = hovered || selected;

    const Rectangle shadow = {bounds.x + 3.0f, bounds.y + 5.0f, bounds.width, bounds.height};
    DrawRectangleRounded(shadow, 0.25f, 8, Fade(BLACK, 0.5f));

    // Carved stone look: a lighter top half and a darker bottom half.
    const Color top = highlighted ? theme::GOLD_BRIGHT : theme::BRONZE_LIGHT;
    const Color bottom = highlighted ? theme::GOLD_DEEP : theme::BRONZE;
    DrawRectangleRounded(bounds, 0.25f, 8, bottom);
    DrawRectangleRounded({bounds.x, bounds.y, bounds.width, bounds.height * 0.55f}, 0.25f, 8, top);
    DrawRectangleRoundedLinesEx(bounds, 0.25f, 8, 2.0f, highlighted ? theme::PARCHMENT : theme::GOLD_DEEP);

    const float fontSize = bounds.height >= 46.0f ? 26.0f : 22.0f;
    const Color textColor = highlighted ? theme::INK : theme::PARCHMENT;
    drawStringCentered(text, bounds.x + bounds.width / 2.0f, bounds.y + (bounds.height - fontSize) / 2.0f - 1.0f,
                       fontSize, textColor, FontStyle::Bold);

    if (selected)
    {
        // Small diamonds next to the keyboard-selected button.
        const float iconY = bounds.y + bounds.height / 2.0f;
        drawDiamond({bounds.x - 18.0f, iconY}, 8.0f, theme::GOLD_BRIGHT);
        drawDiamond({bounds.x + bounds.width + 18.0f, iconY}, 8.0f, theme::GOLD_BRIGHT);
    }

    return hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

bool drawTab(Rectangle bounds, const char* text, bool active, Color accent)
{
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    DrawRectangleRounded(bounds, 0.3f, 6, active ? accent : Fade(theme::PANEL_LIGHT, hovered ? 1.0f : 0.85f));
    DrawRectangleRoundedLinesEx(bounds, 0.3f, 6, 2.0f, active ? theme::PARCHMENT : Fade(accent, 0.7f));
    drawStringCentered(text, bounds.x + bounds.width / 2.0f, bounds.y + (bounds.height - 22.0f) / 2.0f, 22.0f,
                       active ? theme::INK : theme::PARCHMENT, FontStyle::Bold);
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
    if (!field.label.empty())
    {
        drawString(field.label.c_str(), bounds.x, bounds.y - 28.0f, 22.0f, theme::PARCHMENT_DARK, FontStyle::Bold);
    }

    DrawRectangleRounded(bounds, 0.15f, 6, theme::PARCHMENT);
    DrawRectangleRounded({bounds.x, bounds.y, bounds.width, 6.0f}, 0.5f, 4, Fade(theme::INK, 0.12f));
    DrawRectangleRoundedLinesEx(bounds, 0.15f, 6, field.active ? 3.0f : 2.0f,
                                field.active ? theme::GOLD_BRIGHT : theme::BRONZE);

    const std::string shown = field.masked ? std::string(field.text.size(), '*') : field.text;
    const float fontSize = 26.0f;
    const float textY = bounds.y + (bounds.height - fontSize) / 2.0f;
    drawString(shown.c_str(), bounds.x + 14.0f, textY, fontSize, theme::INK, FontStyle::Body);

    // Blinking cursor
    if (field.active && static_cast<int>(GetTime() * 2.0) % 2 == 0)
    {
        const float cursorX = bounds.x + 16.0f + measureString(shown.c_str(), fontSize, FontStyle::Body);
        DrawRectangleRec({cursorX, textY + 2.0f, 2.0f, fontSize - 2.0f}, theme::INK);
    }
}

void drawStar(Vector2 center, float radius, bool filled, Color color)
{
    Vector2 points[11];
    points[0] = center;
    for (int i = 0; i < 10; ++i)
    {
        // Points go counter-clockwise on screen (raylib's triangle fan needs that order).
        const float angle = -PI / 2.0f - static_cast<float>(i) * PI / 5.0f;
        const float length = (i % 2 == 0) ? radius : radius * 0.45f;
        points[i + 1] = {center.x + std::cos(angle) * length, center.y + std::sin(angle) * length};
    }

    if (filled)
    {
        Vector2 fan[12];
        for (int i = 0; i < 11; ++i)
        {
            fan[i] = points[i];
        }
        fan[11] = points[1];
        DrawTriangleFan(fan, 12, color);
    }
    for (int i = 1; i <= 10; ++i)
    {
        const Vector2 next = points[i % 10 + 1];
        DrawLineEx(points[i], next, std::fmax(1.5f, radius * 0.1f), filled ? Fade(BLACK, 0.35f) : Fade(color, 0.8f));
    }
}

void drawStarRow(Vector2 firstCenter, float radius, int filled, int total, Color color)
{
    for (int i = 0; i < total; ++i)
    {
        const Vector2 center = {firstCenter.x + static_cast<float>(i) * radius * 2.4f, firstCenter.y};
        drawStar(center, radius, i < filled, i < filled ? color : Fade(theme::TEXT_MUTED, 0.7f));
    }
}

void drawHeart(Vector2 center, float size, Color color)
{
    const float radius = size * 0.26f;
    DrawCircleV({center.x - size * 0.22f, center.y - size * 0.1f}, radius, color);
    DrawCircleV({center.x + size * 0.22f, center.y - size * 0.1f}, radius, color);
    drawTriangleAnyOrder({center.x - size * 0.47f, center.y - size * 0.02f}, {center.x, center.y + size * 0.45f},
                         {center.x + size * 0.47f, center.y - size * 0.02f}, color);
    DrawCircleV({center.x - size * 0.26f, center.y - size * 0.16f}, size * 0.07f, Fade(WHITE, 0.45f));
}

void drawBadge(const char* text, Vector2 center, float size, Color background, Color textColor)
{
    const float width = measureString(text, size, FontStyle::Bold) + size * 1.2f;
    const Rectangle box = {center.x - width / 2.0f, center.y - size * 0.75f, width, size * 1.5f};
    DrawRectangleRounded(box, 0.5f, 6, background);
    drawStringCentered(text, center.x, center.y - size * 0.55f, size, textColor, FontStyle::Bold);
}

void drawCoinIcon(Vector2 center, float radius)
{
    DrawCircleV({center.x + 1.5f, center.y + 2.0f}, radius, Fade(BLACK, 0.4f));
    DrawCircleV(center, radius, theme::GOLD_BRIGHT);
    DrawRing(center, radius * 0.62f, radius * 0.8f, 0.0f, 360.0f, 24, theme::GOLD_DEEP);
    DrawRectangleRec({center.x - radius * 0.12f, center.y - radius * 0.4f, radius * 0.24f, radius * 0.8f}, theme::GOLD_DEEP);
}

void drawCoinBadge(int coins, float rightX, float topY)
{
    const char* text = TextFormat("%d", coins);
    const float width = measureString(text, 26.0f, FontStyle::Bold) + 64.0f;
    const Rectangle box = {rightX - width, topY, width, 44.0f};
    DrawRectangleRounded(box, 0.5f, 8, Fade(theme::PANEL, 0.92f));
    DrawRectangleRoundedLinesEx(box, 0.5f, 8, 2.0f, theme::GOLD_DEEP);
    drawCoinIcon({box.x + 24.0f, box.y + 22.0f}, 13.0f);
    drawString(text, box.x + 44.0f, box.y + 8.0f, 26.0f, theme::GOLD_BRIGHT, FontStyle::Bold);
}
