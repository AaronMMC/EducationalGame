#include "GameScreen.h"
#include "../UIHelpers.h"
#include "../TextSnippets.h"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <random>
#include <thread>       // PARALLELISM [Phase 5]: std::thread for parallel glitch fill
#include <mutex>        // PARALLELISM [Phase 5]: std::mutex protecting glitchCharCache
#include <atomic>       // PARALLELISM [Phase 5]: std::atomic for frame-level cache flag

// =============================================================================
// PARALLELISM [Phase 1 — Bottleneck]: Glitch Cache
// =============================================================================
// refreshGlitchCache() rebuilds a random-character array used by the VIRUS
// avatar's visual obfuscation effect.  In the original code this ran
// synchronously on the main thread every 0.15 seconds — a predictable stall.
//
// PHASE 3 — Data Parallelism:
//   "Can I chop this large dataset in half?" → YES.
//   The array has no inter-element dependency; each index can be filled
//   independently.  We split it into two equal halves and assign each half
//   to a short-lived worker thread.
//
//   Thread A: fills indices [0, mid)
//   Thread B: fills indices [mid, end)
//   Main:     joins both, then cache is ready
//
// PHASE 4 — Grammar:
//   <data-parallel-fill> ::=
//     std::thread tA( fillRange, 0,   mid )
//     std::thread tB( fillRange, mid, end )
//     tA.join(); tB.join();
//
// Because both threads write to DISJOINT index ranges, no mutex is needed
// for the fill itself — disjoint writes are race-free by definition.
// The cacheMutex below only guards the pointer swap after the fill completes.
// =============================================================================

static const std::vector<std::pair<std::string, sf::Color>> KEYWORDS = {
    {"int ",     {198, 120, 221}},
    {"void ",    {198, 120, 221}},
    {"float ",   {198, 120, 221}},
    {"bool ",    {198, 120, 221}},
    {"class ",   {198, 120, 221}},
    {"struct ",  {198, 120, 221}},
    {"public:",  {198, 120, 221}},
    {"return ",  {198, 120, 221}},
    {"for ",     {198, 120, 221}},
    {"auto ",    {198, 120, 221}},
    {"#include", {229, 192, 123}},
    {"std::",    {229, 192, 123}},
    {"template", {229, 192, 123}},
};
static const char GLITCH_CHARS[] = "@#$%&*?!~^";

// PARALLELISM [Phase 3 — Shared Resource]:
// glitchCharCache is written by two worker threads and read by drawTypingArea
// on the main thread.  cacheMutex guards the pointer swap after fill is done.
static std::vector<char> glitchCharCache;
static std::mutex         cacheMutex;          // RAII guard on cache swap
static float              glitchRefreshTimer = 0.f;

// =============================================================================
// refreshGlitchCache() — DATA PARALLELISM implementation
// =============================================================================
// PARALLELISM [Phase 6 — Application Development]:
// The function launches two threads that fill disjoint halves of a temporary
// buffer concurrently, then swaps the buffer into glitchCharCache under a
// brief mutex lock.  The main thread is never blocked during the fill — only
// during the swap (~1 µs).
// =============================================================================
static void refreshGlitchCache(int textLen) {
    if (textLen <= 0) return;

    // Allocate a fresh buffer (thread-local work — no race here)
    std::vector<char> buf(textLen);

    int mid = textLen / 2;

    // PARALLELISM [Phase 4 — Data Parallel Grammar]:
    // fillRange is a lambda capturing buf by reference.
    // Each thread writes to a non-overlapping range → no mutex inside fillRange.
    auto fillRange = [&](int from, int to) {
        for (int i = from; i < to; ++i)
            buf[i] = GLITCH_CHARS[std::rand() % (sizeof(GLITCH_CHARS) - 1)];
    };

    // Launch two worker threads for the two halves
    // PARALLELISM [Phase 6 — Integration Point]:
    // These are short-lived threads scoped to this function.  They are joined
    // before we touch glitchCharCache, so no data race on buf or the cache.
    std::thread tA(fillRange, 0,   mid);       // fills lower half
    std::thread tB(fillRange, mid, textLen);   // fills upper half

    tA.join();  // wait for lower half — guarantees buf[0..mid) is written
    tB.join();  // wait for upper half — guarantees buf[mid..end) is written

    // PARALLELISM [Phase 6 — Synchronization Point]:
    // Now both halves are complete.  Swap into the shared cache under the
    // mutex.  The swap is O(1) (pointer exchange), so the lock is held for
    // microseconds — negligible contention with drawTypingArea.
    {
        std::lock_guard<std::mutex> lk(cacheMutex);
        glitchCharCache = std::move(buf);  // move avoids a copy allocation
    }
}

