#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "allocore/system/al_Config.h"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Mesh& Mesh::reset() {
	vertices().reset();
	normals().reset();
	colors().reset();
	coloris().reset();
	texCoord2s().reset();
	texCoord3s().reset();
	indices().reset();
	return *this;
}

void Mesh::decompress(){
	int Ni = indices().size();
	if(Ni){
		#define DECOMPRESS(buf, Type)\
		{\
			int N = buf.size();\
			if(N){\
				std::vector<Type> old(N);\
				std::copy(&buf[0], (&buf[0]) + N, old.begin());\
				buf.size(Ni);\
				for(int i=0; i<Ni; ++i)	buf[i] = old[indices()[i]];\
			}\
		}
		DECOMPRESS(vertices(), Vertex)
		DECOMPRESS(colors(), Color)
		DECOMPRESS(coloris(), Color)
		DECOMPRESS(normals(), Normal)
		DECOMPRESS(texCoord2s(), TexCoord2)
		DECOMPRESS(texCoord3s(), TexCoord3)
		#undef DECOMPRESS
		
		indices().reset();
	}
}

void Mesh::equalizeBuffers() {
	const int Nv = vertices().size();
	const int Nn = normals().size();
	const int Nc = colors().size();
	const int Nci= coloris().size();
	const int Nt2= texCoord2s().size();
	const int Nt3= texCoord3s().size();

	if(Nn){
		for(int i=Nn; i<Nv; ++i){
			normals().append(normals()[Nn-1]);
		}
	}
	if(Nc){
		for(int i=Nc; i<Nv; ++i){
			colors().append(colors()[Nc-1]);
		}
	}
	else if(Nci){
		for(int i=Nci; i<Nv; ++i){
			coloris().append(coloris()[Nci-1]);
		}
	}
	if(Nt2){
		for(int i=Nt2; i<Nv; ++i){
			texCoord2s().append(texCoord2s()[Nt2-1]);
		}
	}
	if(Nt3){
		for(int i=Nt3; i<Nv; ++i){
			texCoord3s().append(texCoord3s()[Nt3-1]);
		}
	}
}

class TriFace {
public:
	Mesh::Vertex vertices[3];
	Vec3f norm;

	TriFace(const TriFace& cpy)
	: norm(cpy.norm) {
		vertices[0] = cpy.vertices[0];
		vertices[1] = cpy.vertices[1];
		vertices[2] = cpy.vertices[2];
	}
	TriFace(Mesh::Vertex p0, Mesh::Vertex p1, Mesh::Vertex p2)
	{
		vertices[0] = p0;
		vertices[1] = p1;
		vertices[2] = p2;
		// calculate norm for this face:
		normal<float>(norm, p0, p1, p2);
	}
};
void Mesh::createNormalsMesh(Mesh& mesh, float length, bool perFace){

	struct F{
		static void initMesh(Mesh& m, int n){
			m.vertices().size(n*2);
			m.reset();
			m.primitive(Graphics::LINES);		
		}
	};

	if (perFace) {	
		// compute vertex based normals
		if(indices().size()){

			int Ni = indices().size();
			Ni = Ni - (Ni%3); // must be multiple of 3
			F::initMesh(mesh, (Ni/3)*2);

			for(int i=0; i<Ni; i+=3){
				Index i1 = indices()[i+0];
				Index i2 = indices()[i+1];
				Index i3 = indices()[i+2];
				const Vertex& v1 = vertices()[i1];
				const Vertex& v2 = vertices()[i2];
				const Vertex& v3 = vertices()[i3];
				
				// get mean:
				const Vertex mean = (v1 + v2 + v3)/3.f;
				
				// get face normal:
				Vertex facenormal = cross(v2-v1, v3-v1);
				facenormal.normalize();
				
				mesh.vertex(mean);
				mesh.vertex(mean + (facenormal*length));
			}
		} else {
			printf("createNormalsMesh only valid for indexed meshes\n");
		} 
	} else {
		int Ni = al::min(vertices().size(), normals().size());
		F::initMesh(mesh, Ni*2);
		
		for(int i=0; i<Ni; ++i){
			const Vertex& v = vertices()[i];
			mesh.vertex(v);
			mesh.vertex(v + normals()[i]*length);
		}
	}
}

