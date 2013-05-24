#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QTextStream>
#include "coreapplication.h"
#include <QFileInfo>
#include <iostream>
#include <videoprocessor.h>
#include "boost/program_options.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/positional_options.hpp"
#include "boost/program_options/variables_map.hpp"

namespace po = boost::program_options;

void printHelp(po::options_description& desc) {
    std::stringstream ss;
    ss << desc;
    qDebug() << QString::fromStdString(ss.str());
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString inputPath;
    QString outputPath;
    int salientPathTracking;
    QString manualFeatureFilePath;
    int gravitateToCenter;
    QRect cropWindow;

    ///////// PROGRAM OPTIONS /////////
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("input-video,I", po::value<string>(),"input video")
            ("output-video,O", po::value<string>(),"output video")
            ("crop-tl,C", po::value<string>(),"coordinates for top left corner of crop window")
            ("crop-size,S", po::value<string>(),"cropped window size")
            ("feature-detector,F", po::value<string>(),"feature detection method")  // TODO Permissable feature detector?
            ("salient-path-tracking", po::value<int>(&salientPathTracking)->default_value(0));
            ("manual-features,M", po::value<string>(), "file containing manually tracked salient features")
            ("gravitate-to-center,G", po::value<int>(&gravitateToCenter)->default_value(1),"gravitate salient point to middle");

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
        qDebug() << "No input video given";
        printHelp(desc);
        return 1;
    }
    inputPath = QString::fromStdString(vm['input-video'].as<string>());

    // Process Output Video
    if (vm.count("output-video")==1) {
        outputPath = QString::fromStdString(vm['output-video'].as<string>());
    } else {
        QFileInfo fileInfo = QFileInfo(inputPath);
        outputPath = fileInfo.path() + fileInfo.baseName() + "_output.avi";
    }

    // Process crop-size
    QStringList sizeList;
    if (vm.count("crop-size")) {
        QString size = QString::fromStdString(vm['crop-size'].as<string>());
        sizeList = size.split("\\,");
        if (sizeList.size() != 2) {
            qDebug() << "Invalid crop-size given. Should be w,h";
            printHelp(desc);
            return 1;
        }
    } else {
        qDebug() << "No crop window size given.";
        printHelp(desc);
        return 1;
    }

    // Process crop-center
    QStringList cropTl;
    if (vm.count("crop-tl")) {
        QString corner = QString::fromStdString(vm['crop-tl'].as<string>());
        cropTl = corner.split("\\,");
        if (cropTl.size() != 2) {
            qDebug() << "Invalid crop-tl given. Should be x,y";
            printHelp(desc);
            return 1;
        }
    } else {
        qDebug() << "No crop top left coords given.";
        printHelp(desc);
        return 1;
    }

    cropWindow = QRect(QString::toInt(cropTl.at(0)), QString::toInt(cropTl.at(1)), QString::toInt(sizeList.at(0)), QString::toInt(sizeList.at(1)));

    if (salientPathTracking) {
        QString manualFeatureFilePath;
        if (vm.count("manual-features")) {
            manualFeatureFilePath = QString::fromStdString(vm['manual-features'].as<string>());
        } else {
            qDebug() << "No file for manual features given";
            printHelp(desc);
            return 1;
        }
        bool gravitate = gravitateToCenter == 1;
    } else {
        // Activate main with just
    }

   // MainApplication main(a.arguments());
    //main.begin();
    return a.exec();
}