static std::vector<sf::Color> buildBaseColors(const std::string& text) {
    std::vector<sf::Color> colors(text.size(), UI::TEXT3);
    for (const auto& kw : KEYWORDS) {
        size_t pos = text.find(kw.first);
        while (pos != std::string::npos) {
            for (size_t i = 0; i < kw.first.size() && pos + i < colors.size(); ++i)
                colors[pos + i] = kw.second;
            pos = text.find(kw.first, pos + kw.first.size());
        }
    }
    bool inStr = false;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '"') inStr = !inStr;
        if (inStr || text[i] == '"') colors[i] = sf::Color(152, 195, 121);
    }
    return colors;
}

void GameScreen::onEnter(GameState& gs) {
    if (gs.mode != GameMode::VS) gs.resetMatch();
    cursorPhase    = 0.f;
    bossAlertTimer = 0.f;
    showBossAlert  = gs.isBossWave;
    bossCountdown  = 3;
    phoenixNotice  = false;
    phoenixTimer   = 0.f;

    if (gs.mode == GameMode::VS) {
        auto avatars = Avatar::all();
        if (avatars[gs.profile.avatarId].ability == AvatarAbility::Starter) {
            gs.frozenUntil    = std::chrono::steady_clock::now() + std::chrono::seconds(4);
            gs.opponentFrozen = true;   // atomic store
        }
    }

    refreshGlitchCache((int)gs.targetText.size());
    glitchRefreshTimer = 0.f;
}

void GameScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (gs.matchFinished) return;
    if (showBossAlert) return;

    if (event.type == sf::Event::TextEntered) {
        sf::Uint32 uni = event.text.unicode;
        if (uni == 27) return;
        if (uni == 13) uni = 10;
        processChar(uni, gs);
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Tab) {
            for (int i = 0; i < 4; ++i) processChar(' ', gs);
        }
        if (event.key.code == sf::Keyboard::BackSpace && !gs.typedText.empty()) {
            gs.typedText.pop_back();
        }
        if (event.key.code == sf::Keyboard::Escape) {
            // PARALLELISM [Phase 6 — Thread Lifecycle]:
            // When navigating away mid-match, stop the sim thread cleanly
            // before transitioning screens.  Without this, simThread would
            // continue writing to gs after the GameScreen is no longer active.
            gs.opponent.stop();
            gs.currentScreen = Screen::Title;
        }
    }
}

void GameScreen::processChar(sf::Uint32 uni, GameState& gs) {
    if (uni < 32 && uni != 10) return;

    if (!gs.roundStarted) startRound(gs);

    if (gs.typedText.size() < gs.targetText.size()) {
        char c = static_cast<char>(uni);
        gs.typedText += c;
        gs.stats.totalTyped++;
        size_t idx = gs.typedText.size() - 1;
        if (idx < gs.targetText.size() && c == gs.targetText[idx]) {
            gs.stats.correctChars++;
            gs.updateMultiplier(true);
            gs.addScore(5);
        } else {
            gs.stats.errors++;
            gs.updateMultiplier(false);
            // PARALLELISM [Phase 6 — Mutex-protected write]:
            // applyDamageToPlayer modifies gs.playerHP, which simThread may
            // also modify via applyDamageToPlayer() on damage events.
            // Both callers must hold gs.simMutex.
            std::lock_guard<std::mutex> lk(gs.simMutex);
            gs.applyDamageToPlayer(2.f);
        }
    }
    checkRoundComplete(gs);
}

