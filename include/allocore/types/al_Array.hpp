/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	C++ wrapper and utilities for AlloArray

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#ifndef INCLUDE_ALLO_ARRAY_HPP
#define INCLUDE_ALLO_ARRAY_HPP 1

#include <stdlib.h>
#include <strings.h>
#include "allocore/types/al_Array.h"
#include "allocore/math/al_Functions.hpp"
#include "allocore/math/al_Vec.hpp"

#define AL_ARRAY_DEFAULT_ALIGNMENT (4)

#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )

namespace al {


/// Object-oriented interface to AlloArray
class Array : public AlloArray {
public:

	/// Empty constructor defines a 0-dimensional, 1-component array of void type; unallocated data
	Array();

	/// Construct 1-dimensional array
	Array(int components, AlloTy ty, uint32_t dimx);

	/// Construct 2-dimensional array
	Array(int components, AlloTy ty, uint32_t dimx, uint32_t dimy);

	/// Construct 3-dimensional array
	Array(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz);

	///	Copy constructor; copies both the layout and data from cpy
	explicit Array(const AlloArray& cpy);
	explicit Array(const AlloArrayHeader& h2);

	//~Array(){ allo_array_destroy(this); }
	~Array(){ dataFree(); }


	/// Assignment operator copies format and data (allocates memory if necessary)
	Array& operator= (const AlloArray& cpy);


	/// Get type of elements
	AlloTy type() const { return header.type; }

	/// Verify elements are a particular type
	bool isType(AlloTy ty) const { return header.type == ty; }
	template<typename T> bool isType() { return isType(type<T>()); }

	uint8_t components() const { return header.components; }	///< Get number of components
	uint8_t dimcount() const { return header.dimcount; }		///< Get number of dimensions
	uint32_t dim(int i=0) const { return header.dim[i]; }		///< Get size of dimension
	unsigned width() const { return header.dim[0]; }			///< Get size of first dimension
	unsigned height() const { return header.dim[1]; }			///< Get size of second dimension
	unsigned depth() const { return header.dim[2]; }			///< Get size of third dimension
	uint32_t stride(int i=0) const { return header.stride[i]; }	///< Get stride of dimension, in elements

	/// Return the packing alignment (1, 2, 4 or 8 byte)
	uint32_t alignment() const {
		uint32_t i = stride(1);	// row stride
		if (i % 2 > 0) return 1;
		if (i % 4 > 0) return 2;
		if (i % 8 > 0) return 4;
		return 8;
	}

	/// Returns the total memory footprint, in bytes
	size_t size() const { return allo_array_size(this); }
	
	/// Returns number of cells in the Array:
	unsigned cells() const { return allo_array_elements(this); }

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(const AlloArrayHeader& h2);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(const AlloArray& array) { format(array.header); }

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(int components, AlloTy ty, uint32_t dimx);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(int components, AlloTy ty, uint32_t dimx, uint32_t dimy);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void formatAligned(int components, AlloTy ty, uint32_t dimx, size_t align);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void formatAligned(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, size_t align);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void formatAligned(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, size_t align);

	/// Check if this Array conforms to an ArrayHeader format
	bool isFormat(const AlloArrayHeader& h2) const;
	bool isFormat(const Array& src) const { return isFormat(src.header); }

	/// Returns true if Array contains data, false otherwise
	bool hasData() const { return NULL != data.ptr; }

	/// Allocate memory for the given header.
	/// Warning: does not check if memory was already allocated;
	/// Call dataFree() first if you know it will be safe to do so.
	void dataCalloc() { allo_array_allocate(this); }

	/// Free memory and set data.ptr to NULL
	void dataFree() { allo_array_free(this); }

	/// Set all data to zero
	void zero() { if(hasData()) bzero(data.ptr, size()); }


	/// Get mutable component using 1-D index
	template <class T> T& elem(size_t ic, size_t ix){
		return cell<T>(ix)[ic]; }

	/// Get mutable component using 2-D index
	template <class T> T& elem(size_t ic, size_t ix, size_t iy){
		return cell<T>(ix,iy)[ic]; }

	/// Get mutable component using 3-D index
	template <class T> T& elem(size_t ic, size_t ix, size_t iy, size_t iz){
		return cell<T>(ix,iy,iz)[ic]; }

