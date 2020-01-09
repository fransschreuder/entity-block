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


#ifndef ENTITYBLOCK_H
#define ENTITYBLOCK_H

#include <QWidget>
#include <QList>
#include <QPainter>
#include <QSettings>

///Types of ports in order to draw the right symbol.
typedef enum{in, out, inout, buffer, linkage} direction_t;
const char direction_names[][16]={"in", "out", "inout", "buffer", "linkage"};

///Holds the textual properties of an entity port as declared in the VHDL entity. Also used to store generics
class Port
{
public:
    Port();
    QString name;
    direction_t direction;
    QString type;
    QString comment;
    QString def;
};


class EntityBlock
{

public:
    /**
     * @brief EntityBlock Constructor, initializes some values and reads colors from s
     * @param fileName VHDL file to be processed
     * @param targetName SVG file to be stored
     * @param s
     */
    EntityBlock(QString fileName = "", QString targetName="", QSettings* s=NULL, bool simplifiedSymbol=false);
    ~EntityBlock();

    /**
     * @brief loadFile loads a vhdl file from which it extracts the libraries, ports and generics in the entity.
     * This function is already called from the constructor, but can be used separately if fileName = "" in constructor.
     * @param fileName full path of the VHDL file to load.
     */
    bool loadFile(QString fileName);

    /**
     * @brief success false if loadFile did not complete successfully
     */
    bool success;

    /**
     * @brief saveSvg saves the loaded entity as .svg image.
     * This function is already called from the constructor, but can be used separately if fileName = "" in constructor.
     * @param targetName
     */
    void saveSvg(QString targetName);


private:
    /**
     * @brief Used to store colors and dimensions of the symbol to draw.
     */
    QSettings *settings;

    /**
     * @brief paintPortSymbol draws a symbol for ports (in, out, inout, buffer, linkage)
     * @param painter QPainter to draw on.
     * @param direction in, out, inout, buffer or linkage
     * @param x horizontal (center) position of symbol
     * @param y vertical (center) position of symbol
     * @param mirror true for drawing a port symbol on the right side of the entity
     */
    void paintPortSymbol(QPainter& painter, direction_t direction, int x, int y, bool mirror);

    /**
     * @brief paint draws the symbol including all ports and strings on a QPainter. Could be svg or anything else in Qt.
     * @param painter QPainter object to draw on
     */
    void paint(QPainter &painter);

    /**
     * @brief parseEntityString reads a QString containing the entity part of a VHDL file and creates two QLists: ports and generics
     * @param entityString entity part of the VHDL file
     */
    void parseEntityString(QString entityString);

    /**
     * @brief searchNoComments is a helper function to find a string, but it skips vhdl comments marked as --
     * @param string Text to search through
     * @param seed Text to search for
     * @param from Start searching from this position in the string
     * @return position of text found, -1 if not found.
     */
    int searchNoComments(QString string, QString seed, int from);

    /**
     * @brief searchCloseBracket Searches the location of the first unmatched close bracket ).
     * It assumes that the opening bracket was already removed from the string.
     * @param string Text to search in, beginning the character after the opening bracket (
     * @return location of the close bracket or -1 if not found.
     */
    int searchCloseBracket(QString string);

    /**
     * @brief entityName contains the name of the entity, used as a title for the symbol and filename for
     * the SVG file if no filename was specified.
     */
    QString entityName;

    /**
     * @brief libraries used libraries in VHDL file, they are only read, not used for anything
     */
    QList<QString> libraries;

    /**
     * @brief ports Contains properties of the ports in the entity of the vhdl file
     */
    QList<Port> ports;
    /**
     * @brief generics Contains properties of the generics in the entity of the vhdl file
     */
    QList<Port> generics;

    /**
     * @brief imageWidth automatically determined in paint function
     */
    int imageWidth;
    /**
     * @brief imageHeight automatically determined in paint function
     */
    int imageHeight;

    /**
     * Several colors and dimensions, read from QSettings, used to draw the symbol
     */
    QColor cComment, cPortName, cPortType, cBackground, cHeader1, cHeader2, cTitle, cBorder, cPorts,cShadow;
    int cornerRadius;
    int borderWidth;
    int spacing;
    bool createSimplifiedSymbol;


};

#endif // ENTITYBLOCK_H
