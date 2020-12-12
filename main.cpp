#include "mainHeader.h"
#include "FaceDetectorHeader.h"
#include "ObjectColorDetectorHeader.h"
#include "InitializerHeader.h"
#include "FrameHelperHeader.h"

int wk = 0;
Rect excludedFaceROI;

int main()
{
	connectToVideo(); //Подключаемся к любой доступной камере
	initControlWindows(); //Инициализация окна контроля
	loadCascades(); //Инициализация каскадов для трекинга лица
	clockInit(); //Инициализация таймеров для отключения/включения монитора
	
	while(true)
    {
		getFrame(cap, bgrImage); //Получаем кадр из видеокамеры
		depthMap = getFrameForObjectTracking(bgrImage); //Обрабатываем кадр для трекинга объекта
		grayscaleFrame = getGrayFrame(bgrImage); //Обрабатываем полученный кадр для трекинга лица
		excludedFaceROI = detectFace(grayscaleFrame, bgrImage); //Поиск лица, отключение монитора
		detectColorObject(bgrImage, depthMap, excludedFaceROI); //Трекинг объекта
		
		wk = waitKey( 20 ); //Нажатие клавиши
		if( wk == 27 ) //ESC
		{
			break; //Выход 
		}
		else if (wk == 32 )//Space
		{
			scanMode();//Запуск автоматичесского сканирования цвета
		}
		else if (wk == 67)//C
		{
			colorControlPanel();//Запуск ручного скинрования цвета
		}
		else if(wk == 86)//V
		{
			destroyScanModeWindow();//Закрытие окна автоматического сканирования
		}
	}
	
	waitKey();
	return 0;
}