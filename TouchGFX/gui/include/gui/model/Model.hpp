#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    // --- THÊM CÁC HÀM QUẢN LÝ BEST OF 3 Ở ĐÂY ---
    void increaseWinA();
    void increaseWinB();
    int getWinsA() const { return winsA; }
    int getWinsB() const { return winsB; }

    void nextRound();
    int getCurrentRound() const { return currentRound; }

    void resetMatchData();
    // --------------------------------------------

protected:
    ModelListener* modelListener;

    // --- THÊM CÁC BIẾN TRẠNG THÁI Ở ĐÂY ---
    int winsA;
    int winsB;
    int currentRound;
    // --------------------------------------
};

#endif // MODEL_HPP