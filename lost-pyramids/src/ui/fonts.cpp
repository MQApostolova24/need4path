#include "ui/fonts.h"

#include <string>

namespace
{
const int STYLE_COUNT = 6;
const int HEADING_LOAD_SIZE = 96;  // fonts are loaded big and scaled down smoothly
const int BODY_LOAD_SIZE = 48;

struct FontSlot
{
    const char* file;
    int loadSize;
    float spacing;  // extra letter spacing, as a part of the font size
};

const FontSlot SLOTS[STYLE_COUNT] = {
    {"CinzelDecorative/CinzelDecorative-Bold.ttf", HEADING_LOAD_SIZE, 0.04f},
    {"AlegreyaSans/AlegreyaSans-Regular.ttf", BODY_LOAD_SIZE, 0.0f},
    {"AlegreyaSans/AlegreyaSans-Bold.ttf", BODY_LOAD_SIZE, 0.01f},
    {"Rakkas/Rakkas-Regular.ttf", HEADING_LOAD_SIZE, 0.02f},
    {"Macondo/Macondo-Regular.ttf", HEADING_LOAD_SIZE, 0.02f},
    {"Koulen/Koulen-Regular.ttf", HEADING_LOAD_SIZE, 0.06f},
};

Font fonts[STYLE_COUNT];
bool loaded[STYLE_COUNT] = {};

// Looks for the assets folder next to the program first, then in the current folder.
std::string findFontsFolder()
{
    const std::string candidates[] = {
        std::string(GetApplicationDirectory()) + "assets/fonts/",
        "assets/fonts/",
        "../assets/fonts/",
        "../../assets/fonts/",
    };
    for (const std::string& folder : candidates)
    {
        if (DirectoryExists(folder.c_str()))
        {
            return folder;
        }
    }
    return "assets/fonts/";
}

const Font& fontFor(FontStyle style)
{
    return fonts[static_cast<int>(style)];
}

float spacingFor(FontStyle style, float size)
{
    const int index = static_cast<int>(style);
    if (!loaded[index])
    {
        return size / 10.0f;  // raylib's built-in font needs wider spacing
    }
    return SLOTS[index].spacing * size;
}
}  // namespace

void loadGameFonts()
{
    const std::string folder = findFontsFolder();

    for (int i = 0; i < STYLE_COUNT; ++i)
    {
        const std::string path = folder + SLOTS[i].file;
        loaded[i] = false;
        fonts[i] = GetFontDefault();

        if (!FileExists(path.c_str()))
        {
            continue;
        }

        Font font = LoadFontEx(path.c_str(), SLOTS[i].loadSize, nullptr, 0);
        if (IsFontValid(font) && font.texture.id != 0)
        {
            // Mipmaps + trilinear filtering keep the text smooth at every size.
            // (If the graphics driver cannot make mipmaps, bilinear filtering is used.)
            GenTextureMipmaps(&font.texture);
            SetTextureFilter(font.texture,
                             font.texture.mipmaps > 1 ? TEXTURE_FILTER_TRILINEAR : TEXTURE_FILTER_BILINEAR);
            fonts[i] = font;
            loaded[i] = true;
        }
    }
}

void unloadGameFonts()
{
    for (int i = 0; i < STYLE_COUNT; ++i)
    {
        if (loaded[i])
        {
            UnloadFont(fonts[i]);
            loaded[i] = false;
        }
        fonts[i] = GetFontDefault();
    }
}

FontStyle locationFont(Location location)
{
    switch (location)
    {
    case Location::Egypt:
        return FontStyle::Egypt;
    case Location::Mexico:
        return FontStyle::Mexico;
    case Location::Cambodia:
        return FontStyle::Cambodia;
    }
    return FontStyle::Title;
}

void drawString(const char* text, float x, float y, float size, Color color, FontStyle style)
{
    DrawTextEx(fontFor(style), text, {x, y}, size, spacingFor(style, size), color);
}

void drawStringCentered(const char* text, float centerX, float y, float size, Color color, FontStyle style)
{
    drawString(text, centerX - measureString(text, size, style) / 2.0f, y, size, color, style);
}

void drawStringRight(const char* text, float rightX, float y, float size, Color color, FontStyle style)
{
    drawString(text, rightX - measureString(text, size, style), y, size, color, style);
}

void drawStringShadowCentered(const char* text, float centerX, float y, float size, Color color, FontStyle style)
{
    const float offset = size / 22.0f + 1.0f;
    drawStringCentered(text, centerX + offset, y + offset, size, Fade(BLACK, 0.55f), style);
    drawStringCentered(text, centerX, y, size, color, style);
}

float measureString(const char* text, float size, FontStyle style)
{
    return MeasureTextEx(fontFor(style), text, size, spacingFor(style, size)).x;
}
