// created by bhe_setup_tool
// project name: C:\Users\Chaofan\Documents\mesh_tool\mesh_tool_BHE\meshing_tool_BHE\data\BHE

// geometry parameters
width = 100;
length = 300;
box_start = 50;
box_length = 200;
box_width = 60;

// element sizes
elem_size_box = 5;
elem_size_corner = 20;

// model boundaries
Point(1) = {-width/2.0, 0.0, 0.0, elem_size_corner};
Point(2) = { width/2.0, 0.0, 0.0, elem_size_corner};
Point(3) = { width/2.0, length, 0.0, elem_size_corner};
Point(4) = {-width/2.0, length, 0.0, elem_size_corner};
Line(1) = {1, 4};
Line(2) = {4, 3};
Line(3) = {3, 2};
Line(4) = {2, 1};
Line Loop(1) = {1, 2, 3, 4};
Plane Surface(1) = {1};

// bounding box
Point(5) = { box_width/2.0, box_start, 0.0, elem_size_box};
Point(6) = {-box_width/2.0, box_start, 0.0, elem_size_box};
Point(7) = {-box_width/2.0, box_start + box_length, 0.0, elem_size_box};
Point(8) = { box_width/2.0, box_start + box_length, 0.0, elem_size_box};
Line(5) = {5, 6};
Line(6) = {6, 7};
Line(7) = {7, 8};
Line(8) = {8, 5};
Line {5, 6, 7, 8} In Surface {1};

// BHE #0
Point(9) = {0, 100, 0.0, 0.631802};
Point(10) = {0, 99.3682, 0.0, 0.631802};
Point(11) = {0, 100.632, 0.0, 0.631802};
Point(12) = {0.547141, 100.316, 0.0, 0.631802};
Point(13) = {-0.547141, 100.316, 0.0, 0.631802};
Point(14) = {0.547141, 99.6841, 0.0, 0.631802};
Point(15) = {-0.547141, 99.6841, 0.0, 0.631802};
// BHE #1
Point(16) = {0, 150, 0.0, 0.631802};
Point(17) = {0, 149.368, 0.0, 0.631802};
Point(18) = {0, 150.632, 0.0, 0.631802};
Point(19) = {0.547141, 150.316, 0.0, 0.631802};
Point(20) = {-0.547141, 150.316, 0.0, 0.631802};
Point(21) = {0.547141, 149.684, 0.0, 0.631802};
Point(22) = {-0.547141, 149.684, 0.0, 0.631802};
// BHE #2
Point(23) = {0, 200, 0.0, 0.631802};
Point(24) = {0, 199.368, 0.0, 0.631802};
Point(25) = {0, 200.632, 0.0, 0.631802};
Point(26) = {0.547141, 200.316, 0.0, 0.631802};
Point(27) = {-0.547141, 200.316, 0.0, 0.631802};
Point(28) = {0.547141, 199.684, 0.0, 0.631802};
Point(29) = {-0.547141, 199.684, 0.0, 0.631802};
Point{9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29} In Surface{1};

