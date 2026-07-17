#include <gui/containers/StickMan.hpp>
#include <images/BitmapDatabase.hpp>
#include <touchgfx/Color.hpp>

namespace
{
    // Màu cơ bản của từng đội
    const touchgfx::colortype PLAYER_A_BASE_COLOR = touchgfx::Color::getColorFromRGB(0, 15, 255);   // Xanh
    const touchgfx::colortype PLAYER_B_BASE_COLOR = touchgfx::Color::getColorFromRGB(255, 0, 0);     // Đỏ

    // Màu nhấp nháy khi trúng đòn (xen kẽ với trắng)
    const touchgfx::colortype HIT_FLASH_A = touchgfx::Color::getColorFromRGB(255, 0, 0);     // A: đỏ
    const touchgfx::colortype HIT_FLASH_B = touchgfx::Color::getColorFromRGB(255, 165, 0);   // B: cam
    const touchgfx::colortype HIT_FLASH_WHITE = touchgfx::Color::getColorFromRGB(255, 255, 255);
}

StickMan::StickMan()
    : teamId(0), xOffset(0), facingRight(true)
{
}

void StickMan::configure(int teamId, int xOffset, bool facingRight)
{
    this->teamId = teamId;
    this->xOffset = xOffset;
    this->facingRight = facingRight;
}

void StickMan::initialize()
{
    StickManBase::initialize();

    hp = 100;
    stamina = 100;
    currentState = STATE_IDLE;
    stateTimer = 0;
    cooldownTimer = 0;

    // Khởi tạo các cờ tư thế
    moveDir = 0;
    isJumping = false;
    jumpTimer = 0;
    isCrouching = false;
    hitFlashTimer = 0;

    // Tọa độ gốc chung
    armOrigEndY = 30;
    bodyOrigStartY = 40;
    legOrigEndY = 80;

    // Thông số phụ thuộc đội
    if (teamId == 0) {
        baseColor = PLAYER_A_BASE_COLOR;
        flashColor = HIT_FLASH_A;
        leftEdge = -100;
        rightEdge = 180;
        armOrigEndX = 25;   // Tay A đâm sang phải
        legOrigEndX = 25;   // Chân A đá sang phải
    } else {
        baseColor = PLAYER_B_BASE_COLOR;
        flashColor = HIT_FLASH_B;
        leftEdge = -80;
        rightEdge = 200;
        armOrigEndX = 35;   // Tay B đâm sang trái
        legOrigEndX = 40;   // Chân B đá sang trái
    }

    setX(xOffset);
    applyTeamLayout();

    headOrigY = Head.getY();
    containerOrigY = getY();
}

