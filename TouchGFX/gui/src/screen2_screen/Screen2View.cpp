#include <gui/screen2_screen/Screen2View.hpp>

// Khai báo biến toàn cục để truyền lệnh sang Screen1
extern bool global_isPvE;
extern int global_botDifficulty;

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
