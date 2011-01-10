#include <math.h>
#include "allocore/graphics/al_Shapes.hpp"

namespace al{

const double phi = (1 + sqrt(5))/2; // the golden ratio

int addTetrahedron(Mesh& m){
	static const float l = sqrt(1./3);
	static const float vertices[] = {
		 l, l, l,
		-l, l,-l,
		 l,-l,-l,
		-l,-l, l
	};

	static const int indices[] = {0,2,1, 0,1,3, 1,2,3, 2,0,3};

	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	return Nv;
}

int addCube(Mesh& m){
	static const float l = sqrt(1./3);
	static const float vertices[] = {
		-l, l,-l,	 l, l,-l,	// 0  1
		-l,-l,-l,	 l,-l,-l,	// 2  3
			-l, l, l,	 l, l, l,	// 4  5
			-l,-l, l,	 l,-l, l,	// 6  7
	};

	static const int indices[] = {
		6,5,4, 6,7,5, 7,1,5, 7,3,1, 3,0,1, 3,2,0, 2,4,0, 2,6,4,
		4,1,0, 4,5,1, 2,3,6, 3,7,6
	};
	
	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);
	
	return Nv;
}

int addOctahedron(Mesh& m){
	static const float vertices[] = {
		 1,0,0, 0, 1,0, 0,0, 1,	// 0 1 2
		-1,0,0, 0,-1,0, 0,0,-1	// 3 4 5
	};

	static const int indices[] = {
		0,1,2, 1,3,2, 3,4,2, 4,0,2,
		1,0,5, 3,1,5, 4,3,5, 0,4,5
	};
	
	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	return Nv;
}

// Data taken from "Platonic Solids (Regular Polytopes In 3D)"
// http://local.wasp.uwa.edu.au/~pbourke/geometry/platonic/
int addDodecahedron(Mesh& m){
	static const float a = 1.6 * 0.5;
	static const float b = 1.6 / (2 * phi);
	static const float vertices[] = {
		 0, b,-a,	 b, a, 0,	-b, a, 0,	//  0  1  2
		 0, b, a,	 0,-b, a,	-a, 0, b,	//  3  4  5
		 a, 0, b,	 0,-b,-a,	 a, 0,-b,	//  6  7  8
		-a, 0,-b,	 b,-a, 0,	-b,-a, 0	//  9 10 11
	};

	static const int indices[] = {
		 1, 0, 2,	 2, 3, 1,	 4, 3, 5,	 6, 3, 4,
		 7, 0, 8,	 9, 0, 7,	10, 4,11,	11, 7,10,
		 5, 2, 9,	 9,11, 5,	 8, 1, 6,	 6,10, 8,
		 5, 3, 2,	 1, 3, 6,	 2, 0, 9,	 8, 0, 1,
		 9, 7,11,	10, 7, 8,	11, 4, 5,	 6, 4,10
	};
	
	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	return Nv;
}

int addIcosahedron(Mesh& m){
//	float b = 1. / phi;
//	float c = 2. - phi;
//	float vertices[] = {
//		 c,  0,  1,   -c,  0,  1,   -b,  b,  b,    0,  1,  c,    b,  b,  b,
//		-c,  0,  1,    c,  0,  1,    b, -b,  b,    0, -1,  c,   -b, -b,  b,
//		 c,  0, -1,   -c,  0, -1,   -b, -b, -b,    0, -1, -c,    b, -b, -b,
//		-c,  0, -1,    c,  0, -1,    b,  b, -b,    0,  1, -c,   -b,  b, -b,
//		 0,  1, -c,    0,  1,  c,    b,  b,  b,    1,  c,  0,    b,  b, -b,
//		 0,  1,  c,    0,  1, -c,   -b,  b, -b,   -1,  c,  0,   -b,  b,  b,
//		 0, -1, -c,    0, -1,  c,   -b, -b,  b,   -1, -c,  0,   -b, -b, -b,
//		 0, -1,  c,    0, -1, -c,    b, -b, -b,    1, -c,  0,    b, -b,  b,
//		 1,  c,  0,    1, -c,  0,    b, -b,  b,    c,  0,  1,    b,  b,  b,
//		 1, -c,  0,    1,  c,  0,    b,  b, -b,    c,  0, -1,    b, -b, -b,
//		-1,  c,  0,   -1, -c,  0,   -b, -b, -b,   -c,  0, -1,   -b,  b, -b,
//		-1, -c,  0,   -1,  c,  0,   -b,  b,  b,   -c,  0,  1,   -b, -b,  b
//	};
//
//	for(int i=0; i<Nv; i+=5){
//		Vec3f v1(vertices[3*i+ 0], vertices[3*i+ 1], vertices[3*i+ 2]);
//		Vec3f v2(vertices[3*i+ 3], vertices[3*i+ 4], vertices[3*i+ 5]);
//		Vec3f v3(vertices[3*i+ 6], vertices[3*i+ 7], vertices[3*i+ 8]);
//		Vec3f v4(vertices[3*i+ 9], vertices[3*i+10], vertices[3*i+11]);
//		Vec3f v5(vertices[3*i+12], vertices[3*i+13], vertices[3*i+14]);
//		
//		Vec3f vc = (v1+v2+v3+v4+v5)/5;
//		
//		plato5.vertex(v1);
//	}

	static const float vertices[] = {
		-0.57735, -0.57735, 0.57735,
		0.934172,  0.356822, 0,
		0.934172, -0.356822, 0,
		-0.934172, 0.356822, 0,
		-0.934172, -0.356822, 0,
		0,  0.934172,  0.356822,
		0,  0.934172,  -0.356822,
		0.356822,  0,  -0.934172,
		-0.356822,  0,  -0.934172,
		0,  -0.934172,  -0.356822,
		0,  -0.934172,  0.356822,
		0.356822,  0,  0.934172,
		-0.356822,  0,  0.934172,
		0.57735,  0.57735,  -0.57735,
		0.57735,  0.57735, 0.57735,
		-0.57735,  0.57735,  -0.57735,
		-0.57735,  0.57735,  0.57735,
		0.57735,  -0.57735,  -0.57735,
		0.57735,  -0.57735,  0.57735,
		-0.57735,  -0.57735,  -0.57735
	};

	static const int indices[] = {
		18, 2, 1,	11,18, 1,	14,11, 1,	 7,13, 1,	17, 7, 1,
		 2,17, 1,	19, 4, 3,	 8,19, 3,	15, 8, 3,	12,16, 3,
		 0,12, 3,	 4, 0, 3,	 6,15, 3,	 5, 6, 3,	16, 5, 3,
		 5,14, 1,	 6, 5, 1,	13, 6, 1,	 9,17, 2,	10, 9, 2,
		18,10, 2,	10, 0, 4,	 9,10, 4,	19, 9, 4,	19, 8, 7,
		 9,19, 7,	17, 9, 7,	 8,15, 6,	 7, 8, 6,	13, 7, 6,
		11,14, 5,	12,11, 5,	16,12, 5,	12, 0,10,	11,12,10,
		18,11,10
	};

	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);
	
	return Nv;
}

}