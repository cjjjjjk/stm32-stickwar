#include <gui/screen1_screen/Screen1View.hpp>
#include "cmsis_os.h"
#include <texts/TextKeysAndLanguages.hpp>
#include <stdlib.h> // Để dùng hàm abs()
bool global_isPvE = false;
int global_botDifficulty = 2;
int global_winner = 0;
extern osMessageQueueId_t Queue1Handle;


/*
Nó tương tự như khai báo biến static:
static const int COUNTDOWN_START_FRAMES = 180;
Mục đích:
- Không cho file khác truy cập.
- Tránh trùng tên biến.
- Giữ các hằng số chỉ phục vụ cho Screen1View.cpp.
*/

namespace
{
    const int COUNTDOWN_START_FRAMES = 3 * 60;
    const int COUNTDOWN_FIGHT_FRAMES = 15;
}

Screen1View::Screen1View()
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    stickManA1.initialize();
    stickManB1.initialize();

    botMoveTimer = 0;
    botActionTimer = 0;
    playerBlockTimer = 0;
    botActionStep = 0;

    // =========================================================
    // CÔNG TẮC CHẾ ĐỘ CHƠI:
    // false = PvP (2 Joystick), true = PvE (Đánh với Máy)
    // =========================================================
    isPvE = global_isPvE;
    botDifficulty = global_botDifficulty;// độ khó

    countdownTimer = COUNTDOWN_START_FRAMES; // Bắt đầu đếm từ COUNTDOWN_START_FRAMES.
    fightTimer = 0; // Chưa hiện "FIGHT".
    inputBlocked = true; // Khóa điều khiển.

    countdownLabel.setTypedText(touchgfx::TypedText(T___SINGLEUSE_ZSL3)); // Hiện số 3 ban đầu
    countdownLabel.setVisible(true);
    countdownLabel.invalidate();
}

void Screen1View::handleKeyEvent(uint8_t key)
{
    if (inputBlocked) { // nếu đang chặn phím thì thoát ngay
        return;
    }

    Screen1ViewBase::handleKeyEvent(key);
}

