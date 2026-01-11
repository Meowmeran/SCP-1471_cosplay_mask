#include "Orientation.h"

int OrientationHelper::getPhysicalIndex(int logicalIndex, Orientation orientation)
{
    int row = logicalIndex / 4;
    int col = logicalIndex % 4;
    return getPhysicalIndex(row, col, orientation);
}

int OrientationHelper::getPhysicalIndex(int row, int col, Orientation orientation)
{
    switch (orientation) {
        case Orientation::NORMAL:
            return row * 4 + col;
        case Orientation::ROTATED_90:
            return col * 4 + (3 - row);
        case Orientation::ROTATED_180:
            return (3 - row) * 4 + (3 - col);
        case Orientation::ROTATED_270:
            return (3 - col) * 4 + row;
        default:
            return row * 4 + col;
    }
}

int OrientationHelper::getRotationAngle(Orientation orientation)
{
    switch (orientation) {
        case Orientation::NORMAL:
            return 0;
        case Orientation::ROTATED_90:
            return 90;
        case Orientation::ROTATED_180:
            return 180;
        case Orientation::ROTATED_270:
            return 270;
        default:
            return 0;
    }
}