void Mesh::invertNormals() {
	int Nv = normals().size();
	for(int i=0; i<Nv; ++i) normals()[i] = -normals()[i];
}

void Mesh::compress() {

	int Ni = indices().size();
	int Nv = vertices().size();
	if (Ni) {
		printf("cannot compress Mesh with indices\n");
		return;
	}
	if (Nv == 0) {
		printf("cannot compress Mesh with no vertices\n");
		return;
	}
	
	int Nc = colors().size();
	int Nci = coloris().size();
	int Nn = normals().size();
	int Nt2 = texCoord2s().size();
	int Nt3 = texCoord3s().size();

	// map tree to uniquely ID vertices with same values:
	typedef std::map<float, int> Zmap;
	typedef std::map<float, Zmap> Ymap;
	typedef std::map<float, Ymap> Xmap;
	Xmap xmap;
	
	// copy current values:
	Mesh old(*this);
	
	// walk backward through the vertex list
	// create a ID for each one
	for (int i=vertices().size()-1; i>=0; i--) {
		Vertex& v = vertices()[i];
		xmap[v.x][v.y][v.z] = i;
	}
	
	// map of old vertex index to new vertex index:
	typedef std::map<int, int> Imap;
	Imap imap;
	
	
	// reset current mesh:
	reset();
	
	// walk forward, inserting if 
	for (int i=0; i<old.vertices().size(); i++) {
		Vertex& v = old.vertices()[i];
		int idx = xmap[v.x][v.y][v.z];
		Imap::iterator it = imap.find(idx);
		if (it != imap.end()) {
			// use existing
			index(it->second);
		} else {
			// create new
			int newidx = vertices().size();
			vertex(v);
			if (Nc) color(old.colors()[i]);
			if (Nci) colori(old.coloris()[i]);
			if (Nn) normal(old.normals()[i]);
			if (Nt2) texCoord(old.texCoord2s()[i]);
			if (Nt3) texCoord(old.texCoord3s()[i]);
			// store new index:
			imap[idx] = newidx;
			// use new index:
			index(newidx);
		}
	}
}

