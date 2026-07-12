#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() : modelListener(0), winsA(0), winsB(0), currentRound(1)
{
}

void Model::tick()
{

}

// 2. Thêm nội dung các hàm xử lý logic
void Model::increaseWinA() 
{ 
    if (winsA < 2) winsA++; 
}

void Model::increaseWinB() 
{ 
    if (winsB < 2) winsB++; 
}

void Model::nextRound() 
{ 
    currentRound++; 
}

void Model::resetMatchData() 
{
    winsA = 0;
    winsB = 0;
    currentRound = 1;
}