	/// Get const component using 1-D index
	template <class T> const T& elem(size_t ic, size_t ix) const{
		return cell<T>(ix)[ic]; }

	/// Get const component using 2-D index
	template <class T> const T& elem(size_t ic, size_t ix, size_t iy) const{
		return cell<T>(ix,iy)[ic]; }

	/// Get const component using 3-D index
	template <class T> const T& elem(size_t ic, size_t ix, size_t iy, size_t iz) const{
		return cell<T>(ix,iy,iz)[ic]; }


	/// Fill with the same cell value throughout
	template<typename T> void set1d(T * cell);
	template<typename T> void set2d(T * cell);
	template<typename T> void set3d(T * cell);

	/// Use a pure C function to fill an array with data
	template<typename T> void fill(void (*func)(T * values, double normx));
	template<typename T> void fill(void (*func)(T * values, double normx, double normy));
	template<typename T> void fill(void (*func)(T * values, double normx, double normy, double normz));

	// TODO: iterators!

	/// Get the components at a given index in the array (no bounds checking)
	template<typename T> T * cell(size_t x) const;
	template<typename T> T * cell(size_t x, size_t y) const;
	template<typename T> T * cell(size_t x, size_t y, size_t z) const;

	template<typename T, typename TP> T * cell(T* val, const Vec<2,TP> p) const { return cell(val, p[0], p[1]); }
	template<typename T, typename TP> T * cell(T* val, const Vec<3,TP> p) const { return cell(val, p[0], p[1], p[2]); }

	/// Read the component values from array into val array (no bounds checking)
	template<typename T> void read(T* val, int x) const;
	template<typename T> void read(T* val, int x, int y) const;
	template<typename T> void read(T* val, int x, int y, int z) const;

	template<typename T, typename TP> void read(T* val, const Vec<2,TP> p) const { read(val, p[0], p[1]); }
	template<typename T, typename TP> void read(T* val, const Vec<3,TP> p) const { read(val, p[0], p[1], p[2]); }

	/// Read the component values from array into val array (wraps periodically at bounds)
	template<typename T> void read_wrap(T* val, int x) const;
	template<typename T> void read_wrap(T* val, int x, int y) const;
	template<typename T> void read_wrap(T* val, int x, int y, int z) const;

	template<typename T, typename TP> void read_wrap(T* val, const Vec<2,TP> p) const { read_wrap(val, p[0], p[1]); }
	template<typename T, typename TP> void read_wrap(T* val, const Vec<3,TP> p) const { read_wrap(val, p[0], p[1], p[2]); }

	/// Linear interpolated lookup (virtual array index)

	/// Reads the linearly interpolated component values into val array
	///
	template<typename T> void read_interp(T * val, double x) const;
	template<typename T> void read_interp(T * val, double x, double y) const;
	template<typename T> void read_interp(T * val, double x, double y, double z) const;

	template<typename T, typename TP> void read_interp(T* val, const Vec<2,TP> p) const { read_interp(val, p[0], p[1]); }
	template<typename T, typename TP> void read_interp(T* val, const Vec<3,TP> p) const { read_interp(val, p[0], p[1], p[2]); }

	/// Write component values from val array into array (no bounds checking)
	template<typename T> void write(const T* val, int x);
	template<typename T> void write(const T* val, int x, int y);
	template<typename T> void write(const T* val, int x, int y, int z);

	template<typename T, typename TP> void write(const T* val, const Vec<2,TP> p) { write(val, p[0], p[1]); }
	template<typename T, typename TP> void write(const T* val, const Vec<3,TP> p) { write(val, p[0], p[1], p[2]); }

	/// Write plane values from val array into array (wraps periodically at bounds)
	template<typename T> void write_wrap(const T* val, int x);
	template<typename T> void write_wrap(const T* val, int x, int y);
	template<typename T> void write_wrap(const T* val, int x, int y, int z);

	template<typename T, typename TP> void write_wrap(const T* val, const Vec<2,TP> p) { write_wrap(val, p[0], p[1]); }
	template<typename T, typename TP> void write_wrap(const T* val, const Vec<3,TP> p) { write_wrap(val, p[0], p[1], p[2]); }

