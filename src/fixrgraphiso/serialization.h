// -*- C++ -*-
//
// Code used to serialize (read/write) objects
//
// Author: Sergio Mover
//


#ifndef SERIALIZATION_H_INCLUDED
#define SERIALIZATION_H_INCLUDED

#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {

class AcdfgSerializer {
public:
  Acdfg* read_acdfg();

private:
};


} // end fixrgraphiso namespace

#endif // SERIALIZATION_H_INCLUDED
