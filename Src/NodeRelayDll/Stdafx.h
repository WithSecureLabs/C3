#pragma once

#include <iostream>																										//< For std::cout, std::cerr. Remove when common files will not nead it.

// C3 inclusion.
#include "Common/MWR/C3/Sdk.hpp"																						//< C3 Sdk.

// CppCommons.
#include "Common/MWR/CppTools/Payload.h"

using EmbeddedData = MWR::Payload<4096>;
