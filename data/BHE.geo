// created by bhe_setup_tool
// project name: C:\Users\Chaofan\Documents\mesh_tool\mesh_tool_BHE\meshing_tool_BHE\data\BHE

// geometry parameters
width = 20;
length = 30;
box_start = -1;
box_length = -1;
box_width = -1;

// element sizes
elem_size_box = 5;
elem_size_corner = 5;

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

// BHE #0
Point(9) = {0, 10, 0.0, 0.631802};
Point(10) = {0, 9.3682, 0.0, 0.631802};
Point(11) = {0, 10.6318, 0.0, 0.631802};
Point(12) = {0.547141, 10.3159, 0.0, 0.631802};
Point(13) = {-0.547141, 10.3159, 0.0, 0.631802};
Point(14) = {0.547141, 9.6841, 0.0, 0.631802};
Point(15) = {-0.547141, 9.6841, 0.0, 0.631802};
// BHE #1
Point(16) = {0, 20, 0.0, 0.631802};
Point(17) = {0, 19.3682, 0.0, 0.631802};
Point(18) = {0, 20.6318, 0.0, 0.631802};
Point(19) = {0.547141, 20.3159, 0.0, 0.631802};
Point(20) = {-0.547141, 20.3159, 0.0, 0.631802};
Point(21) = {0.547141, 19.6841, 0.0, 0.631802};
Point(22) = {-0.547141, 19.6841, 0.0, 0.631802};
Point{9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22} In Surface{1};

