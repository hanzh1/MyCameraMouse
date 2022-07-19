/*                         Camera Mouse Suite
 *  Copyright (C) 2015, Andrew Kurauchi
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QObject>
#include <QTimer>
#include <qDebug>
#ifdef Q_OS_LINUX
#include <opencv2/imgproc.hpp>
#endif

#include "CameraMouseController.h"
#include <ImageProcessing.h>

namespace CMS {
using namespace std;
CameraMouseController::CameraMouseController(Settings &settings, ITrackingModule *trackingModule, MouseControlModule *controlModule) :
    settings(settings), trackingModule(trackingModule), controlModule(controlModule)
{
    featureCheckTimer.start();
}

CameraMouseController::~CameraMouseController()
{
    delete trackingModule;
    delete controlModule;
}

void CameraMouseController::processFrame(cv::Mat &frame)
{
    prevFrame = frame;
    int width = prevFrame.size().width;
    int height = prevFrame.size().height;
    if (trackingModule->isInitialized())
    {
        Point featurePosition = trackingModule->track(frame);
        if (!featurePosition.empty())
        {
            /*
             * x = 630
             * x = 10
             * y = 5
             * y = 460
            */

            //harding coding the range of frame for now.
            if (featurePosition.X() >= 600 || featurePosition.X() <= 45 || featurePosition.Y() <= 45 || featurePosition.Y() >= 430){
                qDebug("feature out of frame");
                if (1)/*change to 5-4-3-2-1 settings bool*/{
                    five_secs_reset();
                }
            }
            else if (settings.isAutoDetectNoseEnabled() && featureCheckTimer.elapsed() > 1000)
            {
                Point autoFeaturePosition = initializationModule.initializeFeature(frame);
                if (!autoFeaturePosition.empty())
                {
                    double distThreshSq = settings.getResetFeatureDistThreshSq();
                    Point disp = autoFeaturePosition - featurePosition;
                    if (disp * disp > distThreshSq)
                    {
                        trackingModule->setTrackPoint(frame, autoFeaturePosition);
                        controlModule->setScreenReference(controlModule->getPrevPos());
                        controlModule->restart();
                        featurePosition = autoFeaturePosition;
                    }
                    featureCheckTimer.restart();
                }
            }
            trackingModule->drawOnFrame(frame, featurePosition);

            controlModule->update(featurePosition);
        }

    }
    else if (settings.isAutoDetectNoseEnabled())
    {
        Point initialFeaturePosition = initializationModule.initializeFeature(frame);
        if (!initialFeaturePosition.empty())
        {
            trackingModule->setTrackPoint(frame, initialFeaturePosition);
            controlModule->setScreenReference(settings.getScreenResolution()/2);
            controlModule->restart();
        }
    }
    else if (settings.isFiveSecOnLoss()) {
        trackingModule->drawOnFrame(frame,Point(width/2,height/2));
        ImageProcessing::drawText(prevFrame, std::to_string(countdown.second()), width/2 -10,height/2 - 10);
    }
}

void CameraMouseController::processClick(Point position)
{
    if (!prevFrame.empty())
    {
        trackingModule->setTrackPoint(prevFrame, position);
        controlModule->restart();
    }
    qDebug() << position.X();
    qDebug() << position.Y();
}

bool CameraMouseController::isAutoDetectWorking()
{
    return initializationModule.allFilesLoaded();
}

void CameraMouseController::five_secs_reset(){
    countdown = QTime(0,0,5);//resetting countdown
    trackingModule->stopTracking();
    settings.setFiveSecOnLoss(true);
    //updating countdown every second
    QTimer::singleShot(1000,this,SLOT(slot_countdown()));
    QTimer::singleShot(2000,this,SLOT(slot_countdown()));
    QTimer::singleShot(3000,this,SLOT(slot_countdown()));
    QTimer::singleShot(4000,this,SLOT(slot_countdown()));
    QTimer::singleShot(5000,this,SLOT(slot_countdown()));
    //processClick on center after 5 secs
    QTimer::singleShot(5000, this, SLOT(slot_processClick()));
    }

void CameraMouseController::slot_processClick()
{
    /*width and height of the frame. Make this a global???*/
    int width = prevFrame.size().width;
    int height = prevFrame.size().height;
    Point center = Point(width/2, height/2);
    if (!prevFrame.empty())
    {
        trackingModule->setTrackPoint(prevFrame, center);
        controlModule->restart();
    }
}

void CameraMouseController::slot_countdown(){
    countdown = countdown.addSecs(-1);
}

} // namespace CMS

