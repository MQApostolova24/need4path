#pragma once

#include "core/config.h"
#include "core/game.h"
#include "core/location.h"
#include "raylib.h"

// Pictures of everything that can appear inside a pyramid, drawn from simple shapes.
// Every function draws the version that belongs to the given location:
//
//                 Egypt               Mexico               Cambodia
//   key           Ankh key            Jade glyph           Lotus key
//   door          Eye of Horus        Serpent glyph        Stone face
//   treasure      Gold scarab         Gold bead            Silver coin
//   gem           Lapis amulet        Jade mask            Sapphire
//   relic         Golden mask         Obsidian sun disc    Golden lotus
//   time pickup   Hourglass           Sun idol             Rain charm
//   trap          Scarab pit          Spike pit            Cobra nest
//   guardian      Mummy               Jaguar spirit        Naga serpent
//   timer         Falling sand        Solar eclipse        Rising flood

void drawKeySprite(Location location, Vector2 center, float size);
void drawDoorSprite(Location location, Rectangle tile, float time);
void drawTreasureSprite(Location location, Vector2 center, float size, float time);
void drawGemSprite(Location location, Vector2 center, float size, float time);
void drawRelicSprite(Location location, Vector2 center, float size, float time);
void drawTimePickupSprite(Location location, Vector2 center, float size, float time);
void drawTrapSprite(Location location, Vector2 center, float size, bool sprung);
void drawGuardianSprite(Location location, Vector2 center, float size, float time);
void drawExitSprite(Location location, Rectangle tile, float time);
// The explorer in one of the skins (0 .. SKIN_COUNT-1), holding a torch or a flashlight
// on the side it faces.
void drawExplorerSprite(Vector2 center, float size, float time, int skinId, Direction facing, bool flashlight);

// The location's timer picture. fraction = remaining time from 1 (full) to 0 (time is up).
void drawTimerEmblem(Location location, Vector2 center, float size, float fraction, float time);
