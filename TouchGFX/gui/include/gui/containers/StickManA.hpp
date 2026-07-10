#ifndef STICKMANA_HPP
#define STICKMANA_HPP

#include <gui_generated/containers/StickManABase.hpp>

typedef enum {
    STATE_IDLE,
    STATE_WALKING,
    STATE_CROUCHING,
    STATE_BLOCKING,
    STATE_ATTACK_L,   // Đánh nhẹ
    STATE_ATTACK_H,   // Đánh mạnh
    STATE_STUNNED,    // Bị choáng
    STATE_DEAD,
	STATE_JUMPING,
	STATE_ULTIMATE_CHARGE,
	STATE_ULTIMATE_DASH,
	STATE_KICK_L,
    STATE_KICK_H,
} CharState;

class StickManA : public StickManABase
{
public:
    StickManA();
    virtual ~StickManA() {}

    virtual void initialize();

    void processCommand(uint8_t cmd); // Nhận phím
    void tickProcess();               // Cập nhật mỗi khung hình
    void takeDamage(int dmg, bool causeStun);         // Nhận sát thương
    // Các hàm Getters để Screen1 lấy thông tin tính va chạm
	bool getIsCrouching() { return isCrouching; }
	bool getIsJumping() { return isJumping; }
	int getHp() { return hp; }
	int getStamina() { return stamina; }


    CharState getCurrentState() { return currentState; }
    uint32_t getStateTimer() { return stateTimer; }

protected:
    // Chỉ số sinh tồn
    int hp;
    int stamina;
    CharState currentState;

    // Bộ đếm thời gian
    uint32_t stateTimer;
    uint32_t cooldownTimer;

    // Lưu tọa độ gốc của Cánh tay và Thân để reset hoạt ảnh
    int armOrigEndX;
    int armOrigEndY;
    int bodyOrigStartY;
    int moveDir;       // Hướng đang di chuyển (-1, 0, 1)
    int headOrigY;     // Lưu vị trí gốc của đầu
    int containerOrigY, legOrigEndX, legOrigEndY;
    bool isJumping;
	uint32_t jumpTimer;
	bool isCrouching;
    int hitFlashTimer;

};

#endif // STICKMANA_HPP