	/// Linear interpolated write (virtual array index)

	/// AKA trilinear splat
	/// writes the linearly interpolated plane values from val array into array
	template<typename T> void write_interp(const T* val, double x);
	template<typename T> void write_interp(const T* val, double x, double y);
	template<typename T> void write_interp(const T* val, double x, double y, double z);

	template<typename T, typename TP> void write_interp(const T* val, const Vec<2,TP> p) { write_interp(val, p[0], p[1]); }
	template<typename T, typename TP> void write_interp(const T* val, const Vec<3,TP> p) { write_interp(val, p[0], p[1], p[2]); }

	/// Print array information
	void print() const;



	///	Returns the type enumeration ID (AlloTy) for a given type (given as template argument).

	/// E.g., assert(Array::type<float>() == AlloFloat32Ty);
	///
	template<typename T> static AlloTy type();

	///	Derive the appropriate stride values for a given alignment
	static void deriveStride(AlloArrayHeader& h, size_t alignSize);

protected:
	void formatAlignedGeneral(int comps, AlloTy ty, uint32_t * dims, int numDims, size_t align);
public:	// temporarily made public, because protected broke some other project code -gw
	Array(const Array&);
	Array& operator= (const Array&);
protected:

	// temporary hack because the one in al_Function gave a bad result
	// for e.g. wrap<double>(-64.0, -32.0);
	template<typename T>
	static T wrap(T v, const T hi=T(1.), const T lo=T(0.)){
		if(lo == hi) return lo;
		//if(v >= hi){
		if(!(v < hi)){
			T diff = hi - lo;
			v -= diff;
			if(!(v < hi)) v -= diff * (T)(uint32_t)((v - lo)/diff);
		}
		else if(v < lo){
			T diff = hi - lo;
			v += diff;
			if(v < lo) v += diff * (T)(uint32_t)(((lo - v)/diff) + 1);
			if(v==diff) return lo;
		}
		return v;
	}
};










/*
 *
 ********* INLINE IMPLEMENTATION BELOW ***********
 *
 */
#pragma mark --------------------------------------


/*
	Type traits by partial specialization:
 */
template<> inline AlloTy Array::type<uint8_t  >(){ return AlloUInt8Ty; }
template<> inline AlloTy Array::type<uint16_t >(){ return AlloUInt16Ty; }
template<> inline AlloTy Array::type<uint32_t >(){ return AlloUInt32Ty; }
template<> inline AlloTy Array::type<uint64_t >(){ return AlloUInt64Ty; }
template<> inline AlloTy Array::type<int8_t   >(){ return AlloSInt8Ty; }
template<> inline AlloTy Array::type<int16_t  >(){ return AlloSInt16Ty; }
template<> inline AlloTy Array::type<int32_t  >(){ return AlloSInt32Ty; }
template<> inline AlloTy Array::type<int64_t  >(){ return AlloSInt64Ty; }
template<> inline AlloTy Array::type<float    >(){ return AlloFloat32Ty; }
template<> inline AlloTy Array::type<double   >(){ return AlloFloat64Ty; }
template<> inline AlloTy Array::type<AlloArray>(){ return AlloArrayTy; }
template<> inline AlloTy Array::type<void *>(){
	switch(sizeof(void *)) {
		case 4: return AlloPointer32Ty;
		case 8: return AlloPointer64Ty;
	}
	return 0;
}

inline Array::Array(){
	data.ptr = NULL;
	header.type= 0;
	header.components = 1;
	header.dimcount = 0;
	for(int i=0; i<ALLO_ARRAY_MAX_DIMS; ++i) header.dim[i]=0;
}

inline Array::Array(const AlloArray& cpy){
	(*this) = cpy;
}
inline Array::Array(const Array& cpy) {
    (*this) = cpy;
}
inline Array::Array(const AlloArrayHeader& h2){
	allo_array_clear(this);
	format(h2);
}

inline Array::Array(int comps, AlloTy ty, uint32_t dimx){
	allo_array_clear(this);
	format(comps, ty, dimx);
}

inline Array::Array(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy){
	allo_array_clear(this);
	format(comps, ty, dimx, dimy);
}

inline Array::Array(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz){
	allo_array_clear(this);
	format(comps, ty, dimx, dimy, dimz);
}

inline Array& Array::operator= (const Array& cpy) {
	if(&cpy != this){
		format(cpy.header);
		if (cpy.data.ptr) {
			memcpy(data.ptr, cpy.data.ptr, size());
		}
	}
	return *this;
}

inline Array& Array::operator= (const AlloArray& cpy) {
	if(&cpy != this){
		format(cpy.header);
		if (cpy.data.ptr) {
			memcpy(data.ptr, cpy.data.ptr, size());
		}
	}
	return *this;
}


/*
	Set stride factors based on a specific byte alignment
 */
inline void Array::deriveStride(AlloArrayHeader& h, size_t alignSize) {
	allo_array_setstride(&h, alignSize);
}

// Check if this Array conforms to an ArrayHeader format
inline bool Array::isFormat(const AlloArrayHeader& h2) const {
	for(int i=0; i<header.dimcount; ++i){
		if( (header.dim[i] != h2.dim[i]) || (header.stride[i] != h2.stride[i]) )
			return false;
	}
	return (header.components == h2.components)
		&& (header.type == h2.type)
		&& (header.dimcount == h2.dimcount);
}

inline void Array::format(const AlloArrayHeader& h2) {
	if(!isFormat(h2)) {
		dataFree();
		header.type = h2.type;
		header.components = h2.components;
		header.dimcount = h2.dimcount;
		for(int i=0; i < ALLO_ARRAY_MAX_DIMS; ++i) {
			if (i < header.dimcount) {
				header.dim[i] = h2.dim[i];
				header.stride[i] = h2.stride[i];
			} else {
				header.dim[i] = 1;
				header.stride[i] = h2.stride[i-1];
			}
		}
		//printf("reformatted array "); print();
		dataCalloc();
	}
}

inline void Array::format(int comps, AlloTy ty, uint32_t dimx) {
	formatAligned(comps, ty, dimx, AL_ARRAY_DEFAULT_ALIGNMENT);
}
inline void Array::format(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy) {
	formatAligned(comps, ty, dimx, dimy, AL_ARRAY_DEFAULT_ALIGNMENT);
}
inline void Array::format(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz) {
	formatAligned(comps, ty, dimx, dimy, dimz, AL_ARRAY_DEFAULT_ALIGNMENT);
}

inline void Array::formatAligned(int comps, AlloTy ty, uint32_t dimx, size_t align) {
	uint32_t dims[] = {dimx};
	formatAlignedGeneral(comps, ty, dims,1, align);
}
inline void Array::formatAligned(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, size_t align) {
	uint32_t dims[] = {dimx,dimy};
	formatAlignedGeneral(comps, ty, dims,2, align);
}
inline void Array::formatAligned(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, size_t align) {
	uint32_t dims[] = {dimx,dimy,dimz};
	formatAlignedGeneral(comps, ty, dims,3, align);
}

inline void Array::formatAlignedGeneral(int comps, AlloTy ty, uint32_t * dims, int numDims, size_t align) {
	if(numDims > ALLO_ARRAY_MAX_DIMS) numDims = ALLO_ARRAY_MAX_DIMS;
	AlloArrayHeader hh;
	hh.type = ty;
	hh.components = comps;
	hh.dimcount = numDims;
	int i=0;
	for(; i<numDims; ++i)				hh.dim[i] = dims[i];
	for(; i<ALLO_ARRAY_MAX_DIMS; ++i)	hh.dim[i] = 0;
	deriveStride(hh, align);
	format(hh);
}

template<typename T> inline T * Array::cell(size_t x) const {
	size_t fieldstride_x = header.stride[0];
	return (T *)(data.ptr + x*fieldstride_x);
}
template<typename T> inline T * Array::cell(size_t x, size_t y) const {
	size_t fieldstride_x = header.stride[0];
	size_t fieldstride_y = header.stride[1];
	return (T *)(data.ptr + x*fieldstride_x + y*fieldstride_y);
}
template<typename T> inline T * Array::cell(size_t x, size_t y, size_t z) const {
	size_t fieldstride_x = header.stride[0];
	size_t fieldstride_y = header.stride[1];
	size_t fieldstride_z = header.stride[2];
	return (T *)(data.ptr + x*fieldstride_x + y*fieldstride_y + z*fieldstride_z);
}


// read the plane values from array into val array (no bounds checking)
template<typename T> inline void Array::read(T* val, int x) const {
	T * paaa = cell<T>(x);
	for (uint8_t p=0; p<header.components; p++) {
		val[p] = paaa[p];
	}
}
template<typename T> inline void Array::read(T* val, int x, int y) const {
	T * paaa = cell<T>(x, y);
	for (uint8_t p=0; p<header.components; p++) {
		val[p] = paaa[p];
	}
}
template<typename T> inline void Array::read(T* val, int x, int y, int z) const {
	T * paaa = cell<T>(x, y, z);
	for (uint8_t p=0; p<header.components; p++) {
		val[p] = paaa[p];
	}
}

// read the plane values from array into val array (wraps periodically at bounds)
template<typename T> inline void Array::read_wrap(T* val, int x) const {
	read(val, wrap<int>(x, header.dim[0], 0));
}
template<typename T> inline void Array::read_wrap(T* val, int x, int y) const {
	read(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0));
}
template<typename T> inline void Array::read_wrap(T* val, int x, int y, int z) const {
	read(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0), wrap<int>(z, header.dim[2], 0));
}

