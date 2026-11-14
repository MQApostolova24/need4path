// The Trading Post: sell relics for coins and spend coins on explorer skins.

#include "ui/screens.h"

#include "core/skins.h"
#include "ui/draw_utils.h"
#include "ui/location_theme.h"
#include "ui/sprites.h"
#include "ui/theme.h"

namespace
{
const int RELIC_TAB = 0;

void showTradingMessage(App& app, const std::string& message, bool isError)
{
    app.tradingMessage = message;
    app.tradingMessageIsError = isError;
}

// ---------------------------------------------------------------- relics

void sellSelectedRelic(App& app, Location location, Difficulty difficulty)
{
    if (!sellRelic(app.profile, location, difficulty))
    {
        showTradingMessage(app, "You do not have this relic.", true);
        return;
    }
    saveCurrentProfile(app);
    showTradingMessage(app, TextFormat("Sold the %s for %d coins.", getRelicName(location, difficulty),
                                       relicSellPrice(difficulty)),
                       false);
}

void drawRelicSlot(App& app, Rectangle slot, Location location, Difficulty difficulty, bool selected, float time)
{
    const PlayerProfile& profile = app.profile;
    const LocationPalette& p = getPalette(location);
    const int count = profile.relicCount(location, difficulty);
    const bool known = profile.hasDiscovered(location, difficulty);

    DrawRectangleRounded(slot, 0.12f, 8, count > 0 ? Fade(p.accent, 0.18f) : Fade(theme::PANEL, 0.9f));
    DrawRectangleRoundedLinesEx(slot, 0.12f, 8, selected ? 3.0f : 1.0f, selected ? theme::GOLD_BRIGHT : theme::LOCKED);

    const Vector2 icon = {slot.x + 48.0f, slot.y + 52.0f};
    const float textX = slot.x + 92.0f;

    if (!known)
    {
        DrawCircleV(icon, 26.0f, Fade(theme::LOCKED, 0.8f));
        drawStringCentered("?", icon.x, icon.y - 18.0f, 36.0f, theme::PARCHMENT, FontStyle::Title);
        drawString("Unknown relic", textX, slot.y + 26.0f, 22.0f, theme::TEXT_MUTED, FontStyle::Bold);
        drawString(TextFormat("Find it on %s", difficultyName(difficulty)), textX, slot.y + 54.0f, 19.0f,
                   theme::TEXT_MUTED, FontStyle::Body);
        return;
    }

    drawRelicSprite(location, icon, 56.0f, time);
    if (count == 0)
    {
        DrawCircleV(icon, 34.0f, Fade(theme::PANEL, 0.6f));
    }
    drawString(getRelicName(location, difficulty), textX, slot.y + 20.0f, 21.0f,
               count > 0 ? theme::GOLD_BRIGHT : theme::TEXT_MUTED, FontStyle::Bold);
    drawString(count > 0 ? TextFormat("You have %d", count) : "None left - find another", textX, slot.y + 46.0f, 19.0f,
               count > 0 ? theme::TEXT_LIGHT : theme::TEXT_MUTED, FontStyle::Body);

    if (count > 0)
    {
        const Rectangle sell = {slot.x + slot.width - 168.0f, slot.y + slot.height - 52.0f, 154.0f, 40.0f};
        if (drawButton(sell, TextFormat("Sell  +%d", relicSellPrice(difficulty)), false))
        {
            sellSelectedRelic(app, location, difficulty);
        }
    }
}

void runRelicTab(App& app, float time)
{
    const float firstColumn = 290.0f;
    const float columnWidth = 310.0f;

    for (int d = 0; d < DIFFICULTY_COUNT; ++d)
    {
        drawStringCentered(TextFormat("%s  (%d coins)", difficultyName(static_cast<Difficulty>(d)),
                                      relicSellPrice(static_cast<Difficulty>(d))),
                           firstColumn + columnWidth * static_cast<float>(d) + 145.0f, 186.0f, 22.0f, theme::GOLD_BRIGHT,
                           FontStyle::Bold);
    }

    for (int l = 0; l < LOCATION_COUNT; ++l)
    {
        const Location location = static_cast<Location>(l);
        const LocationPalette& p = getPalette(location);
        const float rowY = 222.0f + 150.0f * static_cast<float>(l);

        drawLocationEmblem(location, {100.0f, rowY + 56.0f}, 48.0f, p.accent);
        drawString(getLocationInfo(location).name, 136.0f, rowY + 34.0f, 34.0f, p.accent, locationFont(location));

        for (int d = 0; d < DIFFICULTY_COUNT; ++d)
        {
            const int index = l * DIFFICULTY_COUNT + d;
            const Rectangle slot = {firstColumn + columnWidth * static_cast<float>(d), rowY, 290.0f, 136.0f};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), slot))
            {
                app.tradingSelection = index;
            }
            drawRelicSlot(app, slot, location, static_cast<Difficulty>(d), app.tradingSelection == index, time);
        }
    }

    // Keyboard: arrows move around the grid, Enter sells
    if (IsKeyPressed(KEY_RIGHT))
    {
        app.tradingSelection = (app.tradingSelection + 1) % RELIC_TYPES;
    }
    if (IsKeyPressed(KEY_LEFT))
    {
        app.tradingSelection = (app.tradingSelection + RELIC_TYPES - 1) % RELIC_TYPES;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        app.tradingSelection = (app.tradingSelection + DIFFICULTY_COUNT) % RELIC_TYPES;
    }
    if (IsKeyPressed(KEY_UP))
    {
        app.tradingSelection = (app.tradingSelection + RELIC_TYPES - DIFFICULTY_COUNT) % RELIC_TYPES;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        sellSelectedRelic(app, static_cast<Location>(app.tradingSelection / DIFFICULTY_COUNT),
                          static_cast<Difficulty>(app.tradingSelection % DIFFICULTY_COUNT));
    }
}

