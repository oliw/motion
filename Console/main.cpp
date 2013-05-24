#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QTextStream>
#include "coreapplication.h"
#include <QFileInfo>
#include <QTimer>
#include <iostream>
#include <videoprocessor.h>
#include "mainapplication.h"
#include "boost/program_options.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/positional_options.hpp"
#include "boost/program_options/variables_map.hpp"

namespace po = boost::program_options;

bool verbose = false;

void debug_output(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (verbose) {
        switch (type) {
        case QtDebugMsg:
            std::cerr << msg.toStdString() << std::endl;
            break;
        case QtWarningMsg:
            std::cout << msg.toStdString() << std::endl;
            break;
        case QtFatalMsg:
            std::cerr << msg.toStdString() << std::endl;
            break;
        default:
            std::cout << msg.toStdString() << std::endl;
            break;
        }
    } else {
        switch (type) {
        case QtDebugMsg:
            break;
        case QtWarningMsg:
            std::cout << msg.toStdString() << std::endl;
            break;
        case QtFatalMsg:
            std::cerr << msg.toStdString() << std::endl;
            break;
        default:
            break;
        }
    }
}

void printHelp(po::options_description& desc) {
    std::stringstream ss;
    ss << desc;
    qDebug() << QString::fromStdString(ss.str());
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(debug_output);
    QCoreApplication a(argc, argv);
    string inputPath;
    string outputPath;
    string cropSize;
    string fdMethod;
    bool salient = false;
    string manualFeatureFilePath;
    bool gravitate = false;
    QRect cropWindow;
    bool dumpData;

    ///////// PROGRAM OPTIONS /////////
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("input-video,I", po::value<string>(&inputPath)->implicit_value(""),"input video")
            ("output-video,O", po::value<string>(&outputPath)->implicit_value(""),"output video")
            ("crop-tl,C", po::value<string>()->implicit_value(""),"coordinates for top left corner of crop window")
            ("crop-size,S", po::value<string>(&cropSize)->implicit_value(""),"cropped window size")
            ("feature-detector,F", po::value<string>(&fdMethod)->implicit_value(""),"feature detection method")  // TODO Permissable feature detector?
            ("salient-path-tracking", po::value<bool>(&salient)->zero_tokens(),"enable salient feasture tracking")
            ("manual-features,M", po::value<string>(&manualFeatureFilePath)->implicit_value(""), "file containing manually tracked salient features")
            ("gravitate-to-center,G", po::value<bool>(&gravitate)->zero_tokens(),"gravitate salient point to middle")
            ("dump-data,D", po::value<bool>(&dumpData)->zero_tokens(),"save .mat files in same folder as output video")
            ("verbose,v", po::value<bool>(&verbose)->zero_tokens(),"show all debug messages");

    po::positional_options_description p;
    p.add("input-video", 1);
    p.add("output-video",1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        printHelp(desc);
        return 1;
    }

    // Process Input Video
    if (vm.count("input-video") != 1) {
        qCritical() << "No input video given";
        printHelp(desc);
        return 1;
    }
    inputPath = vm["input-video"].as<string>();

    // Process Output Video
    if (vm.count("output-video")==1) {
        outputPath = vm["output-video"].as<string>();
    } else {
        QFileInfo fileInfo = QFileInfo(QString::fromStdString(inputPath));
        QString defaultPath = fileInfo.path() + fileInfo.baseName() + "_output.avi";
        outputPath = defaultPath.toStdString();
    }

    // Process crop-size
    QStringList sizeList;
    if (vm.count("crop-size")) {
        QString size = QString::fromStdString(vm["crop-size"].as<string>());
        sizeList = size.split(",");
        if (sizeList.size() != 2) {
            qCritical() << "Invalid crop-size given. Should be w,h";
            printHelp(desc);
            return 1;
        }
    } else {
        qCritical() << "No crop window size given.";
        printHelp(desc);
        return 1;
    }

    // Process crop-center
    QStringList cropTl;
    if (vm.count("crop-tl")) {
        QString corner = QString::fromStdString(vm["crop-tl"].as<string>());
        cropTl = corner.split(",");
        if (cropTl.size() != 2) {
            qCritical() << "Invalid crop-tl given. Should be x,y";
            printHelp(desc);
            return 1;
        }
    } else {
        qCritical() << "No crop top left coords given.";
        printHelp(desc);
        return 1;
    }

    cropWindow = QRect(cropTl.at(0).toInt(), cropTl.at(1).toInt(), sizeList.at(0).toInt(), sizeList.at(1).toInt());

    if (salient) {
        if (!vm.count("manual-features")) {
            qCritical() << "No file for manual features given";
            printHelp(desc);
            return 1;
        }
    }

    qWarning() << "Starting core application";
    MainApplication* main = new MainApplication(QString::fromStdString(inputPath), QString::fromStdString(outputPath), cropWindow, salient, QString::fromStdString(manualFeatureFilePath),gravitate,dumpData, &a);
    QObject::connect(main, SIGNAL(quit()), &a, SLOT(quit()));
    QTimer::singleShot(0, main, SLOT(run()));
    return a.exec();
}