// linear interpolated lookup (virtual array index)
// reads the linearly interpolated plane values into val array
template<typename T> inline void Array::read_interp(T * val, double x) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	const unsigned xa = (const unsigned)DOUBLE_FLOOR(x);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double faaa = DOUBLE_FRAC(x);
	double fbaa = 1.f - faaa;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa);
	T * pbaa = cell<T>(xb);
	// for each plane of the field, do the interp:
	for (uint8_t p=0; p<header.components; p++) {
		val[p] =	(paaa[p] * faaa) + (pbaa[p] * fbaa);
	}
}

template<typename T> inline void Array::read_interp(T * val, double x, double y) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	const unsigned xa = (const unsigned)DOUBLE_FLOOR(x);
	const unsigned ya = (const unsigned)DOUBLE_FLOOR(y);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
	double yaf = 1.f - ybf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf;
	double faba = xaf * ybf;
	double fbaa = xbf * yaf;
	double fbba = xbf * ybf;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa, ya);
	T * paba = cell<T>(xa, yb);
	T * pbaa = cell<T>(xb, ya);
	T * pbba = cell<T>(xb, yb);
	// for each plane of the field, do the interp:
	for (uint8_t p=0; p<header.components; p++) {
		val[p] =	(paaa[p] * faaa) +
		(pbaa[p] * fbaa) +
		(paba[p] * faba) +
		(pbba[p] * fbba);
	}
}