// ---------------------------------------------------------------- skins

void useSkin(App& app, int skinId)
{
    const SkinInfo& skin = getSkinInfo(skinId);
    if (app.profile.ownsSkin(skinId))
    {
        equipSkin(app.profile, skinId);
        saveCurrentProfile(app);
        showTradingMessage(app, TextFormat("You are now the %s.", skin.name), false);
        return;
    }

    switch (buySkin(app.profile, skinId))
    {
    case PurchaseResult::Bought:
        saveCurrentProfile(app);
        showTradingMessage(app, TextFormat("Bought and equipped: %s!", skin.name), false);
        break;
    case PurchaseResult::NotEnoughCoins:
        showTradingMessage(app, TextFormat("You need %d more coins.", skin.price - app.profile.coins), true);
        break;
    case PurchaseResult::AlreadyOwned:
    case PurchaseResult::InvalidSkin:
        break;
    }
}

void drawSkinCard(App& app, Rectangle card, int skinId, bool selected, float time)
{
    const SkinInfo& skin = getSkinInfo(skinId);
    const bool owned = app.profile.ownsSkin(skinId);
    const bool equipped = app.profile.equippedSkin == skinId;

    DrawRectangleRounded(card, 0.1f, 8, equipped ? Fade(theme::GOLD_DEEP, 0.3f) : Fade(theme::PANEL, 0.9f));
    DrawRectangleRoundedLinesEx(card, 0.1f, 8, selected ? 3.0f : 1.5f, selected ? theme::GOLD_BRIGHT : theme::BRONZE);

    const Vector2 preview = {card.x + card.width / 2.0f, card.y + 74.0f};
    DrawCircleGradient(preview, 62.0f, Fade(theme::GOLD_BRIGHT, 0.25f), Fade(theme::GOLD_BRIGHT, 0.0f));
    drawExplorerSprite(preview, 96.0f, time, skinId, Direction::Right, false);

    drawStringCentered(skin.name, preview.x, card.y + 136.0f, 24.0f, theme::PARCHMENT, FontStyle::Bold);
    drawStringCentered(skin.description, preview.x, card.y + 164.0f, 18.0f, theme::TEXT_MUTED, FontStyle::Body);

    const Rectangle button = {card.x + 50.0f, card.y + card.height - 52.0f, card.width - 100.0f, 42.0f};
    if (equipped)
    {
        drawBadge("EQUIPPED", {button.x + button.width / 2.0f, button.y + 21.0f}, 20.0f, theme::SUCCESS, theme::INK);
        return;
    }
    const char* label = owned ? "Equip" : TextFormat("Buy  %d", skin.price);
    if (drawButton(button, label, false))
    {
        useSkin(app, skinId);
    }
    if (!owned)
    {
        drawCoinIcon({button.x + 24.0f, button.y + 21.0f}, 10.0f);
    }
}

