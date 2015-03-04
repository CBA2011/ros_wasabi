#include "ros_wasabi_xmlparser.h"

using namespace std;

// Used internally to temporarily store data collected from the xml file
struct emodata {
	std::string category; // fsre-, occ-, or everyday-category name, e.g. 'relaxed'
	std::string emoclass; // primary or secondary
	int xTens;
	int yTens;
	int slope;
	int mass;
	int xReg;
	int boredom;
	int prevalence;
	float base_intensity;
	float act_threshold;
	float sat_threshold;
	std::string decay;
	float pleasure, arousal, dominance;
	float lifetime;
	std::vector<AffectPolygon*> affect_polygons;
} tmp_emodata;


/*!
* Initializing ea's using the xml file.
*/
bool ros_wasabi_xmlparser::initEAbyXML(cogaEmotionalAttendee* ea)
{
	QString filename = "xml/";
	filename = filename.append(ea->EmoConPerson->xmlFilename.c_str());
	QFile file;
	if (!(file.exists(filename))) {
		qDebug() << "WASABIQtWindow::initEAbyXML: file " << filename << " not found.";
		return false;
	}
	QXmlStreamReader xml;
	file.setFileName(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "WASABIQtWindow::initEAbyXML: Could not open " << filename << " read-only in text mode.";
		return false;
	}
	xml.setDevice(&file);
	if (xml.readNextStartElement()) {
		std::cout << "xml.name()='''" << xml.name().toString().toStdString() << "'''" << std::endl;
		if (xml.name() == "emotionml" && xml.attributes().value("version") == "1.0") {
			return readEmotionML(xml, ea);
		}
		else
			xml.raiseError(QObject::tr("The file is not an EMOTIONML version 1.0 file."));
	}
	return false;
}

bool ros_wasabi_xmlparser::readEmotionML(QXmlStreamReader& xml, cogaEmotionalAttendee* ea) {
	Q_ASSERT(xml.isStartElement() && xml.name() == "emotionml");
	bool returnValue = false;
	tmp_emodata.affect_polygons.clear();
	tmp_emodata.emoclass = "undef";
	while (xml.readNextStartElement()) {
		std::cout << "^^^xml.name()='''" << xml.name().toString().toStdString() << "'''" << std::endl;
		if (xml.name() == "info")
			returnValue = readInfo(xml, ea);
		else if (xml.name() == "emotion") {
			std::cout << "***WASABIQtWindow::readEmotionML: xml.name()='''" << xml.name().toString().toStdString() << "'''" << std::endl;
			returnValue = readEmotion(xml, ea);
			if (returnValue && tmp_emodata.emoclass != "undef") {
//We got a primary or secondary emotion
				if (tmp_emodata.emoclass == "primary") {
					std::vector<float> p_a_d_ID_max_min_baseInt;
					p_a_d_ID_max_min_baseInt.push_back(tmp_emodata.pleasure);
					p_a_d_ID_max_min_baseInt.push_back(tmp_emodata.arousal);
					p_a_d_ID_max_min_baseInt.push_back(tmp_emodata.dominance);
					p_a_d_ID_max_min_baseInt.push_back(-1); //MOOD_ID is obsolete without agent MAX!
					p_a_d_ID_max_min_baseInt.push_back(tmp_emodata.sat_threshold);
					p_a_d_ID_max_min_baseInt.push_back(tmp_emodata.act_threshold);
					p_a_d_ID_max_min_baseInt.push_back(tmp_emodata.base_intensity);
					ea->EmoConPerson->buildPrimaryEmotion(p_a_d_ID_max_min_baseInt, tmp_emodata.category, tmp_emodata.decay);
				}
				else if (tmp_emodata.emoclass == "secondary" && tmp_emodata.affect_polygons.size() > 0) {
					SecondaryEmotion* se = new SecondaryEmotion();
					EmotionDynamics* me = dynamic_cast<EmotionDynamics*>(ea->EmoConPerson);
				if (me) {
					se->setEmotionContainer(me);
				}
				se->addPolygon(tmp_emodata.affect_polygons);
				if (!se->setStandardLifetime((double)tmp_emodata.lifetime) || !se->setLifetime((double)tmp_emodata.lifetime)) {
					cerr << "WASABIQtWindow::readEmotionML: WARNING invalid lifetime " << (double)tmp_emodata.lifetime << endl;
				}
				if (!se->setBaseIntensity((double)tmp_emodata.base_intensity)) {
					cerr << "SecondaryEmotion::loadFromFile: WARNING invalid baseIntensity " << (double)tmp_emodata.base_intensity << endl;
				}
				se->type = tmp_emodata.category;
				if (tmp_emodata.decay == "linear") {
					se->setDecayFunction(SecondaryEmotion::LINEAR);
				}
				else if (tmp_emodata.decay == "none") {
					se->setDecayFunction(SecondaryEmotion::NONE);
				}
				else if (tmp_emodata.decay == "exponential") {
					se->setDecayFunction(SecondaryEmotion::EXPONENTIAL);
				}
				else if (tmp_emodata.decay == "cosine") {
					se->setDecayFunction(SecondaryEmotion::COSINE);
				}
				ea->EmoConPerson->affectiveStates.push_back(se);
			}
		}
	}
	else
		xml.skipCurrentElement();
	}
	return returnValue;
}

