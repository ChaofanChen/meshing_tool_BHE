#include "pch.h"
#include <iostream>
/*
BHE SETUP TOOL
--------------------------
Author: Chaofan Chen, Philipp Hein
------------------------------------------------------------------------------
* Reads input file
* Creates GMSH .geo file
* Executes GMSH
* Imports GMSH 2D mesh
* Extrudes imported 2D mesh
* Creates BHE line elements
* Writes OGS mesh file
* Writes OGS geometry file
------------------------------------------------------------------------------
Prepare the input file as follows:
PROJECT project_name
WIDTH model_width
LENGTH model_length
DEPTH model_depth
BOX start length width
ELEM_SIZE box corner
LAYER mat_group number_of_elements element_thickness
BHE BHE_number x-coord y-coord z_top z_bottom radius
ADD_POINT x y delta
------------------------------------------------------------------------------
*/

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Prototypes
struct bhe
{
    int bhe_number;
    double bhe_x;
    double bhe_y;
    double bhe_top;
    double bhe_bottom;
    double bhe_radius;
};

struct node
{
    int node_number;
    double node_x;
    double node_y;
    double node_z;
};

struct prism_element
{
    int element_number;
    int material_group;
    string element_type = "pris";
    int node1, node2, node3, node4, node5, node6;
};

struct bhe_element
{
    int element_number;
    int material_group;
    string element_type = "line";
    int start_node;
    int end_node;
};

struct geometry
{
    double width, length, depth;
    double box_start = -1, box_length = -1, box_width = -1;
    double elem_size_box, elem_size_corner;
};

struct layer
{
    int mat_group;
    int n_elems;
    double elem_thickness;
};

struct additional_point
{
    double x;
    double y;
    double z = 0;
    double delta = 0;
};

bool ReadInputFile(const string &input_filename, string &project_name, geometry &geom, vector<layer> &layers, vector<bhe> &BHEs, vector<additional_point> &add_points);
bool WriteGMSHgeo(const string project_name, const geometry &geom, const vector<bhe> &BHEs, const vector<additional_point> &add_points);
bool ExecuteGMSH(const string project_name);
bool ImportGMSHmsh(const string project_name, vector<node> &nodes, vector<prism_element> &elements);
bool ExtrudeMesh(vector<node> &nodes, vector<prism_element> &elements, vector<layer> &layers, int &cnt_mat_groups, int &cnt_elems);
bool ComputeBHEelements(const vector<bhe> &BHEs, const vector<node> &nodes, vector<bhe_element> &bhe_elements, const int n_mat_groups, int &n_elems);
bool WriteMesh(const string project_name, const vector<node> &nodes, const vector<prism_element> &prism_elements, const vector<bhe_element> &bhe_elements);
bool WriteGLI(const string project_name, const geometry &geom, vector<bhe> &BHEs, const vector<additional_point> &add_points);
vector<string> Tokenize(const string &line);

int main(int argc, char *argv[])
{
    // Check input arguments
    if (argc < 2 || argc > 3)
    {
        cout << "Usage: bhe_setup_tool.exe input-filename (-2D)" << endl;
        return 0;
    }

    bool gmsh_only = false;
    if (argc == 3 && (string(argv[2]) == string("-2D")))
        gmsh_only = true;

    // Declarations
    string input_filename = string(argv[1]);
    vector<bhe> BHEs;
    vector<node> nodes;
    vector<prism_element> prism_elements;
    vector<bhe_element> bhe_elements;
    vector<layer> layers;
    vector<additional_point> add_points;
    geometry geom;
    int cnt_mat_groups = 0, cnt_elems = 0;

    string project_name = input_filename;
    project_name.erase(project_name.end() - 4, project_name.end());


    if (!ReadInputFile(input_filename,project_name, geom, layers, BHEs, add_points))
        return 0;

    if (!WriteGMSHgeo(project_name, geom, BHEs, add_points))
        return 0;

    if (!ExecuteGMSH(project_name))
        return 0;

    if (gmsh_only)
        return 0;

    if (!ImportGMSHmsh(project_name, nodes, prism_elements))
        return 0;

    if (!ExtrudeMesh(nodes, prism_elements, layers, cnt_mat_groups, cnt_elems))
        return 0;

    if (!ComputeBHEelements(BHEs, nodes, bhe_elements, cnt_mat_groups, cnt_elems))
        return 0;

    if (!WriteMesh(project_name, nodes, prism_elements, bhe_elements))
        return 0;

    if (!WriteGLI(project_name, geom, BHEs, add_points))
        return 0;

    cout << "Program terminated normally..." << endl;
    return 0;
}

