#ifndef STICKMAN_HPP
#define STICKMAN_HPP

#include <gui_generated/containers/StickManBase.hpp>

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

/*
 * StickMan hợp nhất StickManA và StickManB thành một class tham số hoá.
 * Toàn bộ logic điều khiển, trạng thái và hoạt ảnh dùng chung; chỉ có
 * tọa độ (đối xứng gương) và màu sắc là khác nhau theo teamId.
 */
class StickMan : public StickManBase
{
public:
    StickMan();
    virtual ~StickMan() {}

    // Cấu hình đội trước khi gọi initialize() (gọi trong Screen1View::setupScreen).
    //   teamId      : 0 = Player A (lệnh chữ hoa), 1 = Player B (lệnh chữ thường)
    //   xOffset     : vị trí X ban đầu của container
    //   facingRight : true = nhìn/đánh sang phải (A), false = sang trái (B)
    void configure(int teamId, int xOffset, bool facingRight);

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
    // Cấu hình đội
    int teamId;
    int xOffset;
    bool facingRight;

    // Màu sắc & biên di chuyển phụ thuộc đội
    touchgfx::colortype baseColor;
    touchgfx::colortype flashColor; // màu nhấp nháy khi trúng đòn (xen kẽ với trắng)
    int leftEdge;
    int rightEdge;

    // Áp dụng bố cục (tọa độ + màu + sao) theo đội, dùng cho khung hình tĩnh
    // trước khi tickProcess() chạy (trong lúc đếm ngược).
    void applyTeamLayout();

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

#endif // STICKMAN_HPP
