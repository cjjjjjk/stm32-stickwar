#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void handleTickEvent();
protected:
    // Các biến AI cho chế độ PvE
        uint32_t botMoveTimer;
        uint32_t botActionTimer;
        uint32_t playerBlockTimer;
        uint8_t botActionStep;
        bool isPvE; // Cờ đánh dấu đang chơi với Máy
        int botDifficulty;
};

#endif // SCREEN1VIEW_HPP
