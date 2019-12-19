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
#include <stdio.h>
#include <QDebug>
#include <QtSvg/QSvgGenerator>

Port::Port()
{
    direction = in;
}

EntityBlock::EntityBlock(QString fileName, QString targetName, QSettings* s)
{
    settings = s;
    cComment = settings->value("Colors/comment",QColor(Qt::darkGreen)).value<QColor>();
    cPortName = settings->value("Colors/portName",QColor(Qt::black)).value<QColor>();
    cPortType = settings->value("Colors/portType",QColor(Qt::darkBlue)).value<QColor>();
    cBackground = settings->value("Colors/background",QColor("#fffff3")).value<QColor>();
    cHeader1 = settings->value("Colors/headerLeft",QColor("#014040")).value<QColor>();
    cHeader2 = settings->value("Colors/headerRight",QColor("#7f7f7f")).value<QColor>();
    cTitle = settings->value("Colors/title",QColor(Qt::white)).value<QColor>();
    cBorder = settings->value("Colors/border",QColor("#235676")).value<QColor>();
    cPorts = settings->value("Colors/port",QColor("#235676")).value<QColor>();
    cShadow = settings->value("Colors/shadow",QColor(Qt::darkGray)).value<QColor>();
    cornerRadius = settings->value("Dimensions/cornerRadius",int(10)).value<int>();
    borderWidth = settings->value("Dimensions/borderWidth",int(2)).value<int>();
    spacing = 10;
    success = false;
    if(fileName != "")
    {
        success = loadFile(fileName);
        saveSvg(targetName);
    }

}

EntityBlock::~EntityBlock()
{



}

bool EntityBlock::loadFile(QString fileName)
{
    QFile file(fileName);
    bool success = file.open(QFile::ReadOnly);
    if(!success){
        success = false;
        return false;
    }
    QString entityString;
    bool entityBusy = false;
    bool entityFound = false;
    while(!file.atEnd())
    {
        QString line = QString(file.readLine());
        line = line.simplified(); //strip whitespace
        if(line.toLower().startsWith("use"))
        {
            libraries.push_back(line);
        }
        if(searchNoComments(line.toLower(),"entity")!=-1&&!entityFound)
        {
            entityString = line;
            entityBusy = true;
            entityFound = true;
        }
        if(entityBusy)
        {
            entityString.append("\n"+line);
            if(searchNoComments(entityString.toLower(),"is")!=-1 && entityName.length()==0) //determine the name of the entity
            {
                entityName = entityString.split("entity", QString::KeepEmptyParts, Qt::CaseInsensitive)[1];
                entityName = entityName.split("is", QString::KeepEmptyParts, Qt::CaseInsensitive)[0].simplified();
                //printf("Entity Name: \"%s\"\n", entityName.toLocal8Bit().data());
            }
            if(line.simplified().toLower().startsWith("end ")&&line.contains(";"))
            {
                entityBusy = false;
                parseEntityString(entityString);
            }
        }


    }
    success = true;
    return true;
}

int EntityBlock::searchNoComments(QString string, QString seed)
{
    QStringList sComments = string.split("\n");
    QStringList sNoComments = string.split("\n");
    int index=0;
    for(int i=0; i< sNoComments.size(); i++)
    {
        if(sNoComments[i].contains("--"))
        {
            sNoComments[i] = sNoComments[i].mid(0,sNoComments[i].indexOf("--"));
        }
        int n = sNoComments[i].indexOf(seed);
        if(n>=0)
            return index+n;
        index += sComments[i].length()+1; //+1 for the newline
    }
    return -1;

}

int EntityBlock::searchCloseBracket(QString string)
{
    QStringList sComments = string.split("\n");
    QStringList sNoComments = string.split("\n");
    int openBrackets = 0;
    int index=0;
    for(int i=0; i< sNoComments.size(); i++)
    {
        if(sNoComments[i].contains("--"))
        {
            sNoComments[i] = sNoComments[i].mid(0,sNoComments[i].indexOf("--"));
        }
        int open = -1;
        while(true)
        {
            open = sNoComments[i].indexOf("(",open+1);
            if(open>=0)openBrackets++;
            else break; //no more open brackets ( on this line.
        }
        open=-1;
        while(true)
        {
            open = sNoComments[i].indexOf(")",open+1);
            if(open>=0)openBrackets--;
            if(openBrackets<0)
                return index + open;
            if(open<0)break; //no more close brackets ) on this line.
        }

        index += sComments[i].length()+1;//+1 for the newline
    }
    return -1;
}