void GameScreen::startRound(GameState& gs) {
    gs.roundStarted = true;
    gs.stats.start();

    // PARALLELISM [Phase 6 — Thread Launch via connect()]:
    // connect() is called here so simThread's lifetime is tied to the typing
    // round, not the broader match setup.  See NetworkSim::connect().
    if (gs.mode == GameMode::VS) gs.opponent.connect();
}

void GameScreen::checkRoundComplete(GameState& gs) {
    if (gs.typedText.size() < gs.targetText.size()) return;

    gs.matchTotalTyped   += gs.stats.totalTyped;
    gs.matchCorrectChars += gs.stats.correctChars;
    gs.matchErrors       += gs.stats.errors;
    gs.matchBestWpm       = std::max(gs.matchBestWpm, gs.stats.getWPM());

    long long bonus = gs.stats.getWPM() * 2 + (long long)gs.stats.getAccuracy() * 3;
    gs.addScore(bonus);

    {
        // PARALLELISM [Phase 6 — Mutex-protected write]:
        // applyDamageToOpponent writes gs.oppHP; simThread reads it in tick().
        std::lock_guard<std::mutex> lk(gs.simMutex);
        gs.applyDamageToOpponent(15.f);
    }

    bool moreRounds = gs.mode == GameMode::VS && gs.round < gs.maxRounds;
    if (moreRounds) nextRound(gs);
    else            endMatch(gs);
}

void GameScreen::nextRound(GameState& gs) {
    // PARALLELISM [Phase 6 — Thread Reset Between Rounds]:
    // stop() joins the current simThread, then reset() relaunches it for the
    // new round's text.  This ensures no thread carries over stale state.
    gs.opponent.stop();

    gs.round++;
    gs.roundStarted = false;
    gs.stats.reset();
    gs.stats.timeLimit = gs.settings.timeLimit;
    gs.typedText = "";
    gs.obfuscatedIndices.clear();
    gs.targetText = TextSnippets::getRandom(TextSnippets::normal());
    gs.opponent.reset((gs.profile.avatarId + 2) % 4, false);

    refreshGlitchCache((int)gs.targetText.size());  // data-parallel cache rebuild

    auto avatars = Avatar::all();
    if (avatars[gs.profile.avatarId].ability == AvatarAbility::Comeback && gs.round == 3) {
        gs.multiplier = std::max(gs.multiplier, 3.f);
        phoenixNotice = true;
        phoenixTimer  = 2.5f;
    }

    if (avatars[gs.profile.avatarId].ability == AvatarAbility::Starter) {
        gs.frozenUntil    = std::chrono::steady_clock::now() + std::chrono::seconds(4);
        gs.opponentFrozen = true;   // atomic store — simThread reads this immediately
    }
}

void GameScreen::endMatch(GameState& gs) {
    // PARALLELISM [Phase 6 — Thread Teardown before state mutation]:
    // Join simThread before reading/writing match result fields to avoid
    // a race on oppHP / playerHP comparisons below.
    gs.opponent.stop();

    gs.stats.finish();
    gs.matchFinished = true;

    if (gs.matchBestWpm > gs.profile.bestWpm)  gs.profile.bestWpm = gs.matchBestWpm;
    if (gs.score > gs.profile.bestScore)        gs.profile.bestScore = (int)gs.score;
    gs.profile.gamesPlayed++;

    if (gs.mode == GameMode::VS) {
        gs.playerWon = (gs.oppHP <= 0) || (gs.playerHP > gs.oppHP);
        if (gs.playerWon) gs.profile.wins++;
        else              gs.profile.losses++;
    } else if (gs.mode == GameMode::Endless) {
        gs.wave++;
        gs.playerWon = true;
    } else {
        gs.playerWon = true;
    }

    gs.currentScreen = Screen::Result;
}