bool ReadInputFile(const string &input_filename, string &project_name, geometry &geom, vector<layer> &layers, vector<bhe> &BHEs, vector<additional_point> &add_points)
{
    // Declarations
    string line;
    ifstream input_file(input_filename.c_str());
    geometry this_geom;
    int cnt_geo_param = 0;

    // Try to open file
    if (input_file.is_open())
    {
        // Read lines
        while (getline(input_file, line))
        {
            bool cmd_understood = false;

            // Get tokens
            vector<string> tokens = Tokenize(line);

            // Check keywords

            if (tokens[0] == string("%") || tokens[0] == string(":") || tokens[0] == string(";") || tokens[0] == string("#") || tokens[0] == string("//") || tokens[0] == string("/*") || tokens[0] == string("7"))
            {
                cmd_understood = true;
            }

            if (tokens[0] == string("PROJECT"))
            {
                if (tokens.size() == 2)
                {
                    project_name = tokens[1];
                    cmd_understood = true;
                }
            }

            if (tokens[0] == string("WIDTH"))
            {
                if (tokens.size() == 2)
                {
                    this_geom.width = atof(tokens[1].c_str());
                    cmd_understood = true;
                    cnt_geo_param++;
                }
            }

            if (tokens[0] == string("LENGTH"))
            {
                if (tokens.size() == 2)
                {
                    this_geom.length = atof(tokens[1].c_str());
                    cmd_understood = true;
                    cnt_geo_param++;
                }
            }

            if (tokens[0] == string("DEPTH"))
            {
                if (tokens.size() == 2)
                {
                    this_geom.depth = atof(tokens[1].c_str());
                    cmd_understood = true;
                    cnt_geo_param++;
                }
            }

            if (tokens[0] == string("BOX"))
            {
                if (tokens.size() == 4)
                {
                    this_geom.box_start = atof(tokens[1].c_str());
                    this_geom.box_length = atof(tokens[2].c_str());
                    this_geom.box_width = atof(tokens[3].c_str());
                    cmd_understood = true;
                }
            }

            if (tokens[0] == string("ELEM_SIZE"))
            {
                if (tokens.size() == 3)
                {
                    this_geom.elem_size_box = atof(tokens[1].c_str());
                    this_geom.elem_size_corner = atof(tokens[2].c_str());
                    cmd_understood = true;
                    cnt_geo_param++;
                }
            }

            if (tokens[0] == string("LAYER"))
            {
                if (tokens.size() == 4)
                {
                    layer this_layer;

                    this_layer.mat_group = atoi(tokens[1].c_str());
                    this_layer.n_elems = atoi(tokens[2].c_str());
                    this_layer.elem_thickness = atof(tokens[3].c_str());

                    cmd_understood = true;

                    layers.push_back(this_layer);
                }
            }

            if (tokens[0] == string("BHE"))
            {
                if (tokens.size() == 7)
                {
                    bhe this_BHE;

                    this_BHE.bhe_number = atoi(tokens[1].c_str());
                    this_BHE.bhe_x = atof(tokens[2].c_str());
                    this_BHE.bhe_y = atof(tokens[3].c_str());
                    this_BHE.bhe_top = atof(tokens[4].c_str());
                    this_BHE.bhe_bottom = atof(tokens[5].c_str());
                    this_BHE.bhe_radius = atof(tokens[6].c_str());
                    cmd_understood = true;

                    BHEs.push_back(this_BHE);
                }
            }

            if (tokens[0] == string("ADD_POINT"))
            {
                if (tokens.size() == 4)
                {
                    additional_point this_point;

                    this_point.x = atof(tokens[1].c_str());
                    this_point.y = atof(tokens[2].c_str());
                    this_point.delta = atof(tokens[3].c_str());
                    cmd_understood = true;

                    add_points.push_back(this_point);
                }
            }

            if (!cmd_understood)
                cout << "Error: Couldn't understand command " << line << "!" << endl;
        }

        geom = this_geom;

        input_file.close();

        if (BHEs.size() == 0)
        {
            cout << "Error: No BHEs defined!" << endl;
            return false;
        }

        if (layers.size() == 0)
        {
            cout << "Error: No layers defined!" << endl;
            return false;
        }

        if (cnt_geo_param != 4)
        {
            cout << "Error: Definition of geometry incomplete!" << endl;
            return false;
        }

        cout << "Reading input file " << input_filename << " successful..." << endl;
        return true;
    }

    cout << "Error: Couldn't open input file!" << endl;
    return false;
}

