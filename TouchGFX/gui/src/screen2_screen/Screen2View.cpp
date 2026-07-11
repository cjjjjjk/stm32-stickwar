#include <gui/screen2_screen/Screen2View.hpp>

// Khai báo biến toàn cục để truyền lệnh sang Screen1
extern bool global_isPvE;
extern int global_botDifficulty;
extern uint8_t global_pendingMode;

Screen2View::Screen2View() :
    buttonCallback(this, &Screen2View::buttonClickHandler)
{
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    // Gắn Callback lắng nghe sự kiện click cho các nút
    pvp.setAction(buttonCallback);
    pve_easy.setAction(buttonCallback);
    pve_medium.setAction(buttonCallback);
    pve_hard.setAction(buttonCallback);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

// Xử lý logic khi người chơi chọn chế độ
void Screen2View::buttonClickHandler(const AbstractButton& src)
{
    if (&src == &pvp) {
        global_isPvE = false;
    }
    else if (&src == &pve_easy) {
        global_isPvE = true;
        global_botDifficulty = 1;
    }
    else if (&src == &pve_medium) {
        global_isPvE = true;
        global_botDifficulty = 2;
    }
    else if (&src == &pve_hard) {
        global_isPvE = true;
        global_botDifficulty = 3;
    }

    // Chuyển sang màn hình chiến đấu
    application().gotoScreen1ScreenNoTransition();
}

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();

    if (global_pendingMode == 0xFF) {
        // Reset state
        global_pendingMode = 0;

        // Apply selected mode
        if (current_highlight_mode == 1) {
            global_isPvE = false;
        } else if (current_highlight_mode == 2) {
            global_isPvE = true;
            global_botDifficulty = 1;
        } else if (current_highlight_mode == 3) {
            global_isPvE = true;
            global_botDifficulty = 2;
        } else if (current_highlight_mode == 4) {
            global_isPvE = true;
            global_botDifficulty = 3;
        } else {
            // Default to PvP if confirmed without selecting
            global_isPvE = false;
        }

        // Transition to Screen1
        application().gotoScreen1ScreenNoTransition();
    }
    else if (global_pendingMode != 0 && global_pendingMode != current_highlight_mode && global_pendingMode != 0xFF) {
        current_highlight_mode = global_pendingMode;

        // Reset all buttons pressed state
        pvp.setPressed(false);
        pve_easy.setPressed(false);
        pve_medium.setPressed(false);
        pve_hard.setPressed(false);

        // Highlight selected button
        if (current_highlight_mode == 1) pvp.setPressed(true);
        else if (current_highlight_mode == 2) pve_easy.setPressed(true);
        else if (current_highlight_mode == 3) pve_medium.setPressed(true);
        else if (current_highlight_mode == 4) pve_hard.setPressed(true);

        // Invalidate buttons to force redraw
        pvp.invalidate();
        pve_easy.invalidate();
        pve_medium.invalidate();
        pve_hard.invalidate();
    }
}