bool ros_wasabi_xmlparser::readInfo(QXmlStreamReader& xml, cogaEmotionalAttendee* ea) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "info");
    xml.readNext();
    std::cout << xml.tokenType() << " tokenType!" << std::endl;
    while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
             xml.name() == "info")) {
        if (xml.name() == "" || xml.tokenType() == QXmlStreamReader::EndElement) {
            xml.readNext();
            continue;
        }
        std::cout << "WASABIQtWindow::readInfo: xml.name()='''" << xml.name().toString().toStdString() << "'''" << std::endl;
        //std::cout << "WASABIQtWindow::readInfo: xml.namespaceUri()='''" << xml.namespaceUri().toString().toStdString() << "'''" << std::endl;
        //This results in the http address!
        if (xml.name() == "parameter") {
            if ( xml.namespaceUri().toString().toStdString() != "http://www.becker-asano.de/WASABI/Shema/WASABI") {
                 std::cout << "WASABIQtWindow::readInfo: wrong namespaceUri, only http://www.becker-asano.de/WASABI/Shema/WASABI allowed:" << std::endl;
                 xml.readNext();
                 continue;
            }
            if (xml.attributes().hasAttribute("type") && xml.attributes().value("type") == "dynamic") {
                // These are the general dynamic infos
                std::cout << "xml.attributes().value(\"name\") = " << xml.attributes().value("name").toString().toStdString() << std::endl;
                QStringRef nameValue = xml.attributes().value("name");
                bool ok;
                int value;
                switch (returnIndex(nameValue.toString().toStdString(), "xTens yTens slope mass xReg yReg boredom prevalence")) {
                case 1: //xTens
                    value = xml.attributes().value("value").toInt(&ok);
                    if (ok) {
                        std::cout << "setting xTens for EA " << ea->getLocalID() << std::endl;
                        ea->EmoConPerson->xTens = value;
                    }
                    else {
                        qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                    }
                    break;
                 case 2: //yTens
                    value = xml.attributes().value("value").toInt(&ok);
                    if (ok) {
                        std::cout << "setting yTens for EA " << ea->getLocalID() << std::endl;
                        ea->EmoConPerson->yTens = value;
                    }
                    else {
                        qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                    }
                    break;
                case 3: //slope
                   value = xml.attributes().value("value").toInt(&ok);
                   if (ok) {
                       std::cout << "setting slope for EA " << ea->getLocalID() << std::endl;
                       ea->EmoConPerson->slope = value;
                   }
                   else {
                       qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                   }
                   break;
                case 4: //mass
                   value = xml.attributes().value("value").toInt(&ok);
                   if (ok) {
                       std::cout << "setting mass for EA " << ea->getLocalID() << std::endl;
                       ea->EmoConPerson->mass = value;
                   }
                   else {
                       qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                   }
                   break;
                case 5: //xReg
                   value = xml.attributes().value("value").toInt(&ok);
                   if (ok) {
                       std::cout << "setting xReg for EA " << ea->getLocalID() << std::endl;
                       ea->EmoConPerson->xReg = value;
                   }
                   else {
                       qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                   }
                   break;
                case 6: //yReg
                   value = xml.attributes().value("value").toInt(&ok);
                   if (ok) {
                       std::cout << "setting yReg for EA " << ea->getLocalID() << std::endl;
                       ea->EmoConPerson->yReg = value;
                   }
                   else {
                       qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                   }
                   break;
                case 7: //boredom
                   value = xml.attributes().value("value").toInt(&ok);
                   if (ok) {
                       std::cout << "setting boredom for EA " << ea->getLocalID() << std::endl;
                       ea->EmoConPerson->boredom = value;
                   }
                   else {
                       qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                   }
                   break;
                case 8: //prevalence
                   value = xml.attributes().value("value").toInt(&ok);
                   if (ok) {
                       std::cout << "setting prevalence for EA " << ea->getLocalID() << std::endl;
                       ea->EmoConPerson->prevalence = value;
                   }
                   else {
                       qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to integer.";
                   }
                   break;
                default:
                    qDebug() << "WASABIQtWindow::readInfo: error reading unknown value " << xml.attributes().value("value") << "!";
                }
                //xml.skipCurrentElement();
            }
            else //i.e. if !(xml.attributes().hasAttribute("type") && xml.attributes().value("type") == "dynamic")
            {
                std::cout << "WASABIQtWindow::readInfo: error: only 'parameter' tags with attribute with 'type=dynamic' allowed on top level." << std::endl;
            }
        }
        else if (xml.name() == "primary") {
            tmp_emodata.emoclass = "primary";
            // We have to parse a primary emotion
            xml.readNext();
            while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
                     xml.name() == "primary")) {
                if (xml.name() == "" || xml.tokenType() == QXmlStreamReader::EndElement) {
                    xml.readNext();
                    continue;
                }
                std::cout << "xml.name()='''" << xml.name().toString().toStdString() << "''', tokenType = " << xml.tokenType() << std::endl;
                if (xml.name() == "parameter") {
                    std::cout << "xml.attributes().value(\"name\") = " << xml.attributes().value("name").toString().toStdString() << std::endl;
                    QStringRef nameValue = xml.attributes().value("name");
                    //std::cout << "xml.namespaceUri()='''" << xml.namespaceUri().toString().toStdString() << "'''" << std::endl;
                    if ( xml.namespaceUri().toString().toStdString() != "http://www.becker-asano.de/WASABI/Shema/WASABI") {
                         std::cout << "WASABIQtWindow::readInfo: wrong namespaceUri, only http://www.becker-asano.de/WASABI/Shema/WASABI allowed:" << std::endl;
                         xml.readNext();
                         continue;
                    }
                    bool ok;
                    float value;
                    switch (returnIndex(nameValue.toString().toStdString(), "base_intensity act_threshold sat_threshold decay")) {
                    case 1: //base_intensity
                        value = xml.attributes().value("value").toFloat(&ok);
                        if (ok) {
                            std::cout << "setting base_intensity for EA " << ea->getLocalID() << std::endl;
                            tmp_emodata.base_intensity = value;
                        }
                        else {
                            qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                        }
                        break;
                     case 2: //act_threshold
                        value = xml.attributes().value("value").toFloat(&ok);
                        if (ok) {
                            std::cout << "setting activation threshold for EA " << ea->getLocalID() << std::endl;
                            tmp_emodata.act_threshold = value;
                        }
                        else {
                            qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                        }
                        break;
                    case 3: //sat_threshold
                       value = xml.attributes().value("value").toFloat(&ok);
                       if (ok) {
                           std::cout << "setting saturation threshold for EA " << ea->getLocalID() << std::endl;
                           tmp_emodata.sat_threshold = value;
                       }
                       else {
                           qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                       }
                       break;
                    case 4: //decay
                       std::cout << "setting decay function for EA " << ea->getLocalID() << std::endl;
                       tmp_emodata.decay = xml.attributes().value("value").toString().toStdString();
                       break;
                    default:
                        qDebug() << "WASABIQtWindow::readInfo: (primary) error reading incorrect parameter value " << xml.attributes().value("value") << "!";
                    }
                }
                xml.readNext();
            }
        }
        else if (xml.name() == "secondary") {
            // We have to parse a primary emotion
            tmp_emodata.emoclass = "secondary";
            tmp_emodata.affect_polygons.clear();
            xml.readNext();
            while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
                     xml.name() == "secondary") && xml.tokenType() != QXmlStreamReader::Invalid) {
                if (xml.name() == "" || xml.tokenType() == QXmlStreamReader::EndElement) {
                    xml.readNext();
                    continue;
                }
                std::cout << "xml.name()='''" << xml.name().toString().toStdString() << "''', tokenType = " << xml.tokenType() << std::endl;
                //std::cout << "xml.namespaceUri()='''" << xml.namespaceUri().toString().toStdString() << "'''" << std::endl;
                if ( xml.namespaceUri().toString().toStdString() != "http://www.becker-asano.de/WASABI/Shema/WASABI") {
                     std::cout << "WASABIQtWindow::readInfo: wrong namespaceUri, only http://www.becker-asano.de/WASABI/Shema/WASABI allowed:" << std::endl;
                     xml.readNext();
                     continue;
                }
                if (xml.name() == "parameter") {
                    std::cout << "xml.attributes().value(\"name\") = " << xml.attributes().value("name").toString().toStdString() << std::endl;
                    QStringRef nameValue = xml.attributes().value("name");
                    bool ok;
                    float value;
                    switch (returnIndex(nameValue.toString().toStdString(), "base_intensity lifetime decay")) {
                    case 1: //base_intensity
                        value = xml.attributes().value("value").toFloat(&ok);
                        if (ok) {
                            std::cout << "setting base_intensity for EA " << ea->getLocalID() << std::endl;
                            tmp_emodata.base_intensity = value;
                        }
                        else {
                            qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                        }
                        break;
                     case 2: //lifetime
                        value = xml.attributes().value("value").toFloat(&ok);
                        if (ok) {
                            std::cout << "setting lifetime threshold for EA " << ea->getLocalID() << std::endl;
                            tmp_emodata.lifetime = value;
                        }
                        else {
                            qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                        }
                        break;
                    case 3: //decay
                       std::cout << "setting decay function for EA " << ea->getLocalID() << std::endl;
                       tmp_emodata.decay = xml.attributes().value("value").toString().toStdString();
                       break;
                    default:
                        qDebug() << "WASABIQtWindow::readInfo: (secondary) error reading incorrect parameter value " << xml.attributes().value("value") << "!";
                    }
                }
                else if(xml.name() == "polygon"){
                    // We have to parse a primary emotion
                    QStringRef type = xml.attributes().value("type");
                    if (type != "QUAD") {
                        std::cout << "WASABIQtWindow::readInfo: wrong type '''" << type.toString().toStdString() << "''' for polygon" << std::endl;
                        break;
                    }
                    std::vector<AffectVertex*> affect_vertices;
                    xml.readNext();
                    while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
                             xml.name() == "polygon")) {
                        if (xml.name() == "" || xml.tokenType() == QXmlStreamReader::EndElement) {
                            xml.readNext();
                            continue;
                        }
                        std::cout << "xml.name()='''" << xml.name().toString().toStdString() << "''', tokenType = " << xml.tokenType() << std::endl;
                        //std::cout << "xml.namespaceUri()='''" << xml.namespaceUri().toString().toStdString() << "'''" << std::endl;
                        if ( xml.namespaceUri().toString().toStdString() != "http://www.becker-asano.de/WASABI/Shema/WASABI") {
                             qDebug() << "WASABIQtWindow::readInfo: wrong namespaceUri, only http://www.becker-asano.de/WASABI/Shema/WASABI allowed:";
                             xml.readNext();
                             continue;
                        }
                        if (xml.name() == "vertex") {

                            if (xml.attributes().hasAttribute("pleasure") && xml.attributes().hasAttribute("arousal") && xml.attributes().hasAttribute("dominance") && xml.attributes().hasAttribute("intensity")) {
                                float pad_f[3]; //pleasure, arousal, dominance;
                                int pad[3];
                                float intensity;
                                bool ok;
                                pad_f[0] = xml.attributes().value("pleasure").toFloat(&ok);
                                if (ok) {
                                    pad[0] = (int)((pad_f[0] * 2 - 1.0) * 100); // old range [0.0, 1.0], new range [-100, 100]
                                    pad_f[1] = xml.attributes().value("arousal").toFloat(&ok);
                                    if (ok) {
                                        pad[1] = (int)((pad_f[1] * 2 - 1.0) * 100);
                                        pad_f[2] = xml.attributes().value("dominance").toFloat(&ok);
                                        if (ok) {
                                            pad[2] = (int)((pad_f[2] * 2 - 1.0) * 100);
                                            intensity = xml.attributes().value("intensity").toFloat(&ok);
                                            if (ok) {
                                                affect_vertices.push_back(new AffectVertex(pad, (double)intensity));
                                            }
                                        }
                                    }
                                }
                                if (!ok) {
                                    std::cout << "WASABIQtWindow::readInfo: at least one dimension or intensity was not a float value!" << std::endl;
                                }
                            }
                            else {
                                std::cout << "WASABIQtWindow::readInfo: at least one dimension or intensity is missing in vertex defnition" << std::endl;
                            }
                        }
                        xml.readNext();
                    }
                    if (affect_vertices.size() > 0) {
                        AffectPolygon* ap = new AffectPolygon(affect_vertices, "QUAD");
                        tmp_emodata.affect_polygons.push_back(ap);
                    }
                }
                xml.readNext();
            }
        }
        xml.readNext();
    }
    return true;
}