bool WriteGMSHgeo(const string project_name, const geometry &geom, const vector<bhe> &BHEs, const vector<additional_point> &add_points)
{
    int i;
    int n_BHEs = BHEs.size();
    int n_add_points = add_points.size();
    int cnt_pnt = 9;

    string geo_filename = project_name + ".geo";
    ofstream geo_file(geo_filename.c_str());

    if (geo_file.is_open())
    {
        // Write basic geometry
        geo_file << "// created by bhe_setup_tool" << endl;
        geo_file << "// project name: " << project_name << endl << endl;
        geo_file << "// geometry parameters" << endl;
        geo_file << "width = " << geom.width << ";" << endl;
        geo_file << "length = " << geom.length << ";" << endl;
        geo_file << "box_start = " << geom.box_start << ";" << endl;
        geo_file << "box_length = " << geom.box_length << ";" << endl;
        geo_file << "box_width = " << geom.box_width << ";" << endl << endl;
        geo_file << "// element sizes" << endl;
        geo_file << "elem_size_box = " << geom.elem_size_box << ";" << endl;
        geo_file << "elem_size_corner = " << geom.elem_size_corner << ";" << endl << endl;
        geo_file << "// model boundaries" << endl;
        geo_file << "Point(1) = {-width/2.0, 0.0, 0.0, elem_size_corner};" << endl;
        geo_file << "Point(2) = { width/2.0, 0.0, 0.0, elem_size_corner};" << endl;
        geo_file << "Point(3) = { width/2.0, length, 0.0, elem_size_corner};" << endl;
        geo_file << "Point(4) = {-width/2.0, length, 0.0, elem_size_corner};" << endl;
        geo_file << "Line(1) = {1, 4};" << endl;
        geo_file << "Line(2) = {4, 3};" << endl;
        geo_file << "Line(3) = {3, 2};" << endl;
        geo_file << "Line(4) = {2, 1};" << endl;
        geo_file << "Line Loop(1) = {1, 2, 3, 4};" << endl;
        geo_file << "Plane Surface(1) = {1};" << endl << endl;
        if (!(geom.box_length == -1 || geom.box_start == -1 || geom.box_width == -1))
        {
            geo_file << "// bounding box" << endl;
            geo_file << "Point(5) = { box_width/2.0, box_start, 0.0, elem_size_box};" << endl;
            geo_file << "Point(6) = {-box_width/2.0, box_start, 0.0, elem_size_box};" << endl;
            geo_file << "Point(7) = {-box_width/2.0, box_start + box_length, 0.0, elem_size_box};" << endl;
            geo_file << "Point(8) = { box_width/2.0, box_start + box_length, 0.0, elem_size_box};" << endl;
            geo_file << "Line(5) = {5, 6};" << endl;
            geo_file << "Line(6) = {6, 7};" << endl;
            geo_file << "Line(7) = {7, 8};" << endl;
            geo_file << "Line(8) = {8, 5};" << endl;
            geo_file << "Line {5, 6, 7, 8} In Surface {1};" << endl << endl;
        }


        // Write BHEs
        // Currently fixed with n=6 nodes
        double alpha = 6.134;
        string point_list = "";
        for (i = 0; i < n_BHEs; i++)
        {
            double delta = alpha * BHEs[i].bhe_radius;

            geo_file << "// BHE #" << BHEs[i].bhe_number << endl;
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x << ", " << BHEs[i].bhe_y << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++)); point_list.append(", ");
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x << ", " << BHEs[i].bhe_y - delta << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++)); point_list.append(", ");
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x << ", " << BHEs[i].bhe_y + delta << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++)); point_list.append(", ");
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x + 0.866*delta << ", " << BHEs[i].bhe_y + 0.5*delta << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++)); point_list.append(", ");
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x - 0.866*delta << ", " << BHEs[i].bhe_y + 0.5*delta << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++)); point_list.append(", ");
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x + 0.866*delta << ", " << BHEs[i].bhe_y - 0.5*delta << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++)); point_list.append(", ");
            geo_file << "Point(" << cnt_pnt << ") = {" << BHEs[i].bhe_x - 0.866*delta << ", " << BHEs[i].bhe_y - 0.5*delta << ", 0.0, " << delta << "};" << endl; point_list.append(to_string(cnt_pnt++));
            if (i < (n_BHEs - 1))
                point_list.append(", ");
            if (n_add_points > 0 && i == (n_BHEs - 1))
                point_list.append(", ");
        }

        if (n_add_points > 0)
            for (int j = 0; j < n_add_points; j++)
            {
                geo_file << "Point(" << cnt_pnt << ") = {" << add_points[j].x << ", " << add_points[j].y << ", " << add_points[j].z;
                if (add_points[j].delta > 0)
                {
                    geo_file << ", " << add_points[j].delta;
                    geo_file << "};" << endl;
                }
                else
                    geo_file << "};" << endl;
                point_list.append(to_string(cnt_pnt++));
                if(j < (n_add_points-1))
                    point_list.append(", ");
            }

        geo_file << "Point{" << point_list << "} In Surface{1};" << endl << endl;

        geo_file.close();

        cout << "Writing GMSH geometry file " << geo_filename << " successful..." << endl;
        return true;
    }

    cout << "Error: Couldn't open GMSH geometry file!" << endl;
    return false;
}

