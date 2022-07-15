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

#ifdef Q_OS_LINUX
#include <opencv2/imgproc.hpp>
#endif

#include "CameraMouseController.h"


namespace CMS {

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

    if (trackingModule->isInitialized())
    {
        Point featurePosition = trackingModule->track(frame);
        if (!featurePosition.empty())
        {
            //harding coding the range of frame for now. Should change this to according to frame size
            if (featurePosition.X() >= 1892 || featurePosition.X() <= 90){
                qDebug("feature outta frame");
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
}

void CameraMouseController::processClick(Point position)
{
    if (!prevFrame.empty())
    {
        trackingModule->setTrackPoint(prevFrame, position);
        controlModule->restart();
    }
}

bool CameraMouseController::isAutoDetectWorking()
{
    return initializationModule.allFilesLoaded();
}

void CameraMouseController::five_secs_reset(){
    trackingModule->stopTracking();
    QTimer::singleShot(5000, this, SLOT(slot_processClick()));
}

void CameraMouseController::slot_processClick()
{
    /*width and height of the frame*/
    int width = (int) (prevFrame.size().width);
    int height = (int) (prevFrame.size().height);
    Point center = Point(width/2, height/2);
    if (!prevFrame.empty())
    {
        trackingModule->setTrackPoint(prevFrame, center);
        controlModule->restart();
    }
}



} // namespace CMS

