#ifndef __DCPU_COMP_TYPES_POINTER16_H
#define __DCPU_COMP_TYPES_POINTER16_H

#include <AsmGenerator.h>
#include <stdexcept>
#include <CompilerException.h>
#include <stdint.h>
#include <nodes/IType.h>

#include "TUint16.h"

class TPointer16 : public TUint16
{
	private:
		IType* m_pointingTo;

	public:
		/* inherits all Uint16 operations */
		
		TPointer16(IType* pointingTo) : m_pointingTo(pointingTo) { }
		
		IType* getPointerBaseType();
		
		virtual std::string getName() const;
};
#endif
