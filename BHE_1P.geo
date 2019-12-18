// created by bhe_setup_tool
// project name: BHE

// geometry parameters
width = 20;
length = 20;
box_start = -1;
box_length = -1;
box_width = -1;

// element sizes
elem_size_box = 1;
elem_size_corner = 1;

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
Point(9) = {0, 10, 0.0, 0.85876};
Point(10) = {0, 9.14124, 0.0, 0.85876};
Point(11) = {0, 10.8588, 0.0, 0.85876};
Point(12) = {0.743686, 10.4294, 0.0, 0.85876};
Point(13) = {-0.743686, 10.4294, 0.0, 0.85876};
Point(14) = {0.743686, 9.57062, 0.0, 0.85876};
Point(15) = {-0.743686, 9.57062, 0.0, 0.85876};
Point{9, 10, 11, 12, 13, 14, 15} In Surface{1};

Extrude {0,0,-50}
{
Surface{1};
Point{9};
Layers{20};
Recombine;
}

Extrude { {0,1,0} , {55,0,-50} , -Pi/2 }
{
  Surface{26}; 
  Point{26};
  Layers{15}; 
  Recombine;
}

Extrude { {0,1,0} , {55,0,-50} , -Pi/2 }
{
  Surface{49}; 
  Point{51};
  Layers{15}; 
  Recombine;
}

Extrude { 0, 0, 50 }
{
  Surface{72}; 
  Point{63};
  Layers{20}; 
  Recombine;
}

// Define the material group
Physical Line(2) = {27, 50, 73, 96};
Physical Volume(1) = {1, 2, 3, 4};

Mesh 3;
Coherence Mesh;

