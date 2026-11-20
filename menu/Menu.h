#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

namespace ui {

    struct MenuItem {
        std::string label;
        std::function<void()> action;
        bool enabled = true;
        std::string shortcut;
    };

    struct Menu {
        std::string title;
        std::vector<MenuItem> items;
        Rectangle headerRect{};
        bool open = false;
    };

    class MenuBar {
    public:
        MenuBar() = default;

        int  addMenu(const std::string& title);   // возвращает индекс меню
        void addItem(int menuIdx,
            const std::string& label,
            std::function<void()> action,
            const std::string& shortcut = "",
            bool enabled = true);

        bool  update();
        void  draw() const;
        bool  isOpen() const;
        void  closeAll();
        float barHeight() const;

    private:
        std::vector<Menu> menus_;

        static constexpr float BAR_H = 28.0f;
        static constexpr float PAD_X = 14.0f;
        static constexpr float ITEM_H = 26.0f;
        static constexpr float SEP_H = 6.0f;
        static constexpr int   FONT = 18;

        Rectangle dropdownRect(const Menu& m) const;
        void      layoutHeaders();
    };

} // namespace ui