template<typename T> inline void Array::read_interp(T * val, double x, double y, double z) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	z = wrap<double>(z, (double)header.dim[2], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	const unsigned xa = (const unsigned)DOUBLE_FLOOR(x);
	const unsigned ya = (const unsigned)DOUBLE_FLOOR(y);
	const unsigned za = (const unsigned)DOUBLE_FLOOR(z);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	unsigned zb = za+1;	if (zb == header.dim[2]) zb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
	double yaf = 1.f - ybf;
	double zbf = DOUBLE_FRAC(z);
	double zaf = 1.f - zbf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf * zaf;
	double faab = xaf * yaf * zbf;
	double faba = xaf * ybf * zaf;
	double fabb = xaf * ybf * zbf;
	double fbaa = xbf * yaf * zaf;
	double fbab = xbf * yaf * zbf;
	double fbba = xbf * ybf * zaf;
	double fbbb = xbf * ybf * zbf;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa, ya, za);
	T * paab = cell<T>(xa, ya, zb);
	T * paba = cell<T>(xa, yb, za);
	T * pabb = cell<T>(xa, yb, zb);
	T * pbaa = cell<T>(xb, ya, za);
	T * pbab = cell<T>(xb, ya, zb);
	T * pbba = cell<T>(xb, yb, za);
	T * pbbb = cell<T>(xb, yb, zb);
	// for each plane of the field, do the 3D interp:
	for (size_t p=0; p<header.components; p++) {
		val[p] =	(paaa[p] * faaa) +
					(pbaa[p] * fbaa) +
					(paba[p] * faba) +
					(paab[p] * faab) +
					(pbab[p] * fbab) +
					(pabb[p] * fabb) +
					(pbba[p] * fbba) +
					(pbbb[p] * fbbb);
	}
}