void runSkinTab(App& app, float time)
{
    for (int i = 0; i < SKIN_COUNT; ++i)
    {
        const Rectangle card = {SCREEN_CENTER_X - 490.0f + 335.0f * static_cast<float>(i % 3),
                                182.0f + 258.0f * static_cast<float>(i / 3), 310.0f, 242.0f};
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), card))
        {
            app.tradingSelection = i;
        }
        drawSkinCard(app, card, i, app.tradingSelection == i, time);
    }

    if (IsKeyPressed(KEY_RIGHT))
    {
        app.tradingSelection = (app.tradingSelection + 1) % SKIN_COUNT;
    }
    if (IsKeyPressed(KEY_LEFT))
    {
        app.tradingSelection = (app.tradingSelection + SKIN_COUNT - 1) % SKIN_COUNT;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP))
    {
        app.tradingSelection = (app.tradingSelection + 3) % SKIN_COUNT;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        useSkin(app, app.tradingSelection);
    }
}
}  // namespace

void runTradingPostScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawMenuBackdrop(time);
    drawHeading("TRADING POST", 14.0f, 44.0f, theme::GOLD_BRIGHT, FontStyle::Title);
    drawCoinBadge(app.profile.coins, static_cast<float>(SCREEN_WIDTH) - 24.0f, 22.0f);

    // Tabs (Tab key switches)
    const char* tabs[] = {"Sell Relics", "Explorer Skins"};
    for (int i = 0; i < 2; ++i)
    {
        if (drawTab({SCREEN_CENTER_X - 250.0f + 260.0f * static_cast<float>(i), 92.0f, 240.0f, 44.0f}, tabs[i],
                    app.tradingTab == i, theme::GOLD_BRIGHT) &&
            app.tradingTab != i)
        {
            app.tradingTab = i;
            app.tradingSelection = 0;
        }
    }
    if (IsKeyPressed(KEY_TAB))
    {
        app.tradingTab = 1 - app.tradingTab;
        app.tradingSelection = 0;
    }

    const Rectangle panel = {40.0f, 156.0f, 1200.0f, 540.0f};
    drawPanel(panel, theme::GOLD_BRIGHT);
    if (app.tradingTab == RELIC_TAB)
    {
        runRelicTab(app, time);
    }
    else
    {
        runSkinTab(app, time);
    }

    if (!app.tradingMessage.empty())
    {
        drawStringCentered(app.tradingMessage.c_str(), SCREEN_CENTER_X, 704.0f, 22.0f,
                           app.tradingMessageIsError ? theme::DANGER : theme::SUCCESS, FontStyle::Bold);
    }
    else
    {
        drawStringCentered("Tab - switch      Arrows - choose      Enter - sell / buy", SCREEN_CENTER_X, 704.0f, 20.0f,
                           theme::PARCHMENT, FontStyle::Body);
    }

    if (drawButton({SCREEN_CENTER_X - 150.0f, 736.0f, 300.0f, 50.0f}, "Back", false) || IsKeyPressed(KEY_ESCAPE))
    {
        changeScreen(app, Screen::MainMenu);
    }
}