void Mesh::generateNormals(bool normalize, bool equalWeightPerFace) {
//	/*
//		Multi-pass algorithm:
//			generate a list of faces (assume triangles?)
//				(vary according to whether mIndices is used)
//			calculate normal per face (use normal<float>(dst, p0, p1, p2))
//			vertices may be used in multiple faces; their norm should be an average of the uses
//				easy enough if indices is being used; not so easy otherwise.
//					create a lookup table by hashing on vertex x,y,z
//
//
//			write avg into corresponding normals for each vertex
//				EXCEPT: if edge is sharper than @angle, just use the face normal directly
//	*/
//	std::vector<TriFace> faces;
//
//	std::map<std::string, int> vertexHash;
//
//	int Ni = indices().size();
//	int Nv = vertices().size();
//	if (Ni) {
//		for (int i=0; i<Ni;) {
//			TriFace face(
//				mVertices[mIndices[i++]],
//				mVertices[mIndices[i++]],
//				mVertices[mIndices[i++]]
//			);
//			faces.push_back(face);
//		}
//	} else {
//		for (int i=0; i<Nv;) {
//			TriFace face(
//				mVertices[i++],
//				mVertices[i++],
//				mVertices[i++]
//			);
//			faces.push_back(face);
//		}
//	}

	int Nv = vertices().size();

	// same number of normals as vertices
	normals().size(Nv);


	// compute vertex based normals
	if(indices().size()){

		for(int i=0; i<Nv; ++i) normals()[i].set(0,0,0);

		int Ni = indices().size();

//		if(primitive() == TRIANGLES){
			Ni = Ni - (Ni%3); // must be multiple of 3

			for(int i=0; i<Ni; i+=3){
				Index i1 = indices()[i+0];
				Index i2 = indices()[i+1];
				Index i3 = indices()[i+2];
				const Vertex& v1 = vertices()[i1];
				const Vertex& v2 = vertices()[i2];
				const Vertex& v3 = vertices()[i3];
				
				// MWAAT (mean weighted by areas of adjacent triangles)
				Vertex vn = cross(v2-v1, v3-v1);

				// MWE (mean weighted equally)
				if (equalWeightPerFace) vn.normalize();

				// MWA (mean weighted by angle)
				// This doesn't work well with dynamic marching cubes- normals
				// pop in and out for small triangles.
//				Vertex v12= v2-v1;
//				Vertex v13= v3-v1;
//				Vertex vn = cross(v12, v13).normalize();
//				vn *= angle(v12, v13) / M_PI;
				
				normals()[i1] += vn;
				normals()[i2] += vn;
				normals()[i3] += vn;
			}
//		}
//		else if(primitive() == TRIANGLE_STRIP){
//			for(int i=2; i<Ni; ++i){
//				Index i1 = indices()[i-2];
//				Index i2 = indices()[i-1];
//				Index i3 = indices()[i-0];
//				const Vertex& v1 = vertices()[i1];
//				const Vertex& v2 = vertices()[i2];
//				const Vertex& v3 = vertices()[i3];
//				
//				Vertex vn = cross(v2-v1, v3-v1);
//				
//				normals()[i1] += vn;
//				normals()[i2] += vn;
//				normals()[i3] += vn;
//			}
//		}

		// normalize the normals
		if(normalize) for(int i=0; i<Nv; ++i) normals()[i].normalize();
	}
	
	// compute face based normals
	else{
//		if(primitive() == TRIANGLES){
			int N = Nv - (Nv % 3);

			for(int i=0; i<N; i+=3){
				int i1 = i+0;
				int i2 = i+1;
				int i3 = i+2;
				const Vertex& v1 = vertices()[i1];
				const Vertex& v2 = vertices()[i2];
				const Vertex& v3 = vertices()[i3];
				
				Vertex vn = cross(v2-v1, v3-v1);
				if(normalize) vn.normalize();
				
				normals()[i1] = vn;
				normals()[i2] = vn;
				normals()[i3] = vn;
			}			
			
//		}
	}
}



Mesh& Mesh::repeatLast(){
	if(indices().size()){
		index(indices().last());
	}
	else{
		if(colors().size()) colors().repeatLast();
		else if(coloris().size()) coloris().repeatLast();
		if(vertices().size()) vertices().repeatLast();
		if(normals().size()) normals().repeatLast();
		if(texCoord2s().size()) texCoord2s().repeatLast();
		else if(texCoord3s().size()) texCoord3s().repeatLast();
	}
	return *this;
}


void Mesh::ribbonize(float * widths, int widthsStride, bool faceBinormal){

	const int N = mVertices.size();

	if(0 == N) return;

	mVertices.size(N*2);
	mNormals.size(N*2);

	// Store last vertex since it will be overwritten eventually
	const Vertex last = mVertices[N-1]; 
	
	int in = faceBinormal ? 2 : 1;
	int ib = faceBinormal ? 1 : 2;
	
	for(int i=N-1; i>=0; --i){
		int i1 = i;
		int i0 = i1-1; if(i0< 0) i0+=N;
		int i2 = i1+1; if(i2>=N) i2-=N;

		const Vertex& v0 = (i0==(N-1)) ? last : mVertices[i0];
		const Vertex& v1 = mVertices[i1];
		const Vertex& v2 = mVertices[i2];

		// compute Frenet frame
		Vertex f[3]; // T,N,B
		{
			const Vertex d1 = (v0 - v2)*0.5;
			const Vertex d2 = (d1 - v1)*2.0;
//			const Vertex d1 = (v0 - v2);
//			const Vertex d2 = (v0 - v1 - v1 + v2);
			//Vertex& t = f[0];
			Vertex& n = f[1];
			Vertex& b = f[2];
			b = cross(d2,d1).sgn();
			n = cross(d1, b).sgn();
			//t = d1.sgn(); // not used
		}
		f[ib] *= widths[i0*widthsStride];
		
		int i12 = i1<<1;
		// v1 is ref, so we must write in reverse to properly handle i=0
		mVertices[i12+1] = v1+f[ib];
		mVertices[i12  ] = v1-f[ib];

		mNormals [i12  ].set(f[in][0], f[in][1], f[in][2]);
		mNormals [i12+1] = mNormals[i12];
	}
	
	if(mColors.size()) mColors.expand<2,true>();
	if(mColoris.size()) mColoris.expand<2,true>();
}



