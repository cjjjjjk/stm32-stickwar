#include <gui/containers/StickManA.hpp>

StickManA::StickManA()
{
}

void StickManA::initialize()
{
    StickManABase::initialize();

    hp = 100;
    stamina = 100;
    currentState = STATE_IDLE;
    stateTimer = 0;
    cooldownTimer = 0;

    // Khởi tạo các cờ tư thế mới
    moveDir = 0;
    isJumping = false;
    jumpTimer = 0;
    isCrouching = false;

    // Tọa độ gốc
    armOrigEndX = 25;
    armOrigEndY = 30;
    legOrigEndX = 25;
    legOrigEndY = 80;
    headOrigY = HeadA.getY();
    bodyOrigStartY = 40;
    containerOrigY = getY();
}

void StickManA::processCommand(uint8_t cmd)
{
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
				stateTimer = 0; // Đặt lại timer để đếm 20 frames lướt
				cooldownTimer = 120; // Bắt đầu tính thời gian hồi chiêu
			}
			break;
    }
}

void StickManA::tickProcess()
{
    stateTimer++;
    if (cooldownTimer > 0) cooldownTimer--;

    // 1. QUẢN LÝ THỂ LỰC
    if (currentState == STATE_BLOCKING) {
        if (stateTimer % 5 == 0) stamina--;
        if (stamina <= 0) currentState = STATE_IDLE;
    }
    else if (currentState == STATE_IDLE && moveDir == 0) {
        if (stamina < 100 && stateTimer % 3 == 0) stamina++;
    }

    // XÓA BÓNG ĐỒ HỌA
    Arm2A.invalidate();
    Leg2A.invalidate();
    HeadA.invalidate();
    BodyA.invalidate();

    /// ==========================================================
    // KHỐI 1: TƯ THẾ (Chạy hoàn toàn độc lập với các đòn đánh)
    // ==========================================================

    // Di chuyển ngang (Cho phép lướt khi đang nhảy)
    if (moveDir != 0) {
        int newX = getX() + (moveDir * 3);

        // --- GIỚI HẠN KHÔNG GIAN (Boundary Check) ---

        const int LEFT_EDGE = -100;
        const int RIGHT_EDGE = 180;

        if (newX < LEFT_EDGE) newX = LEFT_EDGE;
        if (newX > RIGHT_EDGE) newX = RIGHT_EDGE;

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
            // Hạ thân trước
            HeadA.setY(headOrigY + 20);
            BodyA.setStart(13, bodyOrigStartY + 20);
            BodyA.setEnd(13, 110);
            Arm2A.setStart(2, 85);

            // Hạ 2 cánh tay phía sau
            Arm1A.setStart(5, 60); Arm1A.setEnd(23, 70); // Gốc: 40->60, 50->70
            Arm3A.setStart(5, 60); Arm3A.setEnd(40, 60); // Gốc: 40->60, 40->60

            // Chùng gối chân trụ phía sau (Bàn chân giữ nguyên, gối hạ thấp)
            Leg3A.setStart(25, 63); Leg3A.setEnd(5, 86);     // Gốc: 43->63, 66->86
            Leg4A.setStart(45, 115); Leg4A.setEnd(45, 120);  // Nối vào gối mới, bàn chân chạm đất
        } else {
            // Đứng thẳng, phục hồi thân trước
            HeadA.setY(headOrigY);
            BodyA.setStart(13, bodyOrigStartY);
            BodyA.setEnd(13, 90);
            Arm2A.setStart(2, 65);

            // Phục hồi cánh tay sau
            Arm1A.setStart(5, 40); Arm1A.setEnd(23, 50);
            Arm3A.setStart(5, 40); Arm3A.setEnd(40, 40);

            // Phục hồi chân trụ phía sau
            Leg3A.setStart(25, 43); Leg3A.setEnd(5, 66);
            Leg4A.setStart(45, 95); Leg4A.setEnd(45, 120);
        }

    // ==========================================================
    // KHỐI 2: HÀNH ĐỘNG (Đấm, Đá, Đỡ, Choáng)
    // ==========================================================

    // HOẠT ẢNH TAY
    if (currentState == STATE_ATTACK_L || currentState == STATE_ATTACK_H) {
        uint32_t duration = (currentState == STATE_ATTACK_H) ? 20 : 10;
        Arm2A.setEnd(armOrigEndX + 35, armOrigEndY + 20);
        if (stateTimer > duration) currentState = STATE_IDLE;
    }

    // HOẠT ẢNH ULTIMATE: TỤ LỰC VÀ LƯỚT
    // ==========================================================
    else if (currentState == STATE_ULTIMATE_CHARGE) {
        // Chĩa thẳng cánh tay (kiếm) về phía trước để tụ lực
        Arm2A.setEnd(armOrigEndX + 50, armOrigEndY);

        // Càng giữ lâu tay càng to (Max là độ dày 15)
        int currentWidth = 3 + (stateTimer / 5);
        if (currentWidth > 15) currentWidth = 15;
        Arm2A.setLineWidth(currentWidth);
    }
    else if (currentState == STATE_ULTIMATE_DASH) {
        // Giữ nguyên tư thế đâm thẳng và tay to tối đa khi lướt
        Arm2A.setEnd(armOrigEndX + 50, armOrigEndY);
        Arm2A.setLineWidth(15);

        // Logic lướt đúng 100 pixel (Mỗi frame đi 5px, mất 20 frames)
        if (stateTimer <= 20) {
            int newX = getX() + 5;

            // bọc bằng tường tàng hình để tránh lướt xuyên màn hình
            const int RIGHT_EDGE = 180;
            if (newX > RIGHT_EDGE) newX = RIGHT_EDGE;

            setX(newX);
        } else {
            // Lướt xong thì thu chiêu
            Arm2A.setLineWidth(3);
            currentState = STATE_IDLE;
        }
    }
    else if (currentState == STATE_BLOCKING) {
        Arm2A.setEnd(2, 25);
    }
    else if (currentState == STATE_STUNNED) {
    	Arm2A.setLineWidth(3); // Cứ choáng là tay thu nhỏ lại
        Arm2A.setEnd(armOrigEndX, armOrigEndY + 40);
        star1.setVisible(true);
		star2.setVisible(true);
        if (stateTimer > 20) {
        	star1.setVisible(false);
			star2.setVisible(false);
        	currentState = STATE_IDLE;

        }
    }
    else {
            // Nếu đang cúi thì nắm đấm (End) cũng phải hạ theo vai (Start)
            if (isCrouching && !isJumping) {
                Arm2A.setEnd(armOrigEndX, armOrigEndY + 20);
            } else {
                Arm2A.setEnd(armOrigEndX, armOrigEndY); // Tay bình thường
            }
        }

    // ==========================================================
        // 6. HOẠT ẢNH CHÂN (ĐÁ THẲNG - LIÊN KẾT ĐÙI VÀ CẲNG CHÂN)
        // ==========================================================
        if (currentState == STATE_KICK_L || currentState == STATE_KICK_H) {
            uint32_t duration = (currentState == STATE_KICK_H) ? 25 : 12;

            //  Nếu đang cúi, toàn bộ đòn đá phải hạ thấp xuống 20 pixel
            int kickOffsetY = (isCrouching && !isJumping) ? 20 : 0;

            // Đùi (Leg1A) nâng lên song song mặt đất, dời xuống theo kickOffsetY
            Leg1A.setStart(5, 65 + kickOffsetY);
            Leg1A.setEnd(45, 65 + kickOffsetY);

            // Cẳng chân (Leg2A) nối vào đầu gối và đạp thẳng, cũng dời xuống tương ứng
            Leg2A.setStart(45, 30 + kickOffsetY);
            Leg2A.setEnd(85, 30 + kickOffsetY);

            if (stateTimer > duration) currentState = STATE_IDLE;
        } else {
            // Phục hồi chân về trạng thái Cúi hoặc Đứng bình thường
            if (isCrouching && !isJumping) {
                // Đang cúi: Đùi hạ xuống, đầu gối đẩy ra trước
                Leg1A.setStart(5, 85);
                Leg1A.setEnd(35, 105);

                // Cẳng chân nối vào khớp gối
                Leg2A.setStart(35, 70);
            } else {
                // Đứng thẳng bình thường (Trả về thông số gốc)
                Leg1A.setStart(5, 65);
                Leg1A.setEnd(25, 88);
                Leg2A.setStart(25, 53);
            }
            Leg2A.setEnd(legOrigEndX, legOrigEndY);
        }

	// 7. VẼ LẠI HÌNH DÁNG (Đồng bộ tất cả các line)
		Arm1A.invalidate();
		Arm2A.invalidate();
		Arm3A.invalidate();
		Leg1A.invalidate();
		Leg2A.invalidate();
		Leg3A.invalidate();
		Leg4A.invalidate();
		HeadA.invalidate();
		BodyA.invalidate();
}
void StickManA::takeDamage(int dmg, bool causeStun)
{
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

