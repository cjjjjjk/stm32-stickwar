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
    playagain.setAlpha(0);
    playagain.setTouchable(false);

    // =========================================================
    // 3. LOGIC BEST OF 3: QUYẾT ĐỊNH NÚT NÀO ĐƯỢC PHÉP XUẤT HIỆN
    // =========================================================
    int wA = presenter->getWinsA();
    int wB = presenter->getWinsB();

    if (wA == 2 || wB == 2) 
    {
        // Đã có vô địch -> Bật nút Home, tắt nút Play Again
        home.setVisible(true);
        playagain.setVisible(false);
    } 
    else 
    {
        // Mới xong 1 hiệp, chưa ai đủ 2 win -> Bật Play Again, tắt Home
        home.setVisible(false);
        playagain.setVisible(true);
    }

    // Yêu cầu vẽ lại trạng thái ẩn/hiện
    home.invalidate();
    playagain.invalidate();

    // 4. Gắn Callback cho nút
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
    // BƯỚC 1: Hiện dần nút (Chỉ hiện nút nào đang được SetVisible(true) ở trên)
    else if (animationStep == 1)
    {
        // Lấy alpha của nút đang được hiển thị
        int currentAlpha = home.isVisible() ? home.getAlpha() : playagain.getAlpha();

        if (currentAlpha < 150) { // Nút của bạn max trong suốt là 150
            int newAlpha = currentAlpha + 3;
            if (newAlpha > 150) newAlpha = 150;

            // Fade in nút Home nếu nó đang được chọn hiển thị
            if (home.isVisible()) {
                home.setAlpha(newAlpha);
                home.invalidate();
            }
            
            // Fade in nút Play Again nếu nó đang được chọn hiển thị
            if (playagain.isVisible()) {
                playagain.setAlpha(newAlpha);
                playagain.invalidate();
            }
        } else {
            // Mở khóa cảm ứng cho nút tương ứng
            if (home.isVisible()) home.setTouchable(true);
            if (playagain.isVisible()) playagain.setTouchable(true);
            animationStep = 2;
        }
    }
}

// Xử lý khi nhấn nút
void Screen3View::buttonClickHandler(const AbstractButton& src)
{
    if (&src == &home) {
        // Về menu (khi quay lại Screen2, logic setupScreen bên đó sẽ tự gọi ResetGame)
        application().gotoScreen2ScreenNoTransition(); 
    }
    else if (&src == &playagain) {
        // CỰC KỲ QUAN TRỌNG: Tăng hiệp (Round) lên trước khi quay lại trận
        presenter->goToNextRound(); 
        
        // Chơi lại ván mới
        application().gotoScreen1ScreenNoTransition(); 
    }
}