void EntityBlock::parseEntityString(QString entityString)
{
    ports.clear();
    generics.clear();
    if(searchNoComments(entityString.toLower(),"generic")!=-1)
    {
        QStringList comments;
        QString portString;
        QStringList lines;
        portString = entityString.mid(searchNoComments(entityString.toLower(), "generic"),-1); //Take the whole string from the keyword generic
        portString = portString.mid(searchNoComments(portString,"(")+1,-1);
        portString = portString.mid(0,searchCloseBracket(portString));

        lines = portString.split("\n", QString::SkipEmptyParts);
        int PortsStripped = 0;
        for(int i=0; i<lines.size(); i++) //First strip the comments and store them in comments
        {
            if(lines[i].contains("--"))
            {
                QString commentLine = lines[i].mid(lines[i].indexOf("--")+2, -1);
                lines[i] = lines[i].mid(0,lines[i].indexOf("--"));
                int cntPorts = lines[i].count(";");
                if(i==lines.size()-1&&lines[i].simplified()!="")cntPorts++;
                while(comments.size()<PortsStripped+cntPorts)
                    comments.push_back("");
                if(cntPorts==0)
                {
                    comments.push_back("");
                    comments[PortsStripped].append(commentLine);
                }
                else
                    comments[PortsStripped+cntPorts-1].append(commentLine);
                PortsStripped += cntPorts;

            }
            else
            {
                int cntPorts = lines[i].count(";");
                if(i==lines.size()-1&&lines[i].simplified()!="")cntPorts++;
                PortsStripped += cntPorts;
            }
        }
        while(comments.size()<PortsStripped)
            comments.push_back("");

        portString = lines.join("").simplified();
        QStringList portStrings = portString.split(";");
        for(int i=0; i<portStrings.size(); i++)
        {
            Port port;
            port.comment=comments[i].simplified();
            int colonSep = portStrings[i].indexOf(":");
            int defSep = portStrings[i].indexOf(":=");
            port.name=portStrings[i].mid(0,colonSep).simplified();
            if(defSep != -1)
            {
                port.type = portStrings[i].mid(colonSep+1, defSep-(colonSep+1)).simplified();
                port.def = portStrings[i].mid(defSep+2, -1).simplified();
            }
            else
                port.type = portStrings[i].mid(colonSep+1, defSep).simplified();
            port.direction = in; //Generics don't have a direction

            generics.push_back(port);
        }
    }

    if(searchNoComments(entityString.toLower(),"port")!=-1)
    {
        QStringList comments;
        QString portString;
        QStringList lines;
        portString = entityString.mid(searchNoComments(entityString.toLower(), "port"),-1); //Take the whole string from the keyword generic
        portString = portString.mid(searchNoComments(portString,"(")+1,-1);
        portString = portString.mid(0,searchCloseBracket(portString));
        lines = portString.split("\n", QString::SkipEmptyParts);
        int PortsStripped = 0;
        for(int i=0; i<lines.size(); i++) //First strip the comments and store them in comments
        {
            if(lines[i].contains("--"))
            {
                QString commentLine = lines[i].mid(lines[i].indexOf("--")+2, -1);
                lines[i] = lines[i].mid(0,lines[i].indexOf("--"));
                int cntPorts = lines[i].count(";");
                if(i==lines.size()-1&&lines[i].simplified()!="")cntPorts++;
                while(comments.size()<PortsStripped+cntPorts)
                    comments.push_back("");
                if(cntPorts==0)
                {
                    comments.push_back("");
                    comments[PortsStripped].append(commentLine);
                }
                else
                    comments[PortsStripped+cntPorts-1].append(commentLine);
                PortsStripped += cntPorts;

            }
            else
            {
                int cntPorts = lines[i].count(";");
                if(i==lines.size()-1&&lines[i].simplified()!="")cntPorts++;
                PortsStripped += cntPorts;
            }
        }
        while(comments.size()<PortsStripped)
            comments.push_back("");

        portString = lines.join("").simplified();
        QStringList portStrings = portString.split(";");
        for(int i=0; i<portStrings.size(); i++)
        {
            Port port;
            port.comment=comments[i].simplified();
            int colonSep = portStrings[i].indexOf(":");
            int defSep = portStrings[i].indexOf(":=");
            port.name=portStrings[i].mid(0,colonSep).simplified();
            if(port.name.startsWith("signal ", Qt::CaseInsensitive)) //port name may be preceded by signal, just strip that off
            {
                port.name = port.name.mid(QString("signal ").length());
            }
            if(defSep != -1)
            {
                port.type = portStrings[i].mid(colonSep+1, defSep-(colonSep+1)).simplified();
                port.def = portStrings[i].mid(defSep+2, -1).simplified();
            }
            else
                port.type = portStrings[i].mid(colonSep+1, defSep).simplified();
            if(port.type.contains(" "))
            {
                QString dirStr = port.type.mid(0, port.type.indexOf(" ")).simplified().toLower();
                int typeIndex = 0;
                if(dirStr=="in")
                {
                    port.direction = in;
                    typeIndex = 3;
                }
                if(dirStr=="out")
                {
                    port.direction = out;
                    typeIndex = 4;
                }
                if(dirStr=="inout")
                {
                    port.direction = inout;
                    typeIndex = 6;
                }
                if(dirStr=="buffer")
                {
                    port.direction = buffer;
                    typeIndex = 7;
                }
                if(dirStr=="linkage")
                {
                    port.direction = linkage;
                    typeIndex = 8;
                }
                port.type = port.type.mid(typeIndex).simplified();
            }
            else //no type specified, default to in
                port.direction = in; //Generics don't have a direction

            ports.push_back(port);
        }
    }

}