// xử lý mỗi frame
void Screen1View::handleTickEvent()
{
    Screen1ViewBase::handleTickEvent(); // TouchGFX xử lý các hoạt động mặc định.
    // Nếu input đang khoá, chạy cooldown
    if (inputBlocked)
    {
        uint8_t ignoredCommand = 0; // những comman bỏ qua
        while (osMessageQueueGetCount(Queue1Handle) > 0)
        {
            osMessageQueueGet(Queue1Handle, &ignoredCommand, NULL, 0); // đọc ra khỏi queue
        }

        if (countdownTimer > 0)
        {
            if (countdownTimer > 120) {
                countdownLabel.setTypedText(touchgfx::TypedText(T___SINGLEUSE_ZSL3)); // "3"
            } else if (countdownTimer > 60) {
                countdownLabel.setTypedText(touchgfx::TypedText(T___SINGLEUSE_Q67O)); // "2"
            } else {
                countdownLabel.setTypedText(touchgfx::TypedText(T___SINGLEUSE_COUNTDOWN_1)); // "1"
            }
            countdownLabel.invalidate();

            countdownTimer--;
            if (countdownTimer == 0) {
                fightTimer = COUNTDOWN_FIGHT_FRAMES;
                countdownLabel.setTypedText(touchgfx::TypedText(T___SINGLEUSE_COUNTDOWN_FIGHT)); // "FIGHT!"
                countdownLabel.invalidate();
            }

            invalidate();
            return;
        }

        if (fightTimer > 0)
        {
            fightTimer--;
            if (fightTimer == 0) {
                countdownLabel.setVisible(false); // ẩn countdownLabel
                countdownLabel.invalidate(); // vẽ lại
                inputBlocked = false; // mở khoá input
            }

            invalidate();
            return;
        }
    }

    static uint32_t fake_random_counter = 0;
    fake_random_counter++;

    // 1. ĐỌC TÍN HIỆU TỪ HÀNG ĐỢI
    uint8_t res = 0;
    while (osMessageQueueGetCount(Queue1Handle) > 0)
    {
        osMessageQueueGet(Queue1Handle, &res, NULL, 0);

        if (res >= 'A' && res <= 'Z') {
            stickManA1.processCommand(res);
        }
        else if (res >= 'a' && res <= 'z') {
            if (!isPvE) {
                stickManB1.processCommand(res - 32); // Trừ 32 để biến 'a' thành 'A'
            }
        }
    }

    // 2. CHẠY HOẠT ẢNH CHO 2 NHÂN VẬT
    stickManA1.tickProcess();
    stickManB1.tickProcess();

    // 3. LẤY THÔNG SỐ ĐỂ LÀM TRỌNG TÀI
    int posA = stickManA1.getX();
    int posB = stickManB1.getX();
    int distance = abs(posB - posA); // Dùng abs() để lỡ có nhảy xuyên qua nhau vẫn tính đúng

    CharState stateA = stickManA1.getCurrentState();
    CharState stateB = stickManB1.getCurrentState();

    bool crouchA = stickManA1.getIsCrouching();
    bool jumpA = stickManA1.getIsJumping();
    bool crouchB = stickManB1.getIsCrouching();
    bool jumpB = stickManB1.getIsJumping();

    // =========================================================================
        // 4. LOGIC AI (BOT) PVE
        // =========================================================================

        if (isPvE && stateB != STATE_DEAD && stateB != STATE_STUNNED)
        {
            // ---------------------------------------------------------------------
            // 4.1 XỬ LÝ RIÊNG KHI BOT ĐANG GỒNG ULTIMATE
            // ---------------------------------------------------------------------
            if (stateB == STATE_ULTIMATE_CHARGE) {
                botActionTimer++;
                // Vận nội công đủ 20 frame cho tay to ra rồi mới đâm
                if (botActionTimer >= 20) {
                    stickManB1.processCommand('U'); // Gửi lệnh nhả nút -> Lao tới!
                    botActionTimer = 0;
                }
            }
            // ---------------------------------------------------------------------
            // 4.2 LOGIC BÌNH THƯỜNG KHI KHÔNG GỒNG CHIÊU
            // ---------------------------------------------------------------------
            else {
                if (stateA == STATE_BLOCKING) playerBlockTimer++;
                else playerBlockTimer = 0;

                botMoveTimer++;
                botActionTimer++;

                // THIẾT LẬP THÔNG SỐ DỰA TRÊN ĐỘ KHÓ
                int actionThreshold = 50;       // Độ trễ ra đòn
                int guardBreakThreshold = 15;   // Khả năng nhẫn nhịn khi thấy bạn thủ

                if (botDifficulty == 1) {        // MỨC DỄ: Đánh chậm, hiền lành
                    actionThreshold = 80;
                    guardBreakThreshold = 25;
                } else if (botDifficulty == 3) { // MỨC KHÓ: Nhanh, khát máu, ép góc
                    actionThreshold = 30;
                    guardBreakThreshold = 8;
                }

                // Di chuyển tiếp cận
                if (botMoveTimer >= 30) {
                    if (distance > 110) {
                        stickManB1.processCommand('L'); // Lao tới
                    } else {
                        if (fake_random_counter % 5 == 0) stickManB1.processCommand('R'); // Lùi nhử
                        else stickManB1.processCommand('S');
                    }
                    botMoveTimer = 0;
                }
                if (botMoveTimer == 15) stickManB1.processCommand('S');

                // Tấn công khi vào tầm ngắm
                if (distance <= 110 && botActionTimer >= actionThreshold && stickManB1.getStamina() > 20)
                {
                    stickManB1.processCommand('S'); // Dừng lại để đánh

                    if (playerBlockTimer >= guardBreakThreshold) {
                        stickManB1.processCommand('H'); // Phá đỡ ngay lập tức!
                    } else {
                        // Tư duy dùng Tuyệt chiêu Ultimate (Chỉ áp dụng ở Normal và Hard)
                        // Nếu Thể lực >= 80 và hên xui (tỷ lệ 1/6) thì tung chiêu cuối
                        if (botDifficulty >= 2 && stickManB1.getStamina() >= 80 && (fake_random_counter % 6 == 0)) {
                            stickManB1.processCommand('W'); // Bắt đầu gồng
                            botActionTimer = 0; // Reset đếm giờ để chuyển sang logic 4.1
                        }
                        // Đánh thường
                        else if (distance > 90) {
                            stickManB1.processCommand('K'); // Đá xa
                        } else {
                            // Xúc xắc 6 mặt, mở rộng thêm hành động Cúi Né ('C')
                            int randomMove = fake_random_counter % 6;
                            if (randomMove == 0) stickManB1.processCommand('A'); // Đấm nhẹ
                            else if (randomMove == 1) stickManB1.processCommand('H'); // Đấm mạnh
                            else if (randomMove == 2) stickManB1.processCommand('K'); // Đá
                            else if (randomMove == 3) stickManB1.processCommand('J'); // Nhảy né
                            else if (randomMove == 4) stickManB1.processCommand('B'); // Thủ
                            else stickManB1.processCommand('C'); // Cúi né
                        }
                    }
                    botActionTimer = 0;
                    playerBlockTimer = 0;
                }
            }
        }

    // =========================================================================
        // 5. XÉT VA CHẠM (HITBOX) VÀ TRỪ MÁU
        // =========================================================================

        // --- PHE A TẤN CÔNG PHE B ---
        if (stateA == STATE_ATTACK_L || stateA == STATE_ATTACK_H || stateA == STATE_KICK_L || stateA == STATE_KICK_H || stateA == STATE_ULTIMATE_DASH)
        {
            int hitFrame = (stateA == STATE_ATTACK_H || stateA == STATE_KICK_H) ? 10 : 5;
            int attackRange = 90;
            if (stateA == STATE_KICK_L || stateA == STATE_KICK_H) attackRange = 115;
            if (stateA == STATE_ULTIMATE_DASH) attackRange = 160;

            if (stickManA1.getStateTimer() == hitFrame && distance <= attackRange)
            {
                bool isDodged = false;
                // Cúi né đấm
                if ((stateA == STATE_ATTACK_L || stateA == STATE_ATTACK_H) && crouchB && !jumpB) isDodged = true;
                // Nhảy né đá
                if ((stateA == STATE_KICK_L || stateA == STATE_KICK_H) && jumpB) isDodged = true;

                if (!isDodged) {
                    if (stateB == STATE_BLOCKING && stateA != STATE_ULTIMATE_DASH) {
                        // Đỡ thành công -> Trừ máu chip cực nhỏ, KHÔNG CHOÁNG
                        int chipDmg = (stateA == STATE_ATTACK_H || stateA == STATE_KICK_H) ? 1 : 0;
                        if (chipDmg > 0) stickManB1.takeDamage(chipDmg, false);
                    } else {
                        // TRÚNG ĐÒN
                        int dmg = 2; // Đòn nhẹ (Trừ 2 máu)
                        if (stateA == STATE_ATTACK_H || stateA == STATE_KICK_H) dmg = 5; // Đòn mạnh (Trừ 5 máu)
                        if (stateA == STATE_ULTIMATE_DASH) dmg = 15; // Ultimate (Trừ 15 máu)

                        // Chí mạng: Chỉ cộng thêm 2 sát thương
                        if (fake_random_counter % 5 == 0 && stateA != STATE_ULTIMATE_DASH) dmg += 2;
                        // LOGIC CHOÁNG
                            bool willStun = false;
                            // Chỉ cần KHÔNG CÚI, mọi đòn Đấm và Đá (cả nhẹ lẫn mạnh) đều gây choáng
                            if (!crouchA && (stateA == STATE_ATTACK_L || stateA == STATE_ATTACK_H || stateA == STATE_KICK_L || stateA == STATE_KICK_H)) {
                                willStun = true;
                            }

                            stickManB1.takeDamage(dmg, willStun);
                    }
                }
            }
        }

        // --- PHE B TẤN CÔNG PHE A ---
        if (stateB == STATE_ATTACK_L || stateB == STATE_ATTACK_H || stateB == STATE_KICK_L || stateB == STATE_KICK_H || stateB == STATE_ULTIMATE_DASH)
        {
            int hitFrame = (stateB == STATE_ATTACK_H || stateB == STATE_KICK_H) ? 10 : 5;
            int attackRange = 90;
            if (stateB == STATE_KICK_L || stateB == STATE_KICK_H) attackRange = 115;
            if (stateB == STATE_ULTIMATE_DASH) attackRange = 160;

            if (stickManB1.getStateTimer() == hitFrame && distance <= attackRange)
            {
                bool isDodged = false;
                if ((stateB == STATE_ATTACK_L || stateB == STATE_ATTACK_H) && crouchA && !jumpA) isDodged = true;
                if ((stateB == STATE_KICK_L || stateB == STATE_KICK_H) && jumpA) isDodged = true;

                if (!isDodged) {
                    if (stateA == STATE_BLOCKING && stateB != STATE_ULTIMATE_DASH) {
                        int chipDmg = (stateB == STATE_ATTACK_H || stateB == STATE_KICK_H) ? 1 : 0;
                        if (chipDmg > 0) stickManA1.takeDamage(chipDmg, false);
                    } else {
                        int dmg = 2;
                        if (stateB == STATE_ATTACK_H || stateB == STATE_KICK_H) dmg = 5;
                        if (stateB == STATE_ULTIMATE_DASH) dmg = 15;
                        if (fake_random_counter % 5 == 0 && stateB != STATE_ULTIMATE_DASH) dmg += 2;

                        // LOGIC CHOÁNG
                            bool willStun = false;
                            if (!crouchB && (stateB == STATE_ATTACK_L || stateB == STATE_ATTACK_H || stateB == STATE_KICK_L || stateB == STATE_KICK_H)) {
                                willStun = true;
                            }

                            stickManA1.takeDamage(dmg, willStun);
                    }
                }
            }
        }


    // =========================================================================
        // 6. CẬP NHẬT GIAO DIỆN (UI) MÁU & THỂ LỰC VÀ RENDER
        // =========================================================================

        // Ép kiểu toán học để tính chiều dài thanh máu/stamina (Max là 150 pixel)
        // Phe A
        HealthA.setWidth((stickManA1.getHp() * 150) / 100);
        StaminaA.setWidth((stickManA1.getStamina() * 150) / 100);
        HealthA.invalidate();
        StaminaA.invalidate();

        // Phe B
        HealthB.setWidth((stickManB1.getHp() * 150) / 100);
        StaminaB.setWidth((stickManB1.getStamina() * 150) / 100);
        HealthB.invalidate();
        StaminaB.invalidate();
        // Đặt khối này ngay trước dòng invalidate() cuối cùng
            if (stickManA1.getHp() <= 0) {
                global_winner = 2; // Đỏ thắng
                application().gotoScreen3ScreenNoTransition();
            } else if (stickManB1.getHp() <= 0) {
                global_winner = 1; // Xanh thắng
                application().gotoScreen3ScreenNoTransition();
            }

        // Lệnh vẽ lại Screen1 (bao gồm cả các Stickman bên trong)
        invalidate();
    }