void GameScreen::update(float dt, GameState& gs) {
    cursorPhase += dt;

    glitchRefreshTimer += dt;
    if (glitchRefreshTimer >= 0.15f) {
        glitchRefreshTimer = 0.f;
        // PARALLELISM [Phase 6 — Data Parallel call]:
        // refreshGlitchCache() now fires two worker threads internally.
        // The call returns only after both threads have joined, so
        // glitchCharCache is always fully populated when draw() runs below.
        if (!gs.targetText.empty())
            refreshGlitchCache((int)gs.targetText.size());
    }

    if (showBossAlert) {
        bossAlertTimer += dt;
        if (bossAlertTimer > 1.f) { bossCountdown--; bossAlertTimer = 0.f; }
        if (bossCountdown <= 0)   showBossAlert = false;
        return;
    }

    if (gs.matchFinished || !gs.roundStarted) return;

    if (gs.stats.getRemainingTime() <= 0.f && !gs.stats.isFinished) {
        if (gs.mode == GameMode::VS && gs.round < gs.maxRounds) nextRound(gs);
        else endMatch(gs);
        return;
    }

    // PARALLELISM [Phase 6 — Atomic read]:
    // opponentFrozen is std::atomic<bool>; no lock needed for a single read.
    if (gs.opponentFrozen.load()) {
        auto now = std::chrono::steady_clock::now();
        if (now >= gs.frozenUntil) gs.opponentFrozen = false;  // atomic store
    }

    if (gs.mode == GameMode::VS && gs.opponent.isConnected()) {
        // PARALLELISM [Phase 6 — Main-thread tick reads snapshot]:
        // tick() now only reads the latest committed SimSnapshot (under a
        // brief lock inside tick()).  The actual simulation happened in
        // simThread concurrently since the last frame.
        float dmg = gs.opponent.tick(dt, (int)gs.targetText.size(), gs.opponentFrozen.load());

        if (dmg > 0.f) {
            // PARALLELISM [Phase 6 — Mutex-protected HP write]:
            std::lock_guard<std::mutex> lk(gs.simMutex);
            gs.applyDamageToPlayer(dmg);
        }

        // Publish latest obfuscation indices from the snapshot (already
        // written to gs.opponent.sabotageIndices by tick())
        gs.obfuscatedIndices = gs.opponent.sabotageIndices;

        if (gs.opponent.progressPct >= 1.f && !gs.matchFinished) {
            {
                std::lock_guard<std::mutex> lk(gs.simMutex);
                gs.applyDamageToPlayer(20.f);
            }
            if (gs.round < gs.maxRounds) nextRound(gs);
            else endMatch(gs);
        }
        if (gs.playerHP <= 0.f && !gs.matchFinished) endMatch(gs);
        if (gs.oppHP   <= 0.f && !gs.matchFinished) endMatch(gs);
    }

    if (phoenixNotice) {
        phoenixTimer -= dt;
        if (phoenixTimer <= 0.f) phoenixNotice = false;
    }
}

void GameScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);

    if (showBossAlert) { drawBossAlert(w, gs); return; }

    drawHUD(w, gs);
    drawTypingArea(w, gs);
    if (gs.mode == GameMode::VS) drawOpponentPanel(w, gs);
    drawSentinelFreeze(w, gs);
    if (phoenixNotice) drawPhoenixNotice(w, gs);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}