// write plane values from val array into array (no bounds checking)
template<typename T> inline void Array::write(const T* val, int x) {
	T * paaa = cell<T>(x);
	for (uint8_t p=0; p<header.components; ++p) {
		paaa[p] = val[p];
	}
}
template<typename T> inline void Array::write(const T* val, int x, int y) {
	T * paaa = cell<T>(x, y);
	for (uint8_t p=0; p<header.components; ++p) {
		paaa[p] = val[p];
	}
}
template<typename T> inline void Array::write(const T* val, int x, int y, int z) {
	T * paaa = cell<T>(x, y, z);
	for (uint8_t p=0; p<header.components; ++p) {
		paaa[p] = val[p];
	}
}

// write plane values from val array into array (wraps periodically at bounds)
template<typename T> inline void Array::write_wrap(const T* val, int x) {
	write(val, wrap<int>(x, header.dim[0], 0));
}
template<typename T> inline void Array::write_wrap(const T* val, int x, int y) {
	write(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0));
}
template<typename T> inline void Array::write_wrap(const T* val, int x, int y, int z) {
	write(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0), wrap<int>(z, header.dim[2], 0));
}

// linear interpolated write (virtual array index)
// writes the linearly interpolated plane values from val array into array
template<typename T> inline void Array::write_interp(const T* val, double x) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	const unsigned xa = (const unsigned)DOUBLE_FLOOR(x);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	// get the interpolation corner weights:
	double faaa = xaf;
	double fbaa = xbf;
	T * paaa = cell<T>(xa);
	T * pbaa = cell<T>(xb);
	// for each plane of the field, do the 3D interp:
	for (uint8_t p=0; p<header.components; p++) {
		T tmp = val[p];
		paaa[p] += tmp * faaa;
		pbaa[p] += tmp * fbaa;
	}
}
template<typename T> inline void Array::write_interp(const T* val, double x, double y) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	const unsigned xa = (const unsigned)DOUBLE_FLOOR(x);
	const unsigned ya = (const unsigned)DOUBLE_FLOOR(y);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
	double yaf = 1.f - ybf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf;
	double faba = xaf * ybf;
	double fbaa = xbf * yaf;
	double fbba = xbf * ybf;
	T * paaa = cell<T>(xa, ya);
	T * paba = cell<T>(xa, yb);
	T * pbaa = cell<T>(xb, ya);
	T * pbba = cell<T>(xb, yb);
	// for each plane of the field, do the 3D interp:
	for (uint8_t p=0; p<header.components; p++) {
		T tmp = val[p];
		paaa[p] += tmp * faaa;
		paba[p] += tmp * faba;
		pbaa[p] += tmp * fbaa;
		pbba[p] += tmp * fbba;
	}
}

template<typename T> inline void Array::write_interp(const T* val, double x0, double y0, double z0) {
	double x = wrap<double>(x0, (double)header.dim[0], 0.);
	double y = wrap<double>(y0, (double)header.dim[1], 0.);
	double z = wrap<double>(z0, (double)header.dim[2], 0.);
	const unsigned xa = (const unsigned)DOUBLE_FLOOR(x);
	const unsigned ya = (const unsigned)DOUBLE_FLOOR(y);
	const unsigned za = (const unsigned)DOUBLE_FLOOR(z);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	unsigned zb = za+1;	if (zb == header.dim[2]) zb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
	double yaf = 1.f - ybf;
	double zbf = DOUBLE_FRAC(z);
	double zaf = 1.f - zbf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf * zaf;
	double faab = xaf * yaf * zbf;
	double faba = xaf * ybf * zaf;
	double fabb = xaf * ybf * zbf;
	double fbaa = xbf * yaf * zaf;
	double fbab = xbf * yaf * zbf;
	double fbba = xbf * ybf * zaf;
	double fbbb = xbf * ybf * zbf;
	T * paaa = cell<T>(xa, ya, za);
	T * paab = cell<T>(xa, ya, zb);
	T * paba = cell<T>(xa, yb, za);
	T * pabb = cell<T>(xa, yb, zb);
	T * pbaa = cell<T>(xb, ya, za);
	T * pbab = cell<T>(xb, ya, zb);
	T * pbba = cell<T>(xb, yb, za);
	T * pbbb = cell<T>(xb, yb, zb);
	// for each plane of the field, do the 3D interp:
	for (uint8_t p=0; p<header.components; p++) {
		T tmp = val[p];
		paaa[p] += tmp * faaa;
		paab[p] += tmp * faab;
		paba[p] += tmp * faba;
		pabb[p] += tmp * fabb;
		pbaa[p] += tmp * fbaa;
		pbab[p] += tmp * fbab;
		pbba[p] += tmp * fbba;
		pbbb[p] += tmp * fbbb;
	}
}


