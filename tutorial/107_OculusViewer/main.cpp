#include <igl/read_triangle_mesh.h>
#include <igl/viewer/VR_Viewer.h>
#include <igl/viewer/Viewer.h>
#include "tutorial_shared_path.h"
# include <iostream>


Eigen::MatrixXd V;
Eigen::MatrixXi F;

int main(int argc, char *argv[])
{
	// Load a mesh in OFF format
	igl::read_triangle_mesh(TUTORIAL_SHARED_PATH "/arm.obj", V, F);

	// Plot the mesh
	igl::viewer::VR_Viewer viewervr;
	
	viewervr.data.set_mesh(V, F);
	//Test floor


	viewervr.init();
	//viewervr.set_data(viewervr.data, true);
	//viewervr.bind_mesh();

	viewervr.launch();

	/* igl::viewer::Viewer viewer;
	viewer.data.set_mesh(V, F);

	 viewer.init();
	 viewer.launch();*/
}
