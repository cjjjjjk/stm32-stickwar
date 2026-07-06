# TASKS — StickWar STM32

Bảng tổng quan 4 task nâng cấp sau khi hoàn thiện core game.  
Các nâng cấp mục tiêu chính đáp ứng các yêu cầu:
- Yêu cầu mô tả project
- Phi chức năng: Tái sử dụng mã nguồn  
- Cải thiện gameplay
- Cải thiện trải nghiệm game

| # | Task | Loại | Độ khó | 
|---|------|-------|--------|
| T1 | B1 Button Mode Selection | Hardware / Firmware | Thấp |
| T2 | Refactor StickManA/B → class chung | Kiến trúc phần mềm | Trung bình |
| T3 | Hệ thống Round — Best of 3 | Tính năng game | Trung bình | 
| T4 | Visual Feedback: Countdown + Hit Flash | Giao diện / UX | Thấp |

---

## T1 — B1 Button Mode Selection

### Mô tả

Yêu cầu đề bài quy định dùng nút **B1** (nút user xanh trên STM32F429-Discovery, chân **PC13**) để chọn chế độ chơi. Hiện tại chế độ chơi được chọn hoàn toàn qua **touchscreen** ở Screen2. Task này chuyển luồng điều khiển về nút vật lý, đáp ứng đúng spec phần cứng.

### Yêu cầu chi tiết

- **Nhấn đơn (< 500 ms):** Cycle tuần tự qua các chế độ:
  `PvP → PvE Easy → PvE Medium → PvE Hard → PvP → ...`
- **Nhấn giữ (≥ 1000 ms):** Xác nhận chế độ hiện tại và bắt đầu game.
- Chế độ đang chọn được hiển thị trực quan trên Screen2 (highlight button tương ứng).
- Cần debounce tối thiểu 20 ms để loại nhiễu cơ học.

### Phạm vi thực hiện

**Firmware (`Core/Src/main.c` — `StartDefaultTask`):**
- Thêm polling `HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)` trong vòng lặp chính.
- Theo dõi thời điểm nhấn xuống bằng `osKernelGetTickCount()`.
- Tại thời điểm nhả tay: tính `hold_time`, phân nhánh nhấn đơn / nhấn giữ.
- Gửi lệnh chọn mode qua một **queue riêng** (hoặc biến `global_pendingMode`) để TouchGFX đọc.

**GUI (`TouchGFX/gui/src/screen2_screen/Screen2View.cpp`):**
- `handleTickEvent()` đọc `global_pendingMode` và cập nhật highlight.
- Khi nhận lệnh xác nhận, gọi `application().gotoScreen1ScreenNoTransition()`.
- Touch button giữ nguyên như fallback (không xóa).

**Không ảnh hưởng:** `Screen1View`, `StickManA/B`, toàn bộ game logic.

---

## T2 — Refactor StickManA/B → Class chung `StickMan`

### Mô tả

`StickManA.cpp` (325 dòng) và `StickManB.cpp` (290 dòng) là hai file gần như **đồng nhất**, chỉ khác nhau ở offset tọa độ và cách xử lý lệnh chữ hoa/thường. Việc duy trì song song hai file tạo rủi ro khi fix bug (phải sửa ở cả hai), vi phạm nguyên tắc DRY. Task này hợp nhất thành một class tham số hoá, cải thiện khả năng tái sử dụng và bảo trì.

### Yêu cầu chi tiết

- Class mới `StickMan` nhận các tham số cấu hình tại constructor:
  - `int teamId` — 0 = Player A (lệnh uppercase), 1 = Player B (lệnh lowercase)
  - `int xOffset` — vị trí X ban đầu
  - `bool facingRight` — hướng nhìn ban đầu
- Toàn bộ logic `processCommand()`, `update()`, `draw()` gộp vào một implementation.
- Xử lý lowercase → uppercase bên trong `processCommand()` dựa trên `teamId`.
- `Screen1View` tạo hai instance: `StickMan playerA(0, ...)` và `StickMan playerB(1, ...)`.

### Phạm vi thực hiện

**Tạo mới:**
- `TouchGFX/gui/include/gui/containers/StickMan.hpp`
- `TouchGFX/gui/src/containers/StickMan.cpp`

**Xóa:**
- `StickManA.hpp / StickManA.cpp`
- `StickManB.hpp / StickManB.cpp`

**Cập nhật:**
- `Screen1View.hpp` — đổi khai báo từ `StickManA stickManA1` / `StickManB stickManB1` thành `StickMan playerA` / `StickMan playerB`.
- `Screen1View.cpp` — cập nhật tên biến, truyền tham số constructor.
- `CMakeLists.txt` hoặc `.ioc` generated Makefile — thêm `StickMan.cpp`, xóa hai file cũ.