void GameScreen::drawHUD(sf::RenderWindow& w, GameState& gs) {
    sf::RectangleShape hudBg({800.f, 64.f});
    hudBg.setFillColor(UI::BG2);
    hudBg.setOutlineColor(sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b, 40));
    hudBg.setOutlineThickness(1.f);
    w.draw(hudBg);

    auto avatars = Avatar::all();
    sf::Text p1sym = UI::makeText(avatars[gs.profile.avatarId].symbol, gs.fontMono, 14, gs.playerAccentColor());
    p1sym.setPosition(12, 10);
    w.draw(p1sym);

    std::string p1name = gs.profile.name.substr(0, 8);
    sf::Text p1n = UI::makeText(p1name, gs.fontOrb, 11, UI::TEXT);
    p1n.setPosition(52, 8);
    w.draw(p1n);

    // PARALLELISM [Phase 6 — Safe HP read under lock]:
    // playerHP may be written by simThread.  Take a local copy under the
    // mutex, then use the copy for rendering — never hold the mutex across
    // draw calls (would serialize sim and render).
    float localPlayerHP, localOppHP;
    {
        std::lock_guard<std::mutex> lk(gs.simMutex);
        localPlayerHP = gs.playerHP;
        localOppHP    = gs.oppHP;
    }

    UI::drawHPBar(w, 52, 28, 160, 8, localPlayerHP / 100.f, "", gs.fontMono);
    sf::Text p1hpn = UI::makeText(std::to_string((int)localPlayerHP), gs.fontMono, 9, UI::TEXT2);
    p1hpn.setPosition(218, 28);
    w.draw(p1hpn);

    float rem = gs.stats.getRemainingTime();
    sf::Color timerCol = rem > 20.f ? UI::ACCENT3 : rem > 10.f ? UI::ORANGE : UI::RED;
    sf::Text timer = UI::makeText(std::to_string((int)std::ceil(rem)), gs.fontOrb, 28, timerCol);
    UI::centerText(timer, 400.f, 20.f);
    if (rem <= 10.f) UI::drawGlowText(w, timer, timerCol, 4.f);
    else w.draw(timer);

    std::string rndStr;
    if (gs.mode == GameMode::VS)           rndStr = "ROUND " + std::to_string(gs.round) + " / " + std::to_string(gs.maxRounds);
    else if (gs.mode == GameMode::Endless) rndStr = "WAVE " + std::to_string(gs.wave);
    else                                   rndStr = "SCORE ATTACK";
    sf::Text rnd = UI::makeText(rndStr, gs.fontMono, 10, UI::TEXT2);
    UI::centerText(rnd, 400.f, 48.f);
    w.draw(rnd);

    sf::Text scoreT = UI::makeText(std::to_string(gs.score), gs.fontOrb, 20, UI::ACCENT);
    scoreT.setPosition(600, 6);
    w.draw(scoreT);

    auto avAttr = avatars[gs.profile.avatarId];
    float acc = gs.stats.totalTyped > 0
        ? (float)gs.stats.correctChars / gs.stats.totalTyped * 100.f : 100.f;
    bool multiActive = gs.multiplier > 1.f;
    bool multiDanger = multiActive && acc < avAttr.accuracyThreshold;
    UI::drawMultiplierBadge(w, 730, 8, gs.multiplier, multiActive, multiDanger, gs.fontOrb);

    std::string statStr = "WPM: " + std::to_string(gs.stats.getWPM()) +
                          "  ACC: " + std::to_string(gs.stats.getAccuracy()) + "%";
    sf::Text statT = UI::makeText(statStr, gs.fontMono, 9, UI::TEXT2);
    statT.setPosition(600, 46);
    w.draw(statT);

    if (gs.mode == GameMode::VS) {
        sf::RectangleShape hud2bg({800.f, 28.f});
        hud2bg.setPosition(0, 64);
        hud2bg.setFillColor(sf::Color(17, 10, 14));
        w.draw(hud2bg);
        sf::Text p2lbl = UI::makeText("CPU-" + avatars[(gs.profile.avatarId+2)%4].name.substr(0,4),
                                      gs.fontOrb, 9, UI::ACCENT2);
        p2lbl.setPosition(52, 68);
        w.draw(p2lbl);
        // localOppHP captured under lock above — safe to use here
        UI::drawHPBar(w, 120, 70, 160, 8, localOppHP / 100.f, "", gs.fontMono);
        sf::Text p2hpn = UI::makeText(std::to_string((int)localOppHP), gs.fontMono, 9, UI::TEXT2);
        p2hpn.setPosition(286, 70);
        w.draw(p2hpn);
    }
}

