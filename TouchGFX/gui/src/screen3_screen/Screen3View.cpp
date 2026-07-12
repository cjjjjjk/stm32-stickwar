#include <gui/screen3_screen/Screen3View.hpp>

// Biến toàn cục lấy thông tin người thắng từ Screen1 (1 = Xanh thắng, 2 = Đỏ thắng)
extern int global_winner;

Screen3View::Screen3View() :
    buttonCallback(this, &Screen3View::buttonClickHandler)
{
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    // 1. Tàng hình cả 2 ảnh Win ban đầu
    bluewin.setAlpha(0);
    redwin.setAlpha(0);
    bluewin.invalidate();
    redwin.invalidate();

    // 2. Tàng hình 2 nút và khóa cảm ứng
    home.setAlpha(0);
    home.setTouchable(false);
    home.invalidate();

    playagain.setAlpha(0);
    playagain.setTouchable(false);
    playagain.invalidate();

    // 3. Gắn Callback cho nút
    home.setAction(buttonCallback);
    playagain.setAction(buttonCallback);

    animationStep = 0;
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}

void Screen3View::handleTickEvent()
{
    Screen3ViewBase::handleTickEvent();

    // BƯỚC 0: Tăng dần Alpha để hiện ảnh Win (tùy thuộc ai thắng)
    if (animationStep == 0)
    {
        Image& winImage = (global_winner == 1) ? bluewin : redwin;
        int currentAlpha = winImage.getAlpha();

        if (currentAlpha < 255) {
            int newAlpha = currentAlpha + 3;
            if (newAlpha > 255) newAlpha = 255;
            winImage.setAlpha(newAlpha);
            winImage.invalidate();
        } else {
            animationStep = 1;
        }
    }
    // BƯỚC 1: Hiện dần 2 nút tùy chọn
    else if (animationStep == 1)
    {
        int currentAlpha = home.getAlpha();

        if (currentAlpha < 150) {
            int newAlpha = currentAlpha + 3;
            if (newAlpha > 150) newAlpha = 150;

            home.setAlpha(newAlpha);
            playagain.setAlpha(newAlpha);
            home.invalidate();
            playagain.invalidate();
        } else {
            home.setTouchable(true); // Mở khóa cho phép bấm
            playagain.setTouchable(true);
            animationStep = 2;
        }
    }
}

// Xử lý khi nhấn nút
void Screen3View::buttonClickHandler(const AbstractButton& src)
{
    if (&src == &home) {
        application().gotoScreen2ScreenNoTransition(); // Trở về Menu
    }
    else if (&src == &playagain) {
        application().gotoScreen1ScreenNoTransition(); // Chơi lại ván mới
    }
}
void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    uint8_t wA = presenter->getWinsA();
    uint8_t wB = presenter->getWinsB();

    // 1. Kiểm tra xem đã có CHAMPION chưa (Có bên nào đạt 2 win)
    if (wA == 2 || wB == 2)
    {
        // Hiển thị CHAMPION
        if (wA == 2) {
            Unicode::snprintf(txtResultBuffer, TXTRESULT_SIZE, "P1 CHAMPION!");
        } else {
            Unicode::snprintf(txtResultBuffer, TXTRESULT_SIZE, "P2 CHAMPION!");
        }
        
        // Ẩn nút Next Round, hiện nút Về Menu
        btnNextRound.setVisible(false);
        btnBackMenu.setVisible(true);
    }
    else
    {
        // 2. Nếu chưa ai đủ 2 win -> Chuẩn bị đánh round tiếp
        Unicode::snprintf(txtResultBuffer, TXTRESULT_SIZE, "ROUND WINNER!");
        
        // Hiện nút Next Round, Ẩn nút Về Menu
        btnNextRound.setVisible(true);
        btnBackMenu.setVisible(false);
    }
    
    // Yêu cầu vẽ lại các widget vừa thay đổi trạng thái
    btnNextRound.invalidate();
    btnBackMenu.invalidate();
    txtResult.invalidate();
}