#include "Menu.h"

namespace ui {

    int MenuBar::addMenu(const std::string& title) {
        menus_.push_back({});
        menus_.back().title = title;
        return (int)menus_.size() - 1;
    }

    void MenuBar::addItem(int menuIdx,
        const std::string& label,
        std::function<void()> action,
        const std::string& shortcut,
        bool enabled) {
        if (menuIdx < 0 || menuIdx >= (int)menus_.size()) return;
        menus_[menuIdx].items.push_back({ label, std::move(action), enabled, shortcut });
    }

    void MenuBar::layoutHeaders() {
        float x = 0.0f;
        for (auto& m : menus_) {
            int tw = MeasureText(m.title.c_str(), FONT);
            m.headerRect = { x, 0, (float)(tw + PAD_X * 2), BAR_H };
            x += m.headerRect.width;
        }
    }

    Rectangle MenuBar::dropdownRect(const Menu& m) const {
        float w = MeasureText(m.title.c_str(), FONT) + 40.0f;
        for (auto& it : m.items) {
            float need = MeasureText(it.label.c_str(), FONT) + 40.0f;
            if (!it.shortcut.empty())
                need += MeasureText(it.shortcut.c_str(), FONT - 2) + 20.0f;
            if (need > w) w = need;
        }
        float h = 8.0f;
        for (auto& it : m.items) h += (it.label == "-") ? SEP_H : ITEM_H;
        return { m.headerRect.x, m.headerRect.y + m.headerRect.height, w, h };
    }

    bool MenuBar::update() {
        Vector2 mouse = GetMousePosition();
        bool consumed = false;

        layoutHeaders();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            bool hitHeader = false;
            for (size_t i = 0; i < menus_.size(); ++i) {
                if (CheckCollisionPointRec(mouse, menus_[i].headerRect)) {
                    bool wasOpen = menus_[i].open;
                    closeAll();
                    menus_[i].open = !wasOpen;
                    hitHeader = true;
                    consumed = true;
                    break;
                }
            }
            if (!hitHeader) {
                bool hitDropdown = false;
                for (auto& m : menus_) {
                    if (!m.open) continue;
                    if (CheckCollisionPointRec(mouse, dropdownRect(m))) { hitDropdown = true; break; }
                }
                if (!hitDropdown) closeAll();
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            bool someOpen = false;
            for (auto& m : menus_) if (m.open) someOpen = true;
            if (someOpen) {
                for (size_t i = 0; i < menus_.size(); ++i) {
                    if (CheckCollisionPointRec(mouse, menus_[i].headerRect) && !menus_[i].open) {
                        closeAll();
                        menus_[i].open = true;
                    }
                }
            }
        }

        for (auto& m : menus_) {
            if (!m.open) continue;
            Rectangle dd = dropdownRect(m);
            float y = dd.y + 4;
            for (auto& it : m.items) {
                if (it.label == "-") { y += SEP_H; continue; }
                Rectangle itemRect{ dd.x + 2, y, dd.width - 4, ITEM_H };
                bool ihover = it.enabled && CheckCollisionPointRec(mouse, itemRect);
                if (ihover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    closeAll();
                    if (it.action) it.action();
                    consumed = true;
                }
                y += ITEM_H;
            }
        }
        return consumed;
    }

    void MenuBar::draw() const {
        Vector2 mouse = GetMousePosition();

        DrawRectangle(0, 0, GetScreenWidth(), (int)BAR_H, Color{ 245, 245, 245, 255 });
        DrawLine(0, (int)BAR_H, GetScreenWidth(), (int)BAR_H, LIGHTGRAY);

        for (auto& m : menus_) {
            bool hover = CheckCollisionPointRec(mouse, m.headerRect);
            Color bg = m.open ? Color{ 200, 225, 255, 255 }
                : hover ? Color{ 225, 225, 225, 255 }
            : BLANK;
            DrawRectangleRec(m.headerRect, bg);
            DrawText(m.title.c_str(),
                (int)(m.headerRect.x + PAD_X),
                (int)(m.headerRect.y + (BAR_H - FONT) / 2),
                FONT, BLACK);
        }

        for (auto& m : menus_) {
            if (!m.open) continue;
            Rectangle dd = dropdownRect(m);
            DrawRectangleRec(dd, Color{ 250, 250, 250, 255 });
            DrawRectangleLinesEx(dd, 1.0f, GRAY);

            float y = dd.y + 4;
            for (auto& it : m.items) {
                if (it.label == "-") {
                    DrawLine((int)dd.x + 6, (int)(y + SEP_H / 2),
                        (int)(dd.x + dd.width - 6), (int)(y + SEP_H / 2),
                        LIGHTGRAY);
                    y += SEP_H;
                    continue;
                }
                Rectangle itemRect{ dd.x + 2, y, dd.width - 4, ITEM_H };
                bool ihover = it.enabled && CheckCollisionPointRec(mouse, itemRect);
                if (ihover) DrawRectangleRec(itemRect, Color{ 200, 225, 255, 255 });

                Color txt = it.enabled ? BLACK : GRAY;
                DrawText(it.label.c_str(),
                    (int)(itemRect.x + 10),
                    (int)(itemRect.y + (ITEM_H - FONT) / 2),
                    FONT, txt);

                if (!it.shortcut.empty()) {
                    int sw = MeasureText(it.shortcut.c_str(), FONT - 2);
                    DrawText(it.shortcut.c_str(),
                        (int)(itemRect.x + itemRect.width - sw - 10),
                        (int)(itemRect.y + (ITEM_H - FONT) / 2 + 1),
                        FONT - 2, GRAY);
                }
                y += ITEM_H;
            }
        }
    }

    bool MenuBar::isOpen() const {
        for (auto& m : menus_) if (m.open) return true;
        return false;
    }

    void MenuBar::closeAll() { for (auto& m : menus_) m.open = false; }
    float MenuBar::barHeight() const { return BAR_H; }

} // namespace ui