// Đặt tọa độ, kích thước, màu và ảnh sao theo đội. Cần cho khung hình tĩnh
// hiển thị trong lúc đếm ngược (tickProcess() chưa chạy).
void StickMan::applyTeamLayout()
{
    if (teamId == 0) {
        // Player A: nhìn sang phải (bố cục gốc StickManA)
        Head.setPosition(95, 84, 25, 85);
        Body.setPosition(95, 99, 25, 116); Body.setStart(13, 40); Body.setEnd(13, 90);
        Arm1.setPosition(110, 106, 25, 85); Arm1.setStart(5, 40); Arm1.setEnd(23, 50);
        Arm2.setPosition(135, 91, 60, 110); Arm2.setStart(2, 65); Arm2.setEnd(25, 30);
        Leg1.setPosition(110, 126, 45, 120); Leg1.setStart(5, 65); Leg1.setEnd(25, 88);
        Leg2.setPosition(110, 163, 65, 77); Leg2.setStart(25, 53); Leg2.setEnd(25, 80);
        Leg3.setPosition(80, 148, 40, 98); Leg3.setStart(25, 43); Leg3.setEnd(5, 66);
        Leg4.setPosition(40, 120, 65, 120); Leg4.setStart(45, 95); Leg4.setEnd(45, 120);
        Arm3.setPosition(60, 106, 45, 91); Arm3.setStart(5, 40); Arm3.setEnd(40, 40);

        star2.setXY(88, 91);
        star1.setXY(113, 91);
        star2.setBitmap(touchgfx::Bitmap(BITMAP_ICON_THEME_IMAGES_TOGGLE_STAR_BORDER_PURPLE500_15_15_000FFF_SVG_ID));
        star1.setBitmap(touchgfx::Bitmap(BITMAP_ICON_THEME_IMAGES_TOGGLE_STAR_BORDER_PURPLE500_15_15_000FFF_SVG_ID));
    } else {
        // Player B: nhìn sang trái (bố cục gốc StickManB, đối xứng gương)
        Head.setPosition(90, 84, 25, 85);
        Body.setPosition(90, 99, 25, 116); Body.setStart(13, 40); Body.setEnd(13, 90);
        Arm1.setPosition(75, 106, 25, 85); Arm1.setStart(20, 40); Arm1.setEnd(2, 50);
        Arm2.setPosition(15, 91, 60, 115); Arm2.setStart(58, 65); Arm2.setEnd(35, 30);
        Leg1.setPosition(55, 126, 45, 120); Leg1.setStart(40, 65); Leg1.setEnd(20, 88);
        Leg2.setPosition(35, 163, 65, 77); Leg2.setStart(40, 53); Leg2.setEnd(40, 80);
        Leg3.setPosition(90, 148, 40, 98); Leg3.setStart(15, 43); Leg3.setEnd(35, 66);
        Leg4.setPosition(105, 120, 65, 120); Leg4.setStart(20, 95); Leg4.setEnd(20, 122);
        Arm3.setPosition(105, 106, 45, 91); Arm3.setStart(40, 40); Arm3.setEnd(5, 40);

        star2.setXY(83, 92);
        star1.setXY(108, 92);
        star2.setBitmap(touchgfx::Bitmap(BITMAP_ICON_THEME_IMAGES_TOGGLE_STAR_BORDER_PURPLE500_15_15_000FFF_SVG_ID));
        star1.setBitmap(touchgfx::Bitmap(BITMAP_ICON_THEME_IMAGES_TOGGLE_STAR_BORDER_PURPLE500_15_15_000FFF_SVG_ID));
    }

    // Đồng bộ màu cơ bản lên toàn bộ painter (để hiển thị đúng khi đếm ngược)
    HeadPainter.setColor(baseColor);
    BodyPainter.setColor(baseColor);
    Arm1Painter.setColor(baseColor);
    Arm2Painter.setColor(baseColor);
    Arm3Painter.setColor(baseColor);
    Leg1Painter.setColor(baseColor);
    Leg2Painter.setColor(baseColor);
    Leg3Painter.setColor(baseColor);
    Leg4Painter.setColor(baseColor);
}