bool ExecuteGMSH(const string project_name)
{
    string gmsh_path = "";
    string gmsh_call = gmsh_path;
    gmsh_call.append("gmsh.exe ");
    gmsh_call.append(project_name);
    gmsh_call.append(".geo -2 -format msh2");

    try
    {
        cout << "Calling gmsh.exe..." << endl;
        system(gmsh_call.c_str());
    }
    catch (...)
    {
        cout << "Error: Couldn't find gmsh.exe in this directory!";
        return false;
    }

    cout << "Calling gmsh.exe successful..." << endl;
    return true;
}

bool ImportGMSHmsh(const string project_name, vector<node> &nodes, vector<prism_element> &elements)
{
    // Declarations
    string line;
    string mesh_filename = project_name;
    mesh_filename.append(".msh");
    ifstream mesh_file(mesh_filename.c_str());
    bool is_node = false;
    bool is_element = false;
    int cnt_elem = 0;

    // Try to open file
    if (mesh_file.is_open())
    {
        // Read lines
        while (getline(mesh_file, line))
        {
            // Get tokens
            vector<string> tokens = Tokenize(line);

            // Check keywords
            if (tokens[0] == string("$Nodes"))
            {
                is_element = false;
                is_node = true;
            }

            if (tokens[0] == string("$EndNodes"))
            {
                is_element = false;
                is_node = false;
            }

            if (tokens[0] == string("$Elements"))
            {
                is_element = true;
                is_node = false;
            }

            if (tokens[0] == string("$EndElements"))
            {
                is_element = false;
                is_node = false;
            }

            // Read nodes
            if (is_node && tokens.size() == 4)
            {
                node this_node;

                this_node.node_number = atoi(tokens[0].c_str())-1;
                this_node.node_x = atof(tokens[1].c_str());
                this_node.node_y = atof(tokens[2].c_str());
                this_node.node_z = atof(tokens[3].c_str());

                nodes.push_back(this_node);
            }

            // Read elements
            if (is_element && tokens.size() == 8 && atoi(tokens[1].c_str()) == 2)
            {
                prism_element this_element;

                int shift = atoi(tokens[2].c_str());

                this_element.element_number = cnt_elem++;
                this_element.material_group = 0;
                this_element.node1 = atoi(tokens[3+shift].c_str())-1;
                this_element.node2 = atoi(tokens[4+shift].c_str())-1;
                this_element.node3 = atoi(tokens[5+shift].c_str())-1;
                this_element.node4 = 0;
                this_element.node5 = 0;
                this_element.node6 = 0;

                elements.push_back(this_element);
            }
        }

        mesh_file.close();

        cout << "Importing 2D mesh " << mesh_filename << " successful..." << endl;
        return true;
    }

    cout << "Error: Couldn't open mesh file!" << endl;
    return false;
}