bool ros_wasabi_xmlparser::readEmotion(QXmlStreamReader& xml, cogaEmotionalAttendee* ea) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "emotion");
    bool returnValue = false;;
    xml.readNext();
    while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
             xml.name() == "emotion")) {
        if (xml.name() == "" || xml.tokenType() == QXmlStreamReader::EndElement) {
            xml.readNext();
            continue;
        }
        std::cout << "WASABIQtWindow::readEmotion xml.name()='''" << xml.name().toString().toStdString() << "'''" << std::endl;
        if (xml.name() == "info") {
            std::cout << "------------------inner readInfo start--------------------" << std::endl;
            returnValue = readInfo(xml, ea);
            std::cout << "------------------inner readInfo end  --------------------" << std::endl;
        }
        else if (xml.name() == "category") {
            std::string attrname = xml.attributes().value("name").toString().toStdString();
            std::cout << "WASABIQtWindow::readEmotion xml.attributes().value(\"name\") = " << attrname << std::endl;
            if (attrname.length() == 0) {
                std::cerr << "Attribute " << attrname << ".length() == 0!, bailing out." << std::endl;
                returnValue = false;
                break;
            }
            tmp_emodata.category = xml.attributes().value("name").toString().toStdString();
            returnValue = true;
        }
        else if (xml.name() == "dimension") {
            std::string attrname = xml.attributes().value("name").toString().toStdString();
            std::cout << "WASABIQtWindow::readEmotion xml.attributes().value(\"name\") = " << attrname << std::endl;
            if (attrname.length() == 0) {
                std::cerr << "Attribute " << attrname << ".length() == 0!, bailing out." << std::endl;
                returnValue = false;
                break;
            }
            bool ok;
            float value;
            // We have to rescale these values from [0, 1] to [-1, 1] !!!
            switch (returnIndex(attrname, "pleasure arousal dominance")) {
            case 1: //pleasure
                value = xml.attributes().value("value").toFloat(&ok);
                if (ok) {
                    value = value * 2 -1;
                    std::cout << "setting pleasure for EA " << ea->getLocalID() << " with emotion " << tmp_emodata.category << "." << std::endl;
                    tmp_emodata.pleasure = value;
                }
                else {
                    qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                }
                break;
             case 2: //arousal
                value = xml.attributes().value("value").toFloat(&ok);
                if (ok) {
                    value = value * 2 -1;
                    std::cout << "setting arousal for EA " << ea->getLocalID() << " with emotion " << tmp_emodata.category << "." << std::endl;
                    tmp_emodata.arousal = value;
                }
                else {
                    qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
                }
                break;
            case 3: //dominance
               value = xml.attributes().value("value").toFloat(&ok);
               if (ok) {
                   value = value * 2 -1;
                   std::cout << "setting dominance for EA " << ea->getLocalID() << " with emotion " << tmp_emodata.category << "." << std::endl;
                   tmp_emodata.dominance = value;
               }
               else {
                   qDebug() << "WASABIQtWindow::readInfo: error converting " << xml.attributes().value("value") << " to float.";
               }
               break;
            default:
                qDebug() << "WASABIQtWindow::readInfo: error reading incorrect parameter value " << xml.attributes().value("value") << "!";
            }
            returnValue = true;
        }
        //else {
            //xml.skipCurrentElement();
        //}
        //xml.skipCurrentElement();
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            std::cout << "WASABIQtWindow::readEmotion StartElement with name " << xml.name().toString().toStdString() << std::endl;
        }
        if (xml.tokenType() == QXmlStreamReader::Invalid) {
            std::cerr << "WASABIQtWindow::readEmotion Invalid element, bailing out! " << std::endl;
            returnValue = false;
            break;
        }
    }
    return returnValue;
}
