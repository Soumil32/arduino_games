/// @brief checks if two axis aligned bounding boxes are colliding
/// @param x1 x position of the first box
/// @param y1 y position of the first box
/// @param w1 width of the first box
/// @param h1 height of the first box
/// @param x2 x position of the second box
/// @param y2 y position of the second box
/// @param w2 width of the second box
/// @param h2 height of the second box
/// @return if they are colliding
bool checkAABB(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return (x1 <= x2 + w2 && x1 + w1 >= x2 && y1 <= y2 + h2 && y1 + h1 >= y2);
}