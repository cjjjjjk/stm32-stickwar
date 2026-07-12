#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{

}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
    presenter->resetGameMatch(); // Reset wins và round về 0 và 1
}
void Screen2Presenter::activate()
{

}

void Screen2Presenter::deactivate()
{

}