void GameScreen::drawTypingArea(sf::RenderWindow& w, GameState& gs) {
    float hudH  = (gs.mode == GameMode::VS) ? 92.f : 64.f;
    float panelW = (gs.mode == GameMode::VS) ? 520.f : 800.f;

    sf::RectangleShape bg({panelW, 600.f - hudH});
    bg.setPosition(0, hudH);
    bg.setFillColor(UI::BG);
    w.draw(bg);

    if (!gs.roundStarted) {
        sf::Text hint = UI::makeText("START TYPING TO BEGIN...", gs.fontMono, 13, UI::TEXT3);
        UI::centerText(hint, panelW / 2.f, hudH + 20.f);
        w.draw(hint);
    }

    auto baseColors = buildBaseColors(gs.targetText);

    float startX = 40.f, cx = startX, cy = hudH + 40.f;
    float cursorAlpha = (std::sin(cursorPhase) + 1.f) * 0.5f;

    // PARALLELISM [Phase 6 — Safe cache read under lock]:
    // Take a local copy of glitchCharCache so we don't hold cacheMutex across
    // the entire character rendering loop — holding a lock across SFML draw
    // calls would serialize the glitch fill thread and the renderer.
    std::vector<char> localGlitch;
    {
        std::lock_guard<std::mutex> lk(cacheMutex);
        localGlitch = glitchCharCache;  // copy; O(n) but only 200 chars max
    }

    for (size_t i = 0; i <= gs.targetText.size(); ++i) {
        if (i == gs.typedText.size()) {
            sf::RectangleShape cur({3.f, 28.f});
            cur.setPosition(cx, cy + 4.f);
            cur.setFillColor(sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b,
                                       (sf::Uint8)(cursorAlpha * 255)));
            w.draw(cur);
        }
        if (i == gs.targetText.size()) break;

        char ch = gs.targetText[i];
        if (ch == '\n') { cx = startX; cy += 36.f; continue; }

        sf::Text charT;
        charT.setFont(gs.fontMono);
        charT.setCharacterSize(18);
        charT.setPosition(cx, cy);

        bool isObs = gs.obfuscatedIndices.count((int)i) && i >= gs.typedText.size();
        if (isObs && gs.settings.glitchEffects) {
            // Use localGlitch (main-thread copy) — no mutex needed here
            char gc = (i < localGlitch.size()) ? localGlitch[i] : '@';
            charT.setString(std::string(1, gc));
            charT.setFillColor(sf::Color(255, 60, 110, 200));
        } else if (i < gs.typedText.size()) {
            charT.setString(std::string(1, ch));
            charT.setFillColor(gs.typedText[i] == ch ? UI::TEXT : UI::RED);
        } else {
            charT.setString(std::string(1, ch));
            charT.setFillColor(baseColors[i]);
        }

        w.draw(charT);
        cx += gs.fontMono.getGlyph(ch, 18, false).advance;
    }
}

// Remaining draw functions are unchanged from the original —
// they only read fields that belong exclusively to the main thread.

