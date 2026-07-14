#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
protected:
    uint8_t current_highlight_mode = 0;
    Callback<Screen2View, const AbstractButton&> buttonCallback;
	void buttonClickHandler(const AbstractButton& src);
};

#endif // SCREEN2VIEW_HPP
