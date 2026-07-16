#ifndef STICKMANB_HPP
#define STICKMANB_HPP

#include <gui_generated/containers/StickManBBase.hpp>
#include <gui/containers/StickManA.hpp> // Include để dùng chung enum CharState

class StickManB : public StickManBBase
{
public:
    StickManB();
    virtual ~StickManB() {}

    virtual void initialize();

    void processCommand(uint8_t cmd); // Nhận phím
    void tickProcess();               // Cập nhật mỗi khung hình
    void takeDamage(int dmg, bool causeStun);       // Nhận sát thương
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

#endif // STICKMANB_HPP