bool ExtrudeMesh(vector<node> &nodes, vector<prism_element> &elements, vector<layer> &layers, int &cnt_mat_groups, int &cnt_elems)
{
    int i, j, k;
    int n_layers = layers.size();
    int n_nodes_in_plane = nodes.size();
    int n_elems_in_plane = elements.size();
    int cnt = 1;

    // Loop over layers
    for (i = 0; i < n_layers; i++)
    {
        int n_elems = layers[i].n_elems;
        int mat_group = layers[i].mat_group;
        double z_shift = layers[i].elem_thickness;

        if (mat_group > cnt_mat_groups)
            cnt_mat_groups = mat_group;

        // Loop over elements per layer
        for (j = 0; j < n_elems; j++)
        {
            // Create nodes
            for (k = 0; k < n_nodes_in_plane; k++)
            {
                int idx_new = cnt*n_nodes_in_plane + k;
                int idx_old = (cnt - 1)*n_nodes_in_plane + k;

                nodes.push_back(nodes[idx_old]);
                nodes[idx_new].node_z -= z_shift;
                nodes[idx_new].node_number = nodes[idx_old].node_number + n_nodes_in_plane;
            }

            // Create elements
            for (k = 0; k < n_elems_in_plane; k++)
            {
                if (cnt == 1)
                {
                    int idx = (cnt - 1)*n_elems_in_plane + k;

                    elements[idx].node4 = elements[idx].node1 + n_nodes_in_plane;
                    elements[idx].node5 = elements[idx].node2 + n_nodes_in_plane;
                    elements[idx].node6 = elements[idx].node3 + n_nodes_in_plane;
                    elements[idx].material_group = mat_group;
                }
                else
                {
                    int idx_old = (cnt - 2)*n_elems_in_plane + k;

                    prism_element this_element;
                    this_element.element_number = elements[idx_old].element_number + n_elems_in_plane;
                    this_element.material_group = mat_group;
                    this_element.node1 = elements[idx_old].node4;
                    this_element.node2 = elements[idx_old].node5;
                    this_element.node3 = elements[idx_old].node6;
                    this_element.node4 = this_element.node1 + n_nodes_in_plane;
                    this_element.node5 = this_element.node2 + n_nodes_in_plane;
                    this_element.node6 = this_element.node3 + n_nodes_in_plane;

                    elements.push_back(this_element);
                }
            }

            cnt++;
        }
    }

    cnt_elems = elements.size();
    cout << "Extrusion of 2D mesh successful: Created " << nodes.size() << " nodes and " << cnt_elems << " elements..." << endl;

    return true;
}

bool ComputeBHEelements(const vector<bhe> &BHEs, const vector<node> &nodes, vector<bhe_element> &bhe_elements, const int n_mat_groups, int &n_elems)
{
    int i, j;
    int n_BHEs = BHEs.size();
    int n_nodes = nodes.size();
    int cnt_elem = n_elems - 1;
    int cnt_mat_group = n_mat_groups;

    // Loop over BHEs
    for (i = 0; i < n_BHEs; i++)
    {
        // Increase material group per BHE
        cnt_mat_group++;

        vector<node> bhe_nodes;

        // Loop over nodes
        for (j = 0; j < n_nodes; j++)
        {
            // Check if node is on BHE and copy to BHE node list
            if (nodes[j].node_x == BHEs[i].bhe_x && nodes[j].node_y == BHEs[i].bhe_y && nodes[j].node_z <= BHEs[i].bhe_top && nodes[j].node_z >= BHEs[i].bhe_bottom)
                bhe_nodes.push_back(nodes[j]);
        }

        int n_BHE_nodes = bhe_nodes.size();
        int n_BHE_elems = n_BHE_nodes - 1;

        // Loop over and create BHE elements
        for (j = 0; j < n_BHE_elems; j++)
        {
            // Increase element counter
            cnt_elem++;

            bhe_element this_bhe_element;

            // Assign element data
            this_bhe_element.element_number = cnt_elem;
            this_bhe_element.material_group = cnt_mat_group;
            this_bhe_element.start_node = bhe_nodes[j].node_number;
            this_bhe_element.end_node = bhe_nodes[j + 1].node_number;

            // Copy to BHE element list
            bhe_elements.push_back(this_bhe_element);
        }

        cout << "Created " << n_BHE_elems << " elements on BHE #" << i << endl;
    }

    // Write back total number of elements
    n_elems = cnt_elem;

    cout << "BHE meshing successful..." << endl;
    return true;
}

