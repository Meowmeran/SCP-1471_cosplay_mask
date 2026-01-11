#ifndef ORIENTATION_H
#define ORIENTATION_H

enum class Orientation {
    NORMAL = 0,      // 0 degrees
    ROTATED_90 = 1,  // 90 degrees clockwise
    ROTATED_180 = 2, // 180 degrees
    ROTATED_270 = 3  // 270 degrees clockwise
};

class OrientationHelper {
public:
    /**
     * Converts a 4x4 matrix index to physical pixel index based on orientation
     * @param logicalIndex Index in the logical matrix (0-15)
     * @param orientation The orientation of the matrix
     * @return Physical pixel index
     */
    static int getPhysicalIndex(int logicalIndex, Orientation orientation);

    /**
     * Converts row and column to physical pixel index
     * @param row Row in logical matrix (0-3)
     * @param col Column in logical matrix (0-3)
     * @param orientation The orientation of the matrix
     * @return Physical pixel index
     */
    static int getPhysicalIndex(int row, int col, Orientation orientation);

    /**
     * Gets the rotation angle in degrees
     * @param orientation The orientation
     * @return Angle in degrees (0, 90, 180, 270)
     */
    static int getRotationAngle(Orientation orientation);
};

#endif // ORIENTATION_H