void Mesh::merge(const Mesh& src){
//	if (indices().size() || src.indices().size()) {
//		printf("error: Mesh merging with indexed meshes not yet supported\n");
//		return;
//	}

	// TODO: only do merge if source and dest are well-formed
	// TODO: what to do when mixing float and integer colors? promote or demote?

	// TODO: indices are more complex, since the offsets may have changed.
	// we'd have to add indices.size() to all of the src.indices before adding.
	// also, both src & dst should either use or not use indices
	// tricky if src is empty...
	//indices().append(src.indices());

	// Source has indices, and I either do or don't.
	// After this block, I will have indices.
	if(src.indices().size()){
		Index Nv = vertices().size();
		Index Ni = indices().size();
		// If no indices, must create
		if(0 == Ni){
			for(Index i=0; i<Nv; ++i) index(i);
		}
		// Add source indices offset by my number of vertices
		index(src.indices().elems(), src.indices().size(), (unsigned int)Nv);
	}
	
	// Source doesn't have indices, but I do
	else if(indices().size()){
		int Nv = vertices().size();
		for(int i=Nv; i<Nv+src.vertices().size(); ++i) index(i);
	}
	
	// From here, the game is indice invariant

	//equalizeBuffers(); << TODO: must do this if we are using indices.
	vertices().append(src.vertices());
	normals().append(src.normals());
	colors().append(src.colors());
	texCoord2s().append(src.texCoord2s());
	texCoord3s().append(src.texCoord3s());
}


void Mesh::getBounds(Vertex& min, Vertex& max) const {
	if(vertices().size()){
		min.set(vertices()[0]);
		max.set(min);
		for(int v=1; v<vertices().size(); ++v){
			const Vertex& vt = vertices()[v];
			for(int i=0; i<3; ++i){
				min[i] = AL_MIN(min[i], vt[i]);
				max[i] = AL_MAX(max[i], vt[i]);
			}
		}
	}
}

Mesh::Vertex Mesh::getCenter() const {
	Vertex min(0), max(0);
	getBounds(min, max);
	return min+(max-min)*0.5;
}

void Mesh::unitize(bool proportional) {
	Vertex min(0), max(0);
	getBounds(min, max);
	// span of each axis:
	Vertex span = max-min;	// positive only
	// center of each axis:	
	Vertex mid = min + (span * 0.5);
	// axis scalar:
	Vertex scale(2./span.x, 2./span.y, 2./span.z);	// positive only
	
	// adjust to use scale of largest axis:
	if (proportional) {
		float s = al::min(scale.x, al::min(scale.y, scale.z));
		scale.x = scale.y = scale.z = s;
	}
	
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		vt = (vt-mid)*scale;
	}
}

Mesh& Mesh::translate(float x, float y, float z){
	const Vertex xfm(x,y,z);
	for(int i=0; i<vertices().size(); ++i)
		mVertices[i] += xfm;
	return *this;
}

Mesh& Mesh::scale(float x, float y, float z){
	const Vertex xfm(x,y,z);
	for(int i=0; i<vertices().size(); ++i)
		mVertices[i] *= xfm;
	return *this;
}

} // al::