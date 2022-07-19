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

#ifndef CMS_CAMERAMOUSECONTROLLER_H
#define CMS_CAMERAMOUSECONTROLLER_H

#include <opencv/cv.h>
#include <QTime>
#include <QTimer>
#include "FeatureInitializationModule.h"
#include "TrackingModule.h"
#include "MouseControlModule.h"
#include "Point.h"
#include "Settings.h"

namespace CMS {

class CameraMouseController: public QObject
{
    Q_OBJECT

public:
    CameraMouseController(Settings &settings, ITrackingModule *trackingModule, MouseControlModule *controlModule);
    ~CameraMouseController();
    void processFrame(cv::Mat &frame);
    void processClick(Point position);
    bool isAutoDetectWorking();
    void five_secs_reset();

public slots:
    void slot_processClick();
    void slot_countdown();


private:
    Settings &settings;
    FeatureInitializationModule initializationModule;
    ITrackingModule *trackingModule;
    MouseControlModule *controlModule;
    cv::Mat prevFrame;
    QTime featureCheckTimer;
    QTime countdown = QTime(0,0,5);
};

} // namespace CMS

#endif // CMS_CAMERAMOUSECONTROLLER_H
