/**
 *  This program creates an SVG (Scalable Vector Graphics) symbol out of
 *  a VHDL entity
 *
 *  Copyright (C) 2019  Frans Schreuder info@schreuderelectronics.com
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
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "entityblock.h"
#include <QApplication>
#include <QFile>
#include <QCommandLineParser>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    QCoreApplication::setApplicationName("entity-block");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("Reads a vhdl file and outputs a .svg file with the entity block\n(All command line options will be stored)");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("input", "VHDL file to convert");
    parser.addPositionalArgument("output", "SVG file to output");

    QCommandLineOption commentColorOption(QStringList() << "c" << "comment-color",
            "Change default comment color to <color>",
            "color");
    QCommandLineOption portNameColorOption(QStringList() << "n" << "port-name-color",
            "Change default port name color to <color>",
            "color");
    QCommandLineOption portTypeColorOption(QStringList() << "t" << "port-type-color",
            "Change default port type color to <color>",
            "color");
    QCommandLineOption backgroundColorOption(QStringList() << "b" << "background-color",
            "Change default background color to <color>",
            "color");
    QCommandLineOption headerLeftColorOption(QStringList() << "l" << "header-left-color",
            "Change default left (gradient) in the header color to <color>",
            "color");
    QCommandLineOption headerRightColorOption(QStringList() << "r" << "header-right-color",
            "Change default right (gradient) in the header color to <color>",
            "color");
    QCommandLineOption titleColorOption(QStringList() << "e" << "entity-title-color",
            "Change default entity color to <color>",
            "color");
    QCommandLineOption borderColorOption(QStringList() << "B" << "border-color",
            "Change default border color to <color>",
            "color");
    QCommandLineOption portsColorOption(QStringList() << "p" << "port-color",
            "Change default port symbol color to <color>",
            "color");
    QCommandLineOption shadowColorOption(QStringList() << "s" << "shadow-color",
            "Change default shadow color to <color>",
            "color");

    QCommandLineOption cornerRadiusOption(QStringList() << "R" << "corner-radius",
            "Change default corner radius to <number>",
            "number");

    QCommandLineOption borderWidthOption(QStringList() << "w" << "line-weight",
            "Change default line thickness to <number>",
            "number");

    QCommandLineOption simplifiedSymbol(QStringList() << "S" << "symplified-symbol",
            "Generate a symbol without types, comments and generics");

    parser.addOption(commentColorOption);
    parser.addOption(portNameColorOption);
    parser.addOption(portTypeColorOption);
    parser.addOption(backgroundColorOption);
    parser.addOption(headerLeftColorOption);
    parser.addOption(headerRightColorOption);
    parser.addOption(titleColorOption);
    parser.addOption(borderColorOption);
    parser.addOption(portsColorOption);
    parser.addOption(cornerRadiusOption);
    parser.addOption(shadowColorOption);
    parser.addOption(borderWidthOption);
    parser.addOption(simplifiedSymbol);

    // Process the actual command line arguments given by the user
    parser.process(a);


    const QStringList args = parser.positionalArguments();
    QString fileName;
    QString outputName;
    if(args.size()<1||args.size()>2)
    {
        parser.showHelp();
        return 1;
    }
    else
    {
        fileName = args[0];
        if(args.size()>1)
            outputName = args[1];
    }


    QSettings *settings = new QSettings("Schreuder Electronics","entity-block");
    if(parser.isSet(commentColorOption))
    {
        QColor c(parser.value(commentColorOption));
        settings->setValue("Colors/comment",c);
    }
    if(parser.isSet(portNameColorOption))
    {
        QColor c(parser.value(portNameColorOption));
        settings->setValue("Colors/portName",c);
    }
    if(parser.isSet(portTypeColorOption))
    {
        QColor c(parser.value(portTypeColorOption));
        settings->setValue("Colors/portType",c);
    }
    if(parser.isSet(backgroundColorOption))
    {
        QColor c(parser.value(backgroundColorOption));
        settings->setValue("Colors/background",c);
    }
    if(parser.isSet(headerLeftColorOption))
    {
        QColor c(parser.value(headerLeftColorOption));
        settings->setValue("Colors/headerLeft",c);
    }
    if(parser.isSet(headerRightColorOption))
    {
        QColor c(parser.value(headerRightColorOption));
        settings->setValue("Colors/headerRight",c);
    }
    if(parser.isSet(titleColorOption))
    {
        QColor c(parser.value(titleColorOption));
        settings->setValue("Colors/title",c);
    }
    if(parser.isSet(borderColorOption))
    {
        QColor c(parser.value(borderColorOption));
        settings->setValue("Colors/border",c);
    }
    if(parser.isSet(portsColorOption))
    {
        QColor c(parser.value(portsColorOption));
        settings->setValue("Colors/port",c);
    }
    if(parser.isSet(shadowColorOption))
    {
        QColor c(parser.value(shadowColorOption));
        settings->setValue("Colors/shadow",c);
    }

    if(parser.isSet(cornerRadiusOption))
    {
        bool ok;
        int c = parser.value(cornerRadiusOption).toInt(&ok);
        if(!ok) c = 10;
        else if(c < 0) c*=-1;
        settings->setValue("Dimensions/cornerRadius",c);
    }
    if(parser.isSet(borderWidthOption))
    {
        bool ok;
        int c = parser.value(borderWidthOption).toInt(&ok);
        if(!ok) c = 2;
        settings->setValue("Dimensions/borderWidth",c);
    }

    EntityBlock w(fileName,outputName, settings, parser.isSet(simplifiedSymbol));
    delete settings;

    return w.success?0:1;
}