template<typename T> inline void Array::fill(void (*func)(T * values, double normx)) {
	int d0 = header.dim[0];
	double inv_d0 = 1.0/(double)d0;
	int components = header.components;

	T *vals = (T *)(data.ptr);
	for(int x=0; x < d0; x++) {
		func(vals, inv_d0 * x);
		vals += components;
	}
}

template<typename T> inline void Array::fill(void (*func)(T * values, double normx, double normy)) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int s1 = header.stride[1];
	double inv_d0 = 1.0/(double)d0;
	double inv_d1 = 1.0/(double)d1;
	int components = header.components;

	for(int y=0; y < d1; y++) {
		T *vals = (T *)(data.ptr + s1*y);
		for(int x=0; x < d0; x++) {
			func(vals, inv_d0 * x, inv_d1 * y);
			vals += components;
		}
	}
}

template<typename T> inline void Array::fill(void (*func)(T * values, double normx, double normy, double normz)) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int d2 = header.dim[2];
	int s1 = header.stride[1];
	int s2 = header.stride[2];
	double inv_d0 = 1.0/(double)d0;
	double inv_d1 = 1.0/(double)d1;
	double inv_d2 = 1.0/(double)d2;
	int components = header.components;

	for(int z=0; z < d1; z++) {
		for(int y=0; y < d1; y++) {
			T *vals = (T *)(data.ptr + s1*y + s2*z);
			for(int x=0; x < d0; x++) {
				func(vals, inv_d0 * x, inv_d1 * y, inv_d2 * z);
				vals += components;
			}
		}
	}
}

template<typename T> inline void Array::set1d(T * cell) {
	int d0 = header.dim[0];
	int s0 = header.stride[0];
	int components = header.components;

	for(int x=0; x < d0; x++) {
		T *vals = (T *)(data.ptr + s0*x);
		for (int i=0; i<components; i++) {
			vals[i] = cell[i];
		}
	}
}


template<typename T> inline void Array::set2d(T * cell) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int s0 = header.stride[0];
	int s1 = header.stride[1];
	int components = header.components;

	for(int y=0; y < d1; y++) {
		for(int x=0; x < d0; x++) {
			T *vals = (T *)(data.ptr + s0*x + s1*y);
			for (int i=0; i<components; i++) {
				vals[i] = cell[i];
			}
		}
	}
}

template<typename T> inline void Array::set3d(T * cell) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int d2 = header.dim[2];
	int s0 = header.stride[0];
	int s1 = header.stride[1];
	int s2 = header.stride[2];
	int components = header.components;

	for(int z=0; z < d2; z++) {
		for(int y=0; y < d1; y++) {
			for(int x=0; x < d0; x++) {
				T *vals = (T *)(data.ptr + s0*x + s1*y + s2*z);
				for (int i=0; i<components; i++) {
					vals[i] = cell[i];
				}
			}
		}
	}
}

inline void Array::print() const {
	printf("Array %p type %s components %d %d-D: ( ", this, allo_type_name(type()), components(), dimcount());
	for (int i=0; i<dimcount(); i++) printf("%d(stride %d) ", dim(i), stride(i));
	printf(") %d bytes, data: %p)\n", int(size()), data.ptr);
}

#undef DOUBLE_FLOOR
#undef DOUBLE_CEIL
#undef DOUBLE_FRAC

} // ::al::

#endif /* INCLUDE_ALLO_TYPES_CPP_H */