**Không ảnh hưởng:** AI bot logic, collision detection, Screen2, Screen3, firmware.

---

## T3 — Hệ thống Round (Best of 3)

### Mô tả

Hiện tại mỗi ván chơi là một trận đơn lẻ, không có tracking thắng thua xuyên suốt. Sau khi một bên hết HP, game chuyển sang màn hình kết quả và cho phép "Play Again" từ đầu. Task này thêm **hệ thống round** (Best of 3), làm cho demo có chiều sâu và thuyết phục hơn khi báo cáo.

### Yêu cầu chi tiết

- Hiển thị **điểm thắng tích lũy** ở Screen1: `P1: X | P2: X` (tối đa 2 wins mỗi bên).
- Khi một bên thắng round: tăng counter, hiển thị màn hình kết quả round ngắn (Screen3 hiện tại).
  - Nếu chưa ai đạt 2 wins: nút "Next Round" reset HP/stamina, giữ nguyên win counter.
  - Nếu một bên đạt 2 wins: hiển thị "CHAMPION" và cho về menu (Screen2).
- Win counter reset về 0 khi quay về Screen2.

### Phạm vi thực hiện

**Biến global (thêm vào `Screen1View.cpp` hoặc file global riêng):**
```cpp
int global_winsA = 0;
int global_winsB = 0;
int global_currentRound = 1;
```

**`Screen1View.cpp`:**
- Thêm TextArea hiển thị `P1: X | P2: X` và `Round N` ở trên cùng màn hình.
- Trong `handleTickEvent()`: sau khi phát hiện `hp <= 0`, cập nhật `global_winsA/B` trước khi chuyển screen.

**`Screen3View.cpp`:**
- Đọc `global_winsA`, `global_winsB`, `global_currentRound` để hiển thị ngữ cảnh.
- Phân nhánh: "Next Round" (nếu chưa có champion) hoặc "Back to Menu" (nếu có champion).
- Nút "Next Round": tăng `global_currentRound`, reset `global_winner = 0`, gotoScreen1.

**`Screen2View.cpp`:**
- Trong `setupScreen()`: reset `global_winsA = global_winsB = global_currentRound = 1 = 0`.

**Không ảnh hưởng:** Toàn bộ firmware, StickMan logic, AI bot.

---

## T4 — Visual Feedback: Countdown + Hit Flash

### Mô tả

Hai cải tiến nhỏ về giao diện giúp người xem hiểu ngay trạng thái game khi demo mà không cần giải thích:

1. **Countdown 3-2-1:** Đếm ngược trước khi round bắt đầu, tránh tình huống nhân vật đứng im lúng túng ngay khi vào game.
2. **Hit Flash:** Nhân vật bị đánh trúng sẽ chớp màu đỏ/trắng trong vài frame — phản hồi trực quan rõ ràng hơn thanh HP thu nhỏ từ từ.

### Yêu cầu chi tiết

**Countdown:**
- Khi Screen1 khởi tạo (`setupScreen()`), set `countdownTimer = 3 * FPS` (khoảng 180 frames ở 60fps).
- Trong thời gian đếm ngược: **block toàn bộ input** từ queue (đọc nhưng bỏ qua), nhân vật ở `STATE_IDLE`.
- Hiển thị số "3" → "2" → "1" → "FIGHT!" bằng TextArea lớn ở trung tâm màn hình.
- Sau khi hết đếm: ẩn TextArea, cho phép input bình thường.

**Hit Flash:**
- Thêm `int hitFlashTimer = 0` vào class `StickMan` (hoặc `StickManA/B` hiện tại).
- Khi nhận damage (`applyDamage()` được gọi): set `hitFlashTimer = 6` (6 frames).
- Trong `draw()`: nếu `hitFlashTimer > 0` và `hitFlashTimer % 2 == 0`, vẽ tất cả line segment bằng màu đỏ/trắng thay vì màu gốc; giảm `hitFlashTimer--` mỗi frame.

### Phạm vi thực hiện

**`Screen1View.hpp / .cpp`:**
- Thêm `int countdownTimer`, `bool inputBlocked`.
- Thêm TextArea `countdownLabel` trong TouchGFX Designer hoặc tạo động bằng `Unicode::snprintf`.
- Logic countdown trong `handleTickEvent()` trước khi đọc queue.

**`StickManA.cpp` / `StickManB.cpp` (hoặc `StickMan.cpp` nếu T2 đã xong):**
- Thêm field `hitFlashTimer` và logic màu trong `drawStickman()`.

**TouchGFX Designer:**
- Thêm 1 TextArea `countdownLabel` ở Screen1, căn giữa, font lớn, ban đầu hidden.

**Không ảnh hưởng:** Firmware, AI bot, Screen2, Screen3, collision detection.