void EntityBlock::paintPortSymbol(QPainter& painter, direction_t direction, int x, int y, bool mirror)
{
    QPen pen=painter.pen();
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setColor(cPorts);
    pen.setWidth(borderWidth);
    painter.setPen(pen);
    if(direction==in)
    {
        QPainterPath p;
        p.moveTo(x-(5*(mirror?-1:1)), y-5);
        p.lineTo(x-(5*(mirror?-1:1)), y+5);
        p.lineTo(x+(5*(mirror?-1:1)),y);
        p.lineTo(x-(5*(mirror?-1:1)), y-5);
        painter.drawPath(p);
        painter.fillPath(p,QBrush(cPorts));
    }
    if(direction==out)
    {
        QPainterPath p;
        p.moveTo(x+(5*(mirror?-1:1)), y-5);
        p.lineTo(x+(5*(mirror?-1:1)), y+5);
        p.lineTo(x-(5*(mirror?-1:1)),y);
        p.lineTo(x+(5*(mirror?-1:1)), y-5);
        painter.drawPath(p);
        painter.fillPath(p,QBrush(cPorts));
    }
    if(direction==inout)
    {
        QPainterPath p;
        p.moveTo(x+5, y);
        p.lineTo(x, y+5);
        p.lineTo(x-5,y);
        p.lineTo(x, y-5);
        p.lineTo(x+5, y);
        painter.drawPath(p);
        painter.fillPath(p,QBrush(cPorts));
    }
    if(direction==buffer||direction==linkage)
    {
        QPainterPath p;
        p.moveTo(x+5, y+5);
        p.lineTo(x-5, y+5);
        p.lineTo(x-5, y-5);
        p.lineTo(x+5, y-5);
        p.lineTo(x+5, y+5);
        painter.drawPath(p);
        painter.fillPath(p,QBrush(cPorts));
    }


}

