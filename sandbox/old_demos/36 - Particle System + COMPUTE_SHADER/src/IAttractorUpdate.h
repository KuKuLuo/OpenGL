#ifndef _IATTRACTORUPDATE_
#define _IATTRACTORUPDATE_

#include "Classes.h"
#include "GLFWInput.h"

template <class Matrix, class Vector, class Value> struct IAttractorUpdate
{
	virtual ~IAttractorUpdate() {};
	virtual void updateAttractor(AttractorModel<Vector, Value>* attModel, const Camera<Matrix, Vector, Value>* camera, GLFWInput* Input) = 0;
};


#endif