void StickMan::processCommand(uint8_t cmd)
{
    // Player B nhận lệnh chữ thường từ joystick -> chuẩn hoá về chữ hoa.
    // (Lệnh do bot AI gửi đã là chữ hoa nên không bị ảnh hưởng.)
    if (teamId == 1 && cmd >= 'a' && cmd <= 'z') {
        cmd -= 32;
    }

    if (currentState == STATE_STUNNED || currentState == STATE_DEAD) return;

    switch (cmd)
    {
        // 1. NHÓM ĐIỀU KHIỂN TƯ THẾ & DI CHUYỂN
        case 'C':
            isCrouching = true;
            break;
        case 'J':
            if (!isJumping && stamina >= 20) {
                isJumping = true;
                jumpTimer = 0;
                stamina -= 20;
            }
            break;
        case 'L':
            moveDir = -1;
            break;
        case 'R':
            moveDir = 1;
            break;
        case 'S':
            // Khi nhả Joystick ra giữa, tắt hướng đi và ngừng cúi
            moveDir = 0;
            isCrouching = false;
            // Nếu đang đỡ đòn mà nhả nút đỡ (cũng gửi 'S'), thì bỏ đỡ
            if (currentState == STATE_BLOCKING) currentState = STATE_IDLE;
            break;

        // 2. NHÓM HÀNH ĐỘNG (Ghi đè currentState)
        case 'B':
            if (stamina > 10) currentState = STATE_BLOCKING;
            break;
        case 'A':
            if (cooldownTimer == 0 && stamina >= 10) {
                currentState = STATE_ATTACK_L;
                stamina -= 10; stateTimer = 0; cooldownTimer = 20;
            }
            break;
        case 'H':
            if (cooldownTimer == 0 && stamina >= 30) {
                currentState = STATE_ATTACK_H;
                stamina -= 30; stateTimer = 0; cooldownTimer = 40;
            }
            break;
        case 'K':
            if (cooldownTimer == 0 && stamina >= 15) {
                currentState = STATE_KICK_L;
                stamina -= 15; stateTimer = 0; cooldownTimer = 25;
            }
            break;
        case 'X':
            if (cooldownTimer == 0 && stamina >= 40) {
                currentState = STATE_KICK_H;
                stamina -= 40; stateTimer = 0; cooldownTimer = 50;
            }
            break;
        // Nhận mã 'W' khi bắt đầu giữ nút (Bắt đầu tụ lực)
        case 'W':
            if (cooldownTimer == 0 && stamina >= 80) {
                currentState = STATE_ULTIMATE_CHARGE;
                stamina -= 80; // Trừ thể lực ngay lúc bắt đầu
                stateTimer = 0;
            }
            break;
        // Nhận mã 'U' khi nhả nút (Bắt đầu lao đi đâm)
        case 'U':
            // Chỉ lao đi khi đang trong trạng thái tụ lực
            if (currentState == STATE_ULTIMATE_CHARGE) {
                currentState = STATE_ULTIMATE_DASH;
                stateTimer = 0;      // Đặt lại timer để đếm 20 frames lướt
                cooldownTimer = 120; // Bắt đầu tính thời gian hồi chiêu
            }
            break;
    }
}

