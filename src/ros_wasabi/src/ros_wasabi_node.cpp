#include <ros/ros.h>
#include <WASABIEngine.h>

#include "ros_wasabi_xmlparser.h"

int main(int argc, char *argv[])
{
	ros::init(argc, argv, "hello_ros");
	ros::NodeHandle nh;
	WASABIEngine* wasabi = new WASABIEngine("secondary");
	wasabi->initAllEAs();
	ros_wasabi_xmlparser* rwx = new ros_wasabi_xmlparser();
	std::vector<cogaEmotionalAttendee*>::iterator iter_ea;
	for (iter_ea = wasabi->emoAttendees.begin(); iter_ea != wasabi->emoAttendees.end(); ++iter_ea){
		cogaEmotionalAttendee* ea = (*iter_ea);
		if (!(ea->initialized)){
			qDebug() << "WASABIQtWindow::Trying to initialize EA " << ea->getLocalID() << " with xml file " << ea->EmoConPerson->xmlFilename.c_str();
			if (rwx->initEAbyXML(ea)) {
				std::cout << "WASABIQtWindow: xml initialization successful!" << std::endl;
			}
			else {
				std::cerr << "WASABIQtWindow: xml initialization FAILED!!!" << std::endl;
				std::cerr << "Failure to load " << ea->EmoConPerson->xmlFilename.c_str() << ", (which should be in " << std::endl; // << dir.currentPath() << "/xml/)!" 
			}
		}
	}

	ROS_INFO_STREAM("Hello, ROS!");
	return 0;
}