void EntityBlock::paint(QPainter &painter)
{
    QVector<Port> inputPorts, outputPorts, clockPorts, resetPorts;

    //Divide the ports in 4 groups. input, clock and reset are on the left, but grouped together. Output ports on the right.
    for(int i=0; i<ports.size(); i++)
    {
        if(ports[i].name.startsWith("s_axi")||
           ports[i].name.contains("slave_")
                )
            inputPorts.push_back(ports[i]);
        else if(ports[i].name.startsWith("m_axi")||
            ports[i].name.contains("master_"))
            outputPorts.push_back(ports[i]);
        else if(ports[i].direction==in) //in and linkage go left, but clock and reset on the bottom left.
        {
            if(ports[i].name.contains("clk", Qt::CaseInsensitive)||
               ports[i].name.contains("clock", Qt::CaseInsensitive))
                clockPorts.push_back(ports[i]);
            else if (ports[i].name.contains("rst", Qt::CaseInsensitive)||
                     ports[i].name.contains("reset", Qt::CaseInsensitive))
                resetPorts.push_back(ports[i]);
            else
                inputPorts.push_back(ports[i]);
        }
        else if (ports[i].direction==linkage) {
            inputPorts.push_back(ports[i]);
        }
        else //inout, out and buffer go on the right.
            outputPorts.push_back(ports[i]);

    }

    QFont nameFont = painter.font();
    nameFont.setPointSize(10);
    QFont commentFont = painter.font();
    commentFont.setPointSize(8);
    QFont titleFont = painter.font();
    titleFont.setPointSize(12);

    QPen namePen = painter.pen();
    namePen.setColor(cPortName);
    QPen commentPen = painter.pen();
    commentPen.setColor(cComment);
    QPen typePen = painter.pen();
    typePen.setColor(cPortType);
    QPen titlePen = painter.pen();
    titlePen.setColor(cTitle);
    QPen rectPen = painter.pen();
    rectPen.setColor(cBorder);
    rectPen.setWidth(borderWidth);
    rectPen.setCapStyle(Qt::RoundCap);
    rectPen.setJoinStyle(Qt::RoundJoin);
    QBrush rectBrush = painter.brush();
    rectBrush.setStyle(Qt::SolidPattern);
    rectBrush.setColor(cBackground);

    int portH=0;
    int nameH=0;
    int leftOuter=0;
    int leftInner=0;
    int rightInner=0;
    int rightOuter=0;
    int genericWidth=0;

    //Determine maximum width and height of generic labels
    for(int i=0; i<generics.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        QString gText = generics[i].name + " : " + generics[i].type;
        if(generics[i].def!="")
            gText += " := " + generics[i].def;

        QRect nameRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignLeft, gText);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        QRect commentRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignLeft, generics[i].comment);
        if(nameRect.height()+commentRect.height() > portH)
            portH = nameRect.height()+commentRect.height();
        if(nameRect.width()>genericWidth)
            genericWidth = nameRect.width();
        if(commentRect.width()>genericWidth)
            genericWidth = commentRect.width();
    }
    //Determine maximum width and height of input port labels
    for(int i=0; i<inputPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        QRect nameRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, inputPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        QRect commentRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, inputPorts[i].comment);
        QString typeString = inputPorts[i].type;
        if(inputPorts[i].def!="")typeString += " ("+inputPorts[i].def+")";
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QRect typeRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignLeft, typeString);
        if(nameRect.height()+commentRect.height() > portH)
            portH = nameRect.height()+commentRect.height();
        if(nameRect.height() > nameH)
            nameH = nameRect.height();
        if(nameRect.width()>leftOuter)
            leftOuter = nameRect.width();
        if(commentRect.width()>leftInner)
            leftInner = commentRect.width();
        if(typeRect.width()>leftInner)
            leftInner = typeRect.width();

    }

    //Determine maximum width and height of reset port labels
    for(int i=0; i<resetPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        QRect nameRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, resetPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        QRect commentRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, resetPorts[i].comment);
        QString typeString = resetPorts[i].type;
        if(resetPorts[i].def!="")typeString += " ("+resetPorts[i].def+")";
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QRect typeRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignLeft, typeString);
        if(nameRect.height()+commentRect.height() > portH)
            portH = nameRect.height()+commentRect.height();
        if(nameRect.height() > nameH)
            nameH = nameRect.height();
        if(nameRect.width()>leftOuter)
            leftOuter = nameRect.width();
        if(commentRect.width()>leftInner)
            leftInner = commentRect.width();
        if(typeRect.width()>leftInner)
            leftInner = typeRect.width();

    }

    //Determine maximum width and height of clock port labels
    for(int i=0; i<clockPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        QRect nameRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, clockPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        QRect commentRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, clockPorts[i].comment);
        QString typeString = clockPorts[i].type;
        if(clockPorts[i].def!="")typeString += " ("+clockPorts[i].def+")";
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QRect typeRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignLeft, typeString);
        if(nameRect.height()+commentRect.height() > portH)
            portH = nameRect.height()+commentRect.height();
        if(nameRect.height() > nameH)
            nameH = nameRect.height();
        if(nameRect.width()>leftOuter)
            leftOuter = nameRect.width();
        if(commentRect.width()>leftInner)
            leftInner = commentRect.width();
        if(typeRect.width()>leftInner)
            leftInner = typeRect.width();

    }

    //Determine maximum width and height of output port labels
    for(int i=0; i<outputPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        QRect nameRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, outputPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        QRect commentRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignRight, outputPorts[i].comment);
        QString typeString = outputPorts[i].type;
        if(outputPorts[i].def!="")typeString += " ("+outputPorts[i].def+")";
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QRect typeRect = painter.boundingRect(0, 0, 2000, 20, Qt::AlignLeft, typeString);
        if(nameRect.height()+commentRect.height() > portH)
            portH = nameRect.height()+commentRect.height();
        if(nameRect.height() > nameH)
            nameH = nameRect.height();
        if(nameRect.width()>rightOuter)
            rightOuter = nameRect.width();
        if(commentRect.width()>rightInner)
            rightInner= commentRect.width();
        if(typeRect.width()>rightInner)
            rightInner = typeRect.width();

    }

    painter.setPen(titlePen);
    painter.setFont(titleFont);
    //determine size of the title block.
    QRect titleRect = painter.boundingRect(0,0,2000,20, Qt::AlignHCenter, entityName);
    //Check whether we have more ports on the left or right side and adjust the height of the rectangle / image
    int leftCount = inputPorts.size() +
            resetPorts.size()+(((resetPorts.size()>0)&&(inputPorts.size()>0))?1:0) +
            clockPorts.size()+(((clockPorts.size()>0)&&(inputPorts.size()>0||resetPorts.size()>0))?1:0);
    if(titleRect.height()<cornerRadius)titleRect.setHeight(cornerRadius);

    imageHeight = (leftCount > outputPorts.size()? leftCount:outputPorts.size())*portH + (2*titleRect.height());
    int rectWidth = (titleRect.width()+(4*spacing)) > (leftInner+rightInner+(6*spacing))?titleRect.width()+(4*spacing): (leftInner+rightInner+(6*spacing));
    if(genericWidth+(4*spacing)>rectWidth)rectWidth = genericWidth+(4*spacing);
    imageHeight += generics.size()*portH;

    imageWidth = leftOuter+(2*spacing) + rectWidth + rightOuter;


    //draw the half rounded rectangle around the title
    painter.setPen(rectPen);
    QLinearGradient lg(leftOuter+(spacing), 0, leftOuter+(spacing)+rectWidth, 0);
    lg.setColorAt(0, cHeader1);
    lg.setColorAt(1, cHeader2);
    QPainterPath p1, p2, p3;
    p1.moveTo(leftOuter+spacing+cornerRadius,0); //Move cursor to left, but right of the top-left arc
    p1.arcTo(leftOuter+(1*spacing),0,(2*cornerRadius),(2*cornerRadius),90, 90); //draw arc to the left side
    p1.lineTo(leftOuter+(1*spacing),titleRect.height()); //draw left line of header (down)
    p1.lineTo(leftOuter+(1*spacing)+rectWidth,titleRect.height()); //draw bottom line of header (right)
    p1.lineTo(leftOuter+(1*spacing)+rectWidth,(1*cornerRadius)); //right line (up)
    p1.arcTo(leftOuter+(1*spacing)+rectWidth-(2*cornerRadius),0,(2*cornerRadius),(2*cornerRadius),0*16, 90); //draw arc on the right side
    p1.lineTo(leftOuter+spacing+cornerRadius,0); //back to start point

    //Contour of the whole entity rectangle
    p2.addRoundedRect(leftOuter+(1*spacing),0,rectWidth, imageHeight,(1*cornerRadius),(1*cornerRadius));
    //shadow
    p3.addRoundedRect(leftOuter+(1*spacing)+5,3,rectWidth, imageHeight,(1*cornerRadius),(1*cornerRadius));

    QBrush shadowBrush(cShadow);
    painter.fillPath(p3,shadowBrush);
    painter.fillPath(p2,rectBrush);
    painter.drawPath(p2);
    painter.fillPath(p1,lg);
    painter.drawPath(p1);

    //Draw a line between ports and generics.
    if(generics.size()>0)
    {
        painter.drawLine(leftOuter+spacing, imageHeight-titleRect.height()-generics.size()*portH, leftOuter+spacing+rectWidth,imageHeight-titleRect.height()-generics.size()*portH);
    }

    //Start painting right below the title block with ports / labels
    int y=titleRect.height();

    //Draw title label (centered)
    painter.setPen(titlePen);
    painter.setFont(titleFont);
    painter.drawText(leftOuter+spacing,0,rectWidth,titleRect.height(), Qt::AlignHCenter, entityName);

    //Draw input port names, type, comment and symbol
    for(int i=0; i<inputPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        painter.drawText(0,y+(portH-nameH)/2,leftOuter,portH,Qt::AlignRight, inputPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        painter.drawText(leftOuter+(2*spacing),y+nameH,leftInner,portH,Qt::AlignLeft, inputPorts[i].comment);
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QString typeString = inputPorts[i].type;
        if(inputPorts[i].def!="")typeString += " ("+inputPorts[i].def+")";
        painter.drawText(leftOuter+(2*spacing), y, leftInner, portH, Qt::AlignLeft, typeString);
        paintPortSymbol(painter,inputPorts[i].direction, leftOuter+(1*spacing), y+portH/2,false);
        y += portH;
    }

    //Put one port spacing between input ports and reset ports
    if(inputPorts.size()>0&&
            (resetPorts.size()>0||clockPorts.size()>0))
        y += portH;

    //Draw reset port names, type, comment and symbol
    for(int i=0; i<resetPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        painter.drawText(0,y+(portH-nameH)/2,leftOuter,portH,Qt::AlignRight, resetPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        painter.drawText(leftOuter+(2*spacing),y+nameH,leftInner,portH,Qt::AlignLeft, resetPorts[i].comment);
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QString typeString = resetPorts[i].type;
        if(resetPorts[i].def!="")typeString += " ("+resetPorts[i].def+")";
        painter.drawText(leftOuter+(2*spacing), y, leftInner, portH, Qt::AlignLeft, typeString);
        paintPortSymbol(painter,resetPorts[i].direction, leftOuter+(1*spacing), y+portH/2,false);
        y += portH;
    }

    //Put one port spacing between reset ports and clock ports
    if(resetPorts.size()>0&&clockPorts.size()>0)
        y += portH;

    //Draw clock port names, type, comment and symbol
    for(int i=0; i<clockPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        painter.drawText(0,y+(portH-nameH)/2,leftOuter,portH,Qt::AlignRight, clockPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        painter.drawText(leftOuter+(2*spacing),y+nameH,leftInner,portH,Qt::AlignLeft, clockPorts[i].comment);
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QString typeString = clockPorts[i].type;
        if(clockPorts[i].def!="")typeString += " ("+clockPorts[i].def+")";
        painter.drawText(leftOuter+(2*spacing), y, leftInner, portH, Qt::AlignLeft, typeString);
        paintPortSymbol(painter,clockPorts[i].direction, leftOuter+(1*spacing), y+portH/2,false);
        y += portH;
    }

    //Put cursor back to the top for the ports on the right side
    int genericY = y;
    y = titleRect.height();

    //Draw output port names, type, comment and symbol
    for(int i=0; i<outputPorts.size(); i++)
    {
        painter.setPen(namePen);
        painter.setFont(nameFont);
        painter.drawText(imageWidth-rightOuter,y+(portH-nameH)/2,rightOuter,portH,Qt::AlignLeft, outputPorts[i].name);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        painter.drawText(imageWidth - rightOuter - rightInner -(2*spacing),y+nameH,rightInner,portH,Qt::AlignRight, outputPorts[i].comment);
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QString typeString = outputPorts[i].type;
        if(outputPorts[i].def!="")typeString += " ("+outputPorts[i].def+")";
        painter.drawText(imageWidth - rightOuter - rightInner -(2*spacing), y, rightInner, portH, Qt::AlignRight, typeString);
        paintPortSymbol(painter,outputPorts[i].direction, imageWidth-rightOuter-(1*spacing), y+portH/2,true);
        y += portH;
    }

    //Draw generics.
    if(y<genericY)y=genericY;
    for(int i=0; i<generics.size(); i++)
    {
        painter.setPen(typePen);
        painter.setFont(nameFont);
        QString gText = generics[i].name + " : " + generics[i].type;
        if(generics[i].def!="")
            gText += " := " + generics[i].def;
        painter.drawText(leftOuter + (2*spacing),y,rectWidth,portH,Qt::AlignLeft, gText);
        painter.setPen(commentPen);
        painter.setFont(commentFont);
        painter.drawText(leftOuter + (2*spacing),y+nameH,rightOuter,portH,Qt::AlignLeft, generics[i].comment);
        y+= portH;
    }
}

void EntityBlock::saveSvg(QString targetName)
{
    for(int i=0; i<2; i++) //paint the whole thing twice, to calculate the size.
    {
        QPainter painter;
        QString path;
        if(targetName.length()==0)
            path = entityName+".svg";
        else
            path = targetName;
        if(!path.endsWith(".svg", Qt::CaseInsensitive))
            path += ".svg";
        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setTitle(entityName);
        generator.setDescription("Block converted from VHDL to svg with entity-block.");
        generator.setSize(QSize(imageWidth+20, imageHeight+20));
        //generator.setSize(QSize(-1,-1));
        generator.setViewBox(QRect(-10, -10, imageWidth+20, imageHeight+20));

        painter.begin(&generator);
        paint(painter);


        painter.end();
    }
}