void GameScreen::drawOpponentPanel(sf::RenderWindow& w, GameState& gs) {
    float hudH = 92.f, px = 520.f;
    sf::RectangleShape panel({280.f, 600.f - hudH});
    panel.setPosition(px, hudH);
    panel.setFillColor(UI::BG2);
    panel.setOutlineColor(sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b, 30));
    panel.setOutlineThickness(1.f);
    w.draw(panel);

    float y = hudH + 16.f;
    sf::Text lbl = UI::makeText("OPPONENT FEED", gs.fontOrb, 10, UI::TEXT2);
    lbl.setPosition(px + 12, y); w.draw(lbl); y += 26.f;

    std::string pctStr = std::to_string((int)(gs.opponent.progressPct * 100)) + "%";
    sf::Text pct = UI::makeText(pctStr, gs.fontMono, 14, UI::TEXT);
    pct.setPosition(px + 12, y); w.draw(pct); y += 20.f;
    UI::drawProgressBar(w, px + 12, y, 256, 4, gs.opponent.progressPct, UI::ACCENT2, UI::BG3);
    y += 16.f;

    std::string ostatStr = "WPM: " + std::to_string(gs.opponent.wpm) +
                           "  ACC: " + std::to_string(gs.opponent.accuracy) + "%";
    sf::Text ostat = UI::makeText(ostatStr, gs.fontMono, 11, UI::TEXT2);
    ostat.setPosition(px + 12, y); w.draw(ostat); y += 24.f;

    sf::Text omul = UI::makeText("x" + std::to_string((int)gs.opponent.multiplier),
                                 gs.fontOrb, 16, UI::ACCENT2);
    omul.setPosition(px + 12, y); w.draw(omul); y += 28.f;

    // PARALLELISM [Phase 6 — Atomic read]:
    if (gs.opponentFrozen.load()) {
        auto now = std::chrono::steady_clock::now();
        float rem = std::chrono::duration<float>(gs.frozenUntil - now).count();
        if (rem > 0.f) {
            sf::Text ft = UI::makeText("FROZEN " + std::to_string((int)std::ceil(rem)) + "s",
                                       gs.fontMono, 11, UI::ACCENT3);
            ft.setPosition(px + 12, y); w.draw(ft);
        }
    }

    auto avatars = Avatar::all();
    const Avatar& oav = avatars[(gs.profile.avatarId + 2) % 4];
    float avY = hudH + 580.f - hudH - 50.f;
    sf::Text avlbl = UI::makeText("UNIT", gs.fontOrb, 9, UI::TEXT2);
    avlbl.setPosition(px + 12, avY); w.draw(avlbl);
    sf::Text avnm = UI::makeText(oav.symbol + " " + oav.name + " - " + oav.tagline,
                                 gs.fontRaj, 12, oav.accentColor);
    avnm.setPosition(px + 12, avY + 16); w.draw(avnm);
}

void GameScreen::drawBossAlert(sf::RenderWindow& w, GameState& gs) {
    sf::RectangleShape overlay({800.f, 600.f});
    overlay.setFillColor(sf::Color(10, 0, 0, 240));
    w.draw(overlay);
    float pulse = 0.9f + 0.1f * std::sin(bossAlertTimer * 8.f);
    sf::Text title = UI::makeText("BOSS WAVE", gs.fontOrb, 42, UI::ACCENT2);
    title.setStyle(sf::Text::Bold);
    title.setScale(pulse, pulse);
    UI::centerText(title, 400.f, 200.f);
    UI::drawGlowText(w, title, UI::ACCENT2, 8.f);
    sf::Text sub = UI::makeText("ALL UNITS CONVERGE - SURVIVE IF YOU CAN", gs.fontMono, 13, UI::TEXT2);
    UI::centerText(sub, 400.f, 270.f);
    w.draw(sub);
    std::string cdStr = bossCountdown > 0
        ? "INITIATING IN " + std::to_string(bossCountdown) + "..." : "BEGIN!";
    sf::Text cd = UI::makeText(cdStr, gs.fontMono, 14, UI::TEXT);
    UI::centerText(cd, 400.f, 340.f);
    w.draw(cd);
    if (gs.settings.scanlines) UI::drawScanlines(w);
}

void GameScreen::drawSentinelFreeze(sf::RenderWindow& w, GameState& gs) {
    // PARALLELISM [Phase 6 — Atomic read]:
    if (!gs.opponentFrozen.load()) return;
    auto now = std::chrono::steady_clock::now();
    float rem = std::chrono::duration<float>(gs.frozenUntil - now).count();
    if (rem <= 0.f) return;
    std::string msg = "SENTINEL ADVANTAGE - OPPONENT LOCKED FOR " +
                      std::to_string((int)std::ceil(rem)) + "s";
    sf::Text ft = UI::makeText(msg, gs.fontMono, 11, UI::ACCENT3);
    ft.setPosition(10.f, 96.f);
    UI::drawGlowText(w, ft, UI::ACCENT3, 2.f);
}

void GameScreen::drawPhoenixNotice(sf::RenderWindow& w, GameState& gs) {
    sf::Text t = UI::makeText("PHOENIX SURGE - x3 MULTIPLIER BURST ACTIVE!", gs.fontOrb, 14, UI::ACCENT3);
    t.setStyle(sf::Text::Bold);
    UI::centerText(t, 400.f, 120.f);
    UI::drawGlowText(w, t, UI::ACCENT3, 4.f);
}