#ifndef SCREEN3VIEW_HPP
#define SCREEN3VIEW_HPP

#include <gui_generated/screen3_screen/Screen3ViewBase.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>

class Screen3View : public Screen3ViewBase
{
public:
    Screen3View();
    virtual ~Screen3View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

protected:
    uint8_t animationStep;

    // Khai báo bộ bắt sự kiện cho 2 nút Menu và Play Again
    Callback<Screen3View, const AbstractButton&> buttonCallback;
    void buttonClickHandler(const AbstractButton& src);
};

#endif // SCREEN3VIEW_HPP