bool WriteMesh(const string project_name, const vector<node> &nodes, const vector<prism_element> &prism_elements, const vector<bhe_element> &bhe_elements)
{
    // Declarations
    string line;
    string mesh_filename = project_name;
    mesh_filename.append(".bhe.msh");
    ofstream mesh_file(mesh_filename.c_str());

    int i;
    int n_nodes = nodes.size();
    int n_elems = prism_elements.size() + bhe_elements.size();

    // Try to open mesh files
    if (mesh_file.is_open())
    {
        // Write header
        mesh_file << "#FEM_MSH" << endl;
        mesh_file << "$PCS_TYPE" << endl;
        mesh_file << "NO_PCS" << endl;
        // Write nodes
        mesh_file << "$NODES" << endl;
        mesh_file << n_nodes << endl;
        for (i = 0; i < n_nodes; i++)
            mesh_file << nodes[i].node_number << " " << nodes[i].node_x << " " << nodes[i].node_y << " " << nodes[i].node_z << endl;
        //Write elements
        mesh_file << "$ELEMENTS" << endl;
        mesh_file << n_elems << endl;
        for (i = 0; i < prism_elements.size(); i++)
            mesh_file << prism_elements[i].element_number << " " << prism_elements[i].material_group << " " << prism_elements[i].element_type << " " << prism_elements[i].node1 << " " << prism_elements[i].node2 << " " << prism_elements[i].node3 << " " << prism_elements[i].node4 << " " << prism_elements[i].node5 << " " << prism_elements[i].node6 << endl;
        for (i = 0; i < bhe_elements.size(); i++)
            mesh_file << bhe_elements[i].element_number << " " << bhe_elements[i].material_group << " " << bhe_elements[i].element_type << " " << bhe_elements[i].start_node << " " << bhe_elements[i].end_node << endl;
        mesh_file << "#STOP" << endl;

        mesh_file.close();

        cout << "Write mesh to " << mesh_filename << " successful..." << endl;
        return true;
    }

    cout << "Error: Couldn't open mesh file!" << endl;
    return false;
}

