#include <gui/containers/StickManB.hpp>

StickManB::StickManB()
{
}

void StickManB::initialize()
{
    StickManBBase::initialize();

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

    // Tọa độ gốc
    armOrigEndX = 35;  // Tay B đâm ngược về trái
    armOrigEndY = 30;
    legOrigEndX = 40;  // Chân B nằm bên phải, đá sang trái
    legOrigEndY = 80;
    headOrigY = HeadB.getY();
    bodyOrigStartY = 40;
    containerOrigY = getY();
}

void StickManB::processCommand(uint8_t cmd)
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
            moveDir = -1; // Đi sang trái (Tiến về phía A)
            break;
        case 'R':
            moveDir = 1;  // Đi sang phải (Lùi lại)
            break;
        case 'S':
            moveDir = 0;
            isCrouching = false;
            if (currentState == STATE_BLOCKING) currentState = STATE_IDLE;
            break;

        // 2. NHÓM HÀNH ĐỘNG
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
        // Chiêu Cuối
        case 'W':
            if (cooldownTimer == 0 && stamina >= 80) {
                currentState = STATE_ULTIMATE_CHARGE;
                stamina -= 80;
                stateTimer = 0;
            }
            break;
        case 'U':
            if (currentState == STATE_ULTIMATE_CHARGE) {
                currentState = STATE_ULTIMATE_DASH;
                stateTimer = 0;
                cooldownTimer = 120;
            }
            break;
    }
}

void StickManB::tickProcess()
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
    Arm2B.invalidate();
    Leg2B.invalidate();
    HeadB.invalidate();
    BodyB.invalidate();

    // ==========================================================
    // KHỐI 1: TƯ THẾ
    // ==========================================================

    if (moveDir != 0) {
        int newX = getX() + (moveDir * 3);
        const int LEFT_EDGE = -80;
        const int RIGHT_EDGE = 200;

        if (newX < LEFT_EDGE) newX = LEFT_EDGE;
        if (newX > RIGHT_EDGE) newX = RIGHT_EDGE;

        setX(newX);
    }

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

    // HOẠT ẢNH CÚI NGƯỜI CỦA B (Tọa độ X giữ nguyên của B, hạ trục Y thêm 20)
    if (isCrouching && !isJumping) {
        HeadB.setY(headOrigY + 20);
        BodyB.setStart(13, bodyOrigStartY + 20);
        BodyB.setEnd(13, 110);
        Arm2B.setStart(58, 85);

        Arm1B.setStart(20, 60); Arm1B.setEnd(2, 70);
        Arm3B.setStart(40, 60); Arm3B.setEnd(5, 60);

        Leg3B.setStart(15, 63); Leg3B.setEnd(35, 86);
        Leg4B.setStart(20, 115); Leg4B.setEnd(20, 122);
    } else {
        HeadB.setY(headOrigY);
        BodyB.setStart(13, bodyOrigStartY);
        BodyB.setEnd(13, 90);
        Arm2B.setStart(58, 65);

        Arm1B.setStart(20, 40); Arm1B.setEnd(2, 50);
        Arm3B.setStart(40, 40); Arm3B.setEnd(5, 40);

        Leg3B.setStart(15, 43); Leg3B.setEnd(35, 66);
        Leg4B.setStart(20, 95); Leg4B.setEnd(20, 122);
    }

    // ==========================================================
    // KHỐI 2: HÀNH ĐỘNG
    // ==========================================================

    if (currentState == STATE_ATTACK_L || currentState == STATE_ATTACK_H) {
        uint32_t duration = (currentState == STATE_ATTACK_H) ? 20 : 10;
        // Đâm về phía trái: X giảm đi 35
        Arm2B.setEnd(armOrigEndX - 35, armOrigEndY + 20);
        if (stateTimer > duration) currentState = STATE_IDLE;
    }
    else if (currentState == STATE_ULTIMATE_CHARGE) {
        // Tụ lực về phía trái: X giảm đi 50
        Arm2B.setEnd(armOrigEndX - 50, armOrigEndY);
        int currentWidth = 3 + (stateTimer / 5);
        if (currentWidth > 15) currentWidth = 15;
        Arm2B.setLineWidth(currentWidth);
    }
    else if (currentState == STATE_ULTIMATE_DASH) {
        Arm2B.setEnd(armOrigEndX - 50, armOrigEndY);
        Arm2B.setLineWidth(15);

        if (stateTimer <= 20) {
            // Lướt sang trái (trừ đi 5 pixel mỗi frame)
            int newX = getX() - 5;
            const int LEFT_EDGE = -80;
            if (newX < LEFT_EDGE) newX = LEFT_EDGE;
            setX(newX);
        } else {
            Arm2B.setLineWidth(3);
            currentState = STATE_IDLE;
        }
    }
    else if (currentState == STATE_BLOCKING) {
        // Đưa kiếm sát vào thân để đỡ (X của thân cỡ 13, gốc kiếm là 58)
        Arm2B.setEnd(58, 25);
    }
    else if (currentState == STATE_STUNNED) {
        Arm2B.setLineWidth(3);
        Arm2B.setEnd(armOrigEndX, armOrigEndY + 40);
        star1.setVisible(true);
        star2.setVisible(true);
        if (stateTimer > 20) {
        	star1.setVisible(false);
			star2.setVisible(false);
        	currentState = STATE_IDLE;
        }
    }
    else {
        Arm2B.setLineWidth(3);
        if (isCrouching && !isJumping) {
            Arm2B.setEnd(armOrigEndX, armOrigEndY + 20);
        } else {
            Arm2B.setEnd(armOrigEndX, armOrigEndY);
        }
    }

    // HOẠT ẢNH CHÂN B (Đá về phía trái)
    if (currentState == STATE_KICK_L || currentState == STATE_KICK_H) {
        uint32_t duration = (currentState == STATE_KICK_H) ? 25 : 12;
        int kickOffsetY = (isCrouching && !isJumping) ? 20 : 0;

        // Đùi B đá thẳng sang trái (từ X=40 vươn ra xa về X=0)
        Leg1B.setStart(40, 65 + kickOffsetY);
        Leg1B.setEnd(0, 65 + kickOffsetY);

        // Cẳng chân B nối tiếp, lao tiếp sang trái (từ X=20 vươn ra xa về X=-20)
        Leg2B.setStart(20, 30 + kickOffsetY);
        Leg2B.setEnd(-20, 30 + kickOffsetY);

        if (stateTimer > duration) currentState = STATE_IDLE;
    } else {
        if (isCrouching && !isJumping) {
            // Hạ gối B xuống
            Leg1B.setStart(40, 85);
            Leg1B.setEnd(10, 105);
            Leg2B.setStart(30, 70);
        } else {
            Leg1B.setStart(40, 65);
            Leg1B.setEnd(20, 88);
            Leg2B.setStart(40, 53);
        }
        Leg2B.setEnd(legOrigEndX, legOrigEndY);
    }

    // VẼ LẠI HÌNH DÁNG
    Arm1B.invalidate();
    Arm2B.invalidate();
    Arm3B.invalidate();
    Leg1B.invalidate();
    Leg2B.invalidate();
    Leg3B.invalidate();
    Leg4B.invalidate();
    HeadB.invalidate();
    BodyB.invalidate();
}

void StickManB::takeDamage(int dmg, bool causeStun)
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
