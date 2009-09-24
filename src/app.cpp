#include <iostream>
#include <cstdlib>
#include <cctype>
#include <exception>
#include <stdexcept>
#include <log4cxx/logger.h>
#include <log4cxx/level.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include "app.h"
#include "appinfo.h"

namespace
{
    bool matches_option(const QString& str1, const QString& str2, int mindashes=1, int maxdashes=2){
        int dashes = 0;
        if ( str2.length() > 0 ){
            while ((dashes<str2.length())&&(str2[dashes]=='-')){
                dashes++;
            }
        }
        if ( (dashes < mindashes) || (dashes > maxdashes) ){
            return false;
        }
        QString substr=str2.right(str2.length()-dashes);
        return (str1.compare(substr,Qt::CaseInsensitive)==0);
    }
}

inline std::ostream& operator<<(std::ostream& out, const QString& str)
{
    QByteArray a = str.toUtf8();
    out << a.constData();
    return out;
}

App::App(int& argc, char** argv) : QApplication(argc,argv), _invocation(argv[0]), _gui(false), _interactive(false)
{
    // Enforce singleton property
    if ( _instance ){
        throw std::runtime_error("Only one instance of App allowed.");
    }

    // Set the singleton instance to this
    _instance = this;

    // Set the application properties
    setApplicationName(APPLICATION_NAME);
    setApplicationVersion(APPLICATION_VERSION_STRING);
    setOrganizationName(APPLICATION_VENDOR_NAME);
    setOrganizationDomain(APPLICATION_VENDOR_URL);
    
    // Configure the logging mechanism
    log4cxx::LoggerPtr rootlogger = log4cxx::Logger::getRootLogger();
    rootlogger->addAppender(new log4cxx::ConsoleAppender(new log4cxx::PatternLayout("[%-5p] %m%n")));
    
    // Parse the commandline
    int idx = 1;
    while ( idx < argc ){
        QString arg(argv[idx]);
        if ( matches_option(arg,"help",0) || matches_option(arg,"h") || matches_option(arg,"?",0) ){
            printHelpMessage();
            std::exit(0);
        }else if ( matches_option(arg,"version",0) ){
            printVersionMessage();
            std::exit(0);
        }else if ( matches_option(arg,"version-triplet") ){
            printVersionTripletMessage();
            std::exit(0);
        }else if ( matches_option(arg,"prefset") ){
            // Verify that there is another argument
            if ( (idx+1) >= argc ){
                LOG4CXX_FATAL(_logger,"Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }
            
            // Increment the index
            idx++;
            
            // Get the next parameter
            std::string param(argv[idx]);
            
            // Determine if there is an equals sign
            // If there is, set the preference;
            // Otherwise, remove the preference
            size_t eqidx = param.find('=');
            if ( eqidx != std::string::npos ){
                std::string key = param.substr(0,eqidx);
                std::string val = param.substr(eqidx+1);
                setPreference(key,val);
            }else{
                unsetPreference(param);
            }
        }else if ( matches_option(arg,"prefdel") ){
            // Verify that there is another argument
            if ( (idx+1) >= argc ){
                LOG4CXX_FATAL(_logger,"Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }
            
            // Increment the index
            idx++;
            
            // Get the next parameter
            std::string param(argv[idx]);
            
            // Remove the preference
            unsetPreference(param);
        }else if ( matches_option(arg,"preflist") ){
            printAllPreferences();
        }else if ( matches_option(arg,"prefget") ){
            // Verify that there is another argument
            if ( (idx+1) >= argc ){
                LOG4CXX_FATAL(_logger,"Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }
            
            // Increment the index
            idx++;
            
            // Get the next parameter
            std::string param(argv[idx]);
            
            // Print the preference
            printPreference(param);
        }else if ( matches_option(arg,"loglevel") ){
            // Verify that there is another argument
            if ( (idx+1) >= argc ){
                LOG4CXX_FATAL(_logger,"Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }
            
            // Increment the index
            idx++;
            
            // Get the next parameter
            std::string param(argv[idx]);
            
            // Determine if there is an equals sign and act accordingly
            size_t eqidx = param.find('=');
            if ( eqidx != std::string::npos ){
                std::string logger = param.substr(0,eqidx);
                std::string level  = param.substr(eqidx+1);
                setLogLevel(logger,level);
            }else{
                setLogLevel("",param);
            }
        }else if ( matches_option(arg,"appid") || matches_option(arg,"application-identifier") ){
            printApplicationIdentifier();
            std::exit(0);
        }else if ( matches_option(arg,"gui") ){
            if ( _interactive ){
                LOG4CXX_FATAL(_logger,"Cannot specify both \"--gui\" and \"--interactive\" simultaneously.");
                std::exit(1);
            }
            if ( _gui ){
                LOG4CXX_WARN(_logger,"Option \"" << arg << "\" already specified. Ignoring.");
            }
            _gui = true;
        }else if ( matches_option(arg,"interactive") ){
            if ( _gui ){
                LOG4CXX_FATAL(_logger,"Cannot specify both \"--gui\" and \"--interactive\" simultaneously.");
                std::exit(1);
            }
            if ( _interactive ){
                LOG4CXX_WARN(_logger,"Option \"" << arg << "\" already specified. Ignoring.");
            }
            _interactive = true;
        }else{
            LOG4CXX_WARN(_logger,"Unrecognized option: \"" << arg << "\". Ignoring");
        }
        idx++;
    }
    
    
    if ( _gui ){
        initGUI();
    }else if (_interactive) {
        interactiveMain();
        std::exit(0);
    }else{
        consoleMain();
        std::exit(0);
    }
}

App::~App()
{
}

App*
App::INSTANCE()
{
    return _instance;
}

void 
App::initGUI()
{
    // Construct the main window
    _mainwindow.reset(new QMainWindow);
    _mainwindow->setCentralWidget(new QWidget);
    
    // Setup the central widget
    QWidget* centralwidget = _mainwindow->centralWidget();
    QLabel* label = new QLabel("Hello world!",centralwidget);
    
    // Setup the toolbars
    // ...
    
    
    // Setup the icons
    // ...
    
    // Display the main window
    _mainwindow->setVisible(true);
}

void 
App::interactiveMain()
{
    std::cout << "Hello world!" << std::endl;
}

void 
App::consoleMain()
{
    std::cout << "Hello world!" << std::endl;
}

void 
App::printHelpMessage()
{
    std::cout << "Usage: " << getProjectInvocation() << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    --help                       Displays this help message." << std::endl;
    std::cout << "    --version                    Prints the program version." << std::endl;
    std::cout << "    --version-triplet            Prints the undecorated program version." << std::endl;
    std::cout << "    --appid                      Prints the unique application identifier." << std::endl;
    std::cout << "    --prefset <key>=<val>        Sets the given preference." << std::endl;
    std::cout << "    --prefdel <key>              Unsets the given preference." << std::endl;
    std::cout << "    --prefget <key>              Prints the given preference." << std::endl;
    std::cout << "    --preflist                   Lists all preferences that are set." << std::endl;
    std::cout << "    --loglevel <level>           Sets the current logging level." << std::endl;
    std::cout << "    --loglevel <logger>=<level>  Sets the logging level for the given logger." << std::endl;
    std::cout << "    --gui                        Run in graphical user interface mode." << std::endl;
    std::cout << "    --interactive                Run in interactive commandline mode." << std::endl;
    std::cout << "Log Levels:" << std::endl;
    std::cout << "    all" << std::endl;
    std::cout << "    trace" << std::endl;
    std::cout << "    debug" << std::endl;
    std::cout << "    info" << std::endl;
    std::cout << "    warn" << std::endl;
    std::cout << "    error" << std::endl;
    std::cout << "    fatal" << std::endl;
    std::cout << "    off" << std::endl;
}

void 
App::printVersionMessage()
{
    std::cout << getProjectName() << " v" << getProjectVersion() << std::endl;
    std::cout << getProjectVendorName() << "; Copyright (C) " << getProjectCopyrightYears();
}

void 
App::printVersionTripletMessage()
{
    std::cout << getProjectVersion() << std::endl;
}

void 
App::printApplicationIdentifier()
{
    std::cout << getProjectID() << std::endl;
}

QString 
App::getProjectName()
{
    return APPLICATION_NAME;
}


QString 
App::getProjectCodeName()
{
    return APPLICATION_CODENAME;
}

QString 
App::getProjectVendorID()
{
    return APPLICATION_VENDOR_ID;
}

QString 
App::getProjectVendorName()
{
    return APPLICATION_VENDOR_NAME;
}

QString 
App::getProjectID()
{
    return APPLICATION_ID;
}

int 
App::getProjectMajorVersion()
{
    return APPLICATION_VERSION_MAJOR;
}

int 
App::getProjectMinorVersion()
{
    return APPLICATION_VERSION_MINOR;
}

int 
App::getProjectPatchVersion()
{
    return APPLICATION_VERSION_PATCH;
}

QString 
App::getProjectVersion()
{
    return APPLICATION_VERSION_STRING;
}

QString 
App::getProjectCopyrightYears()
{
    return APPLICATION_COPYRIGHT_YEARS;
}

QString
App::getProjectInvocation()
{
    return _invocation;
}

std::string
App::asKey(const std::string& key)const
{
    std::string result(key);
    for ( size_t i = 0; i < result.size(); i++ ){
        if ( (result[i]=='/') || (result[i]=='\\') ){
            result[i] = '.';
        }
    }
    return result;
}

void 
App::setPreference(const std::string& key, const std::string& val)
{
    QSettings settings;
    std::string newkey = asKey(key);
    size_t indexofprevdot = 0;
    size_t indexofdot     = newkey.find('.');
    int groups = 0;
    while ( indexofdot != std::string::npos ){
        groups++;
        std::string nextgroup = newkey.substr(indexofprevdot,indexofdot-indexofprevdot);
        settings.beginGroup(QString(nextgroup.c_str()));
        indexofprevdot = indexofdot+1;
        indexofdot = newkey.find('.',indexofprevdot);
    }
    
    std::string actualkey = newkey.substr(indexofprevdot);
    settings.setValue(QString(actualkey.c_str()),QString(val.c_str()));
    settings.sync();
    
    for ( int i = 0; i< groups; i++ ){
        settings.endGroup();
    }
}

void 
App::unsetPreference(const std::string& key)
{
    QSettings settings;
    std::string newkey = asKey(key);
    size_t indexofprevdot = 0;
    size_t indexofdot     = newkey.find('.');
    int groups = 0;
    while ( indexofdot != std::string::npos ){
        groups++;
        std::string nextgroup = newkey.substr(indexofprevdot,indexofdot-indexofprevdot);
        settings.beginGroup(QString(nextgroup.c_str()));
        indexofprevdot = indexofdot+1;
        indexofdot = newkey.find('.',indexofprevdot);
    }
    
    std::string actualkey = newkey.substr(indexofprevdot);
    settings.remove(QString(actualkey.c_str()));
    
    for ( int i = 0; i< groups; i++ ){
        settings.endGroup();
    }
}

void 
App::printPreference(const std::string& key)const
{
    QSettings settings;
    std::string newkey = asKey(key);
    size_t indexofprevdot = 0;
    size_t indexofdot     = newkey.find('.');
    int groups = 0;
    while ( indexofdot != std::string::npos ){
        groups++;
        std::string nextgroup = newkey.substr(indexofprevdot,indexofdot-indexofprevdot);
        settings.beginGroup(QString(nextgroup.c_str()));
        indexofprevdot = indexofdot+1;
        indexofdot = newkey.find('.',indexofprevdot);
    }
    
    std::string actualkey = newkey.substr(indexofprevdot);
    QString keystring(actualkey.c_str());
    
    if ( settings.contains(keystring) ){
        std::cout << settings.value(keystring).toString() << std::endl;
    }else{
        std::cout << "undefined" << std::endl;
    }
    
    
    for ( int i = 0; i< groups; i++ ){
        settings.endGroup();
    }
}

void 
App::printAllPreferences()const
{
    QSettings settings;
    QStringList keys = settings.allKeys();
    for ( QStringList::const_iterator it = keys.begin(); it != keys.end(); ++it ){
        QString qkeystr = *it;
        QByteArray qkeystrdata = qkeystr.toUtf8();
        std::string keystr(qkeystrdata.constData());
        std::string key = asKey(keystr);
        std::cout << key << "=" << settings.value(qkeystr).toString() << std::endl;
    }
}

void 
App::setLogLevel(const std::string& logger, const std::string& level)
{
    log4cxx::LoggerPtr loggerptr = ((logger=="")?(log4cxx::Logger::getRootLogger()):(log4cxx::Logger::getLogger(logger)));
    std::string lowercaselevel(level);
    for ( size_t i = 0; i < lowercaselevel.size(); i++ ){
        lowercaselevel[i] = std::tolower(lowercaselevel[i]);
    }
    
    if ( lowercaselevel == "all" ){
        loggerptr->setLevel(log4cxx::Level::getAll());
    }else if ( lowercaselevel == "trace" ){
        loggerptr->setLevel(log4cxx::Level::getTrace());
    }else if ( lowercaselevel == "debug" ){
        loggerptr->setLevel(log4cxx::Level::getDebug());
    }else if ( lowercaselevel == "info" ){
        loggerptr->setLevel(log4cxx::Level::getInfo());
    }else if ( lowercaselevel == "warn" ){
        loggerptr->setLevel(log4cxx::Level::getWarn());
    }else if ( lowercaselevel == "error" ){
        loggerptr->setLevel(log4cxx::Level::getError());
    }else if ( lowercaselevel == "fatal" ){
        loggerptr->setLevel(log4cxx::Level::getFatal());
    }else if ( (lowercaselevel == "off")  || (lowercaselevel == "none") ){
        loggerptr->setLevel(log4cxx::Level::getOff());
    }else{
        LOG4CXX_FATAL(_logger,"Unrecognized logging level: \"" << level << "\".");
        std::exit(1);
    }
}

App*
App::_instance = 0;

log4cxx::LoggerPtr
App::_logger = log4cxx::Logger::getLogger("App");
