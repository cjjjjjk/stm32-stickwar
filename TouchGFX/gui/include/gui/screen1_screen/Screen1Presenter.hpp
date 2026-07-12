#ifndef SCREEN1PRESENTER_HPP
#define SCREEN1PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen1View;

class Screen1Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen1Presenter(Screen1View& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Screen1Presenter() {}
    // Lấy dữ liệu từ Model
    int getWinsA() { return model->getWinsA(); }
    int getWinsB() { return model->getWinsB(); }
    int getCurrentRound() { return model->getCurrentRound(); }

    // Cập nhật lên Model khi có người thắng round
    void playerAWonRound() { model->increaseWinA(); }
    void playerBWonRound() { model->increaseWinB(); }

private:
    Screen1Presenter();

    Screen1View& view;
};

#endif // SCREEN1PRESENTER_HPP