void StickMan::tickProcess()
{
    stateTimer++;
    if (cooldownTimer > 0) cooldownTimer--;

    // Hướng đánh/lướt: A sang phải (+1), B sang trái (-1)
    const int dir = facingRight ? 1 : -1;

    const bool hitFlashActive = hitFlashTimer > 0;
    const touchgfx::colortype activeColor = hitFlashActive
        ? ((hitFlashTimer % 2 == 0) ? flashColor : HIT_FLASH_WHITE)
        : baseColor;

    HeadPainter.setColor(activeColor);
    BodyPainter.setColor(activeColor);
    Arm1Painter.setColor(activeColor);
    Arm2Painter.setColor(activeColor);
    Arm3Painter.setColor(activeColor);
    Leg1Painter.setColor(activeColor);
    Leg2Painter.setColor(activeColor);
    Leg3Painter.setColor(activeColor);
    Leg4Painter.setColor(activeColor);

    // 1. QUẢN LÝ THỂ LỰC
    if (currentState == STATE_BLOCKING) {
        if (stateTimer % 5 == 0) stamina--;
        if (stamina <= 0) currentState = STATE_IDLE;
    }
    else if (currentState == STATE_IDLE && moveDir == 0) {
        if (stamina < 100 && stateTimer % 3 == 0) stamina++;
    }

    // XÓA BÓNG ĐỒ HỌA
    Arm2.invalidate();
    Leg2.invalidate();
    Head.invalidate();
    Body.invalidate();

    // ==========================================================
    // KHỐI 1: TƯ THẾ (Chạy hoàn toàn độc lập với các đòn đánh)
    // ==========================================================

    // Di chuyển ngang (Cho phép lướt khi đang nhảy)
    if (moveDir != 0) {
        int newX = getX() + (moveDir * 3);
        if (newX < leftEdge) newX = leftEdge;
        if (newX > rightEdge) newX = rightEdge;
        setX(newX);
    }

    // Nhảy (Không bao giờ kẹt trên trời)
    if (isJumping) {
        jumpTimer++;
        if (jumpTimer <= 15) {
            setY(getY() - 6);
        } else if (jumpTimer <= 30) {
            setY(getY() + 6);
        } else {
            setY(containerOrigY);
            isJumping = false;
        }
    }

    // ==========================================================
    // 4. HOẠT ẢNH CÚI NGƯỜI (Hạ trọng tâm ĐỒNG BỘ TOÀN THÂN)
    // ==========================================================
    if (isCrouching && !isJumping) {
        // Hạ thân trước (chung cho cả hai đội)
        Head.setY(headOrigY + 20);
        Body.setStart(13, bodyOrigStartY + 20);
        Body.setEnd(13, 110);
        if (teamId == 0) {
            Arm2.setStart(2, 85);
            Arm1.setStart(5, 60); Arm1.setEnd(23, 70);
            Arm3.setStart(5, 60); Arm3.setEnd(40, 60);
            Leg3.setStart(25, 63); Leg3.setEnd(5, 86);
            Leg4.setStart(45, 115); Leg4.setEnd(45, 120);
        } else {
            Arm2.setStart(58, 85);
            Arm1.setStart(20, 60); Arm1.setEnd(2, 70);
            Arm3.setStart(40, 60); Arm3.setEnd(5, 60);
            Leg3.setStart(15, 63); Leg3.setEnd(35, 86);
            Leg4.setStart(20, 115); Leg4.setEnd(20, 122);
        }
    } else {
        // Đứng thẳng, phục hồi thân trước
        Head.setY(headOrigY);
        Body.setStart(13, bodyOrigStartY);
        Body.setEnd(13, 90);
        if (teamId == 0) {
            Arm2.setStart(2, 65);
            Arm1.setStart(5, 40); Arm1.setEnd(23, 50);
            Arm3.setStart(5, 40); Arm3.setEnd(40, 40);
            Leg3.setStart(25, 43); Leg3.setEnd(5, 66);
            Leg4.setStart(45, 95); Leg4.setEnd(45, 120);
        } else {
            Arm2.setStart(58, 65);
            Arm1.setStart(20, 40); Arm1.setEnd(2, 50);
            Arm3.setStart(40, 40); Arm3.setEnd(5, 40);
            Leg3.setStart(15, 43); Leg3.setEnd(35, 66);
            Leg4.setStart(20, 95); Leg4.setEnd(20, 122);
        }
    }

    // ==========================================================
    // KHỐI 2: HÀNH ĐỘNG (Đấm, Đá, Đỡ, Choáng)
    // ==========================================================

    // HOẠT ẢNH TAY
    if (currentState == STATE_ATTACK_L || currentState == STATE_ATTACK_H) {
        uint32_t duration = (currentState == STATE_ATTACK_H) ? 20 : 10;
        Arm2.setEnd(armOrigEndX + dir * 35, armOrigEndY + 20);
        if (stateTimer > duration) currentState = STATE_IDLE;
    }
    // HOẠT ẢNH ULTIMATE: TỤ LỰC VÀ LƯỚT
    else if (currentState == STATE_ULTIMATE_CHARGE) {
        // Chĩa thẳng cánh tay (kiếm) về phía trước để tụ lực
        Arm2.setEnd(armOrigEndX + dir * 50, armOrigEndY);

        // Càng giữ lâu tay càng to (Max là độ dày 15)
        int currentWidth = 3 + (stateTimer / 5);
        if (currentWidth > 15) currentWidth = 15;
        Arm2.setLineWidth(currentWidth);
    }
    else if (currentState == STATE_ULTIMATE_DASH) {
        // Giữ nguyên tư thế đâm thẳng và tay to tối đa khi lướt
        Arm2.setEnd(armOrigEndX + dir * 50, armOrigEndY);
        Arm2.setLineWidth(15);

        // Logic lướt đúng 100 pixel (Mỗi frame đi 5px, mất 20 frames)
        if (stateTimer <= 20) {
            int newX = getX() + dir * 5;
            // bọc bằng tường tàng hình để tránh lướt xuyên màn hình
            if (facingRight) { if (newX > rightEdge) newX = rightEdge; }
            else             { if (newX < leftEdge)  newX = leftEdge; }
            setX(newX);
        } else {
            // Lướt xong thì thu chiêu
            Arm2.setLineWidth(3);
            currentState = STATE_IDLE;
        }
    }
    else if (currentState == STATE_BLOCKING) {
        // Đưa kiếm sát vào thân để đỡ
        Arm2.setEnd(facingRight ? 2 : 58, 25);
    }
    else if (currentState == STATE_STUNNED) {
        Arm2.setLineWidth(3); // Cứ choáng là tay thu nhỏ lại
        Arm2.setEnd(armOrigEndX, armOrigEndY + 40);
        star1.setVisible(true);
        star2.setVisible(true);
        if (stateTimer > 20) {
            star1.setVisible(false);
            star2.setVisible(false);
            currentState = STATE_IDLE;
        }
    }
    else {
        Arm2.setLineWidth(3);
        // Nếu đang cúi thì nắm đấm (End) cũng phải hạ theo vai (Start)
        if (isCrouching && !isJumping) {
            Arm2.setEnd(armOrigEndX, armOrigEndY + 20);
        } else {
            Arm2.setEnd(armOrigEndX, armOrigEndY); // Tay bình thường
        }
    }

    // ==========================================================
    // 6. HOẠT ẢNH CHÂN (ĐÁ THẲNG - LIÊN KẾT ĐÙI VÀ CẲNG CHÂN)
    // ==========================================================
    if (currentState == STATE_KICK_L || currentState == STATE_KICK_H) {
        uint32_t duration = (currentState == STATE_KICK_H) ? 25 : 12;
        // Nếu đang cúi, toàn bộ đòn đá phải hạ thấp xuống 20 pixel
        int kickOffsetY = (isCrouching && !isJumping) ? 20 : 0;

        if (teamId == 0) {
            // Đùi (Leg1) nâng lên song song mặt đất, đá sang phải
            Leg1.setStart(5, 65 + kickOffsetY);
            Leg1.setEnd(45, 65 + kickOffsetY);
            // Cẳng chân (Leg2) nối vào đầu gối và đạp thẳng sang phải
            Leg2.setStart(45, 30 + kickOffsetY);
            Leg2.setEnd(85, 30 + kickOffsetY);
        } else {
            // Đùi B đá thẳng sang trái (từ X=40 vươn ra xa về X=0)
            Leg1.setStart(40, 65 + kickOffsetY);
            Leg1.setEnd(0, 65 + kickOffsetY);
            // Cẳng chân B nối tiếp, lao tiếp sang trái (từ X=20 về X=-20)
            Leg2.setStart(20, 30 + kickOffsetY);
            Leg2.setEnd(-20, 30 + kickOffsetY);
        }

        if (stateTimer > duration) currentState = STATE_IDLE;
    } else {
        // Phục hồi chân về trạng thái Cúi hoặc Đứng bình thường
        if (isCrouching && !isJumping) {
            if (teamId == 0) {
                Leg1.setStart(5, 85);
                Leg1.setEnd(35, 105);
                Leg2.setStart(35, 70);
            } else {
                Leg1.setStart(40, 85);
                Leg1.setEnd(10, 105);
                Leg2.setStart(30, 70);
            }
        } else {
            if (teamId == 0) {
                Leg1.setStart(5, 65);
                Leg1.setEnd(25, 88);
                Leg2.setStart(25, 53);
            } else {
                Leg1.setStart(40, 65);
                Leg1.setEnd(20, 88);
                Leg2.setStart(40, 53);
            }
        }
        Leg2.setEnd(legOrigEndX, legOrigEndY);
    }

    // 7. VẼ LẠI HÌNH DÁNG (Đồng bộ tất cả các line)
    Arm1.invalidate();
    Arm2.invalidate();
    Arm3.invalidate();
    Leg1.invalidate();
    Leg2.invalidate();
    Leg3.invalidate();
    Leg4.invalidate();
    Head.invalidate();
    Body.invalidate();

    if (hitFlashTimer > 0) hitFlashTimer--;
}

void StickMan::takeDamage(int dmg, bool causeStun)
{
    hitFlashTimer = 6;
    hp -= dmg;
    if (hp <= 0) {
        hp = 0;
        currentState = STATE_DEAD;
    } else {
        // Chỉ bị choáng nếu đòn đánh trúng đầu
        if (causeStun) {
            currentState = STATE_STUNNED;
            stateTimer = 0;
        }
    }
}