bool WriteGLI(const string project_name, const geometry &geom, vector<bhe> &BHEs, const vector<additional_point> &add_points)
{
    // Declarations
    string line;
    string gli_filename = project_name;
    gli_filename.append(".gli");
    ofstream gli_file(gli_filename.c_str());

    int i;
    int n_BHEs = BHEs.size();
    int n_add_points = add_points.size();
    int cnt_pnt = 8;

    // Try to open mesh files
    if (gli_file.is_open())
    {
        // Write points
        gli_file << "#POINTS" << endl;
        gli_file << "0 " << -geom.width / 2.0 << " 0 0" << endl;
        gli_file << "1 " <<  geom.width / 2.0 << " 0 0" << endl;
        gli_file << "2 " <<  geom.width / 2.0 << " " << geom.length << " 0" << endl;
        gli_file << "3 " << -geom.width / 2.0 << " " << geom.length << " 0" << endl;
        gli_file << "4 " << -geom.width / 2.0 << " 0 " << -geom.depth << endl;
        gli_file << "5 " << geom.width / 2.0 << " 0 " << -geom.depth << endl;
        gli_file << "6 " << geom.width / 2.0 << " " << geom.length << " " << -geom.depth << endl;
        gli_file << "7 " << -geom.width / 2.0 << " " << geom.length << " " << -geom.depth << endl;
        for (i = 0; i < n_BHEs; i++)
        {
            gli_file << cnt_pnt++ << " " << BHEs[i].bhe_x << " " << BHEs[i].bhe_y << " " << BHEs[i].bhe_top << " $NAME BHE"<< BHEs[i].bhe_number << "_top" << endl;
            gli_file << cnt_pnt++ << " " << BHEs[i].bhe_x << " " << BHEs[i].bhe_y << " " << BHEs[i].bhe_bottom << " $NAME BHE" << BHEs[i].bhe_number << "_bottom" << endl;
        }
        for (i = 0; i < n_add_points; i++)
            gli_file << cnt_pnt++ << " " << add_points[i].x << " " << add_points[i].y << " " << add_points[i].z << " $NAME P" << i+1 << endl;
        // Write polylines
        gli_file << "#POLYLINE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "ply_top" << endl;
        gli_file << "$POINTS" << endl;
        gli_file << "0" << endl;
        gli_file << "1" << endl;
        gli_file << "2" << endl;
        gli_file << "3" << endl;
        gli_file << "0" << endl;
        gli_file << "#POLYLINE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "ply_bottom" << endl;
        gli_file << "$POINTS" << endl;
        gli_file << "4" << endl;
        gli_file << "5" << endl;
        gli_file << "6" << endl;
        gli_file << "7" << endl;
        gli_file << "4" << endl;
        gli_file << "#POLYLINE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "ply_left" << endl;
        gli_file << "$POINTS" << endl;
        gli_file << "0" << endl;
        gli_file << "3" << endl;
        gli_file << "7" << endl;
        gli_file << "4" << endl;
        gli_file << "0" << endl;
        gli_file << "#POLYLINE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "ply_right" << endl;
        gli_file << "$POINTS" << endl;
        gli_file << "1" << endl;
        gli_file << "2" << endl;
        gli_file << "6" << endl;
        gli_file << "5" << endl;
        gli_file << "1" << endl;
        gli_file << "#POLYLINE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "ply_inflow" << endl;
        gli_file << "$POINTS" << endl;
        gli_file << "0" << endl;
        gli_file << "1" << endl;
        gli_file << "5" << endl;
        gli_file << "4" << endl;
        gli_file << "0" << endl;
        gli_file << "#POLYLINE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "ply_outflow" << endl;
        gli_file << "$POINTS" << endl;
        gli_file << "3" << endl;
        gli_file << "2" << endl;
        gli_file << "6" << endl;
        gli_file << "7" << endl;
        gli_file << "3" << endl;
        for (i = 0; i < n_BHEs; i++)
        {
            gli_file << "#POLYLINE" << endl;
            gli_file << "$NAME" << endl;
            gli_file << "ply_BHE" << BHEs[i].bhe_number << endl;
            gli_file << "$POINTS" << endl;
            gli_file << 8 + 2 * i << endl;
            gli_file << 9 + 2 * i << endl;
        }
        // Write surfaces
        gli_file << "#SURFACE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "top" << endl;
        gli_file << "$POLYLINES" << endl;
        gli_file << "ply_top" << endl;
        gli_file << "#SURFACE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "bottom" << endl;
        gli_file << "$POLYLINES" << endl;
        gli_file << "ply_bottom" << endl;
        gli_file << "#SURFACE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "left" << endl;
        gli_file << "$POLYLINES" << endl;
        gli_file << "ply_left" << endl;
        gli_file << "#SURFACE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "right" << endl;
        gli_file << "$POLYLINES" << endl;
        gli_file << "ply_right" << endl;
        gli_file << "#SURFACE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "inflow" << endl;
        gli_file << "$POLYLINES" << endl;
        gli_file << "ply_inflow" << endl;
        gli_file << "#SURFACE" << endl;
        gli_file << "$NAME" << endl;
        gli_file << "outflow" << endl;
        gli_file << "$POLYLINES" << endl;
        gli_file << "ply_outflow" << endl;
        gli_file << "#STOP" << endl;

        gli_file.close();

        cout << "Write OGS geometry to " << gli_filename << " successful..." << endl;
        return true;
    }

    cout << "Error: Couldn't open OGS geometry file!" << endl;
    return false;
}

vector<string> Tokenize(const string &line)
{
    const string delimiter = " ";
    vector<string> tokens;

    // Skip delim at beginning.
    string::size_type lastPos = line.find_first_not_of(delimiter, 0);
    // Find first "non-delimiter".
    string::size_type pos = line.find_first_of(delimiter, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        tokens.push_back(line.substr(lastPos, pos - lastPos));
        lastPos = line.find_first_not_of(delimiter, pos);
        pos = line.find_first_of(delimiter, lastPos);
    }

    return tokens;
}
