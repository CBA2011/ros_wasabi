#ifndef ROS_WASABI_XMLPARSER_H
#define ROS_WASABI_XMLPARSER_H
#include <QDebug>
#include <QDir>
#include <QXmlStreamReader>
#include "WASABIEngine.h"
#include "SecondaryEmotion.h"

class ros_wasabi_xmlparser
{
public:
	ros_wasabi_xmlparser() {}
	//~ros_wasabi_xmlparser() {}

	bool initEAbyXML(cogaEmotionalAttendee* ea);
	bool readEmotionML(QXmlStreamReader& xml, cogaEmotionalAttendee *ea);
	bool readInfo(QXmlStreamReader& xml, cogaEmotionalAttendee *ea);
	bool readEmotion(QXmlStreamReader& xml, cogaEmotionalAttendee *ea);
};


#endif // ROS_WASABI_XMLPARSER_H