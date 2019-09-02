#pragma once

// Standard library includes.
#include <iostream>																										//< For std::cout, std::cerr. Remove when common files will not nead it.

// C3 inclusion.
#include "Common/MWR/C3/Sdk.hpp"																						//< C3 Sdk.
#include "Common/C3_BUILD_VERSION_HASH_PART.hxx"																		//< Build versioning header.

// CppCommons.
#include "Common/MWR/WinTools/Services.h"
#include "Common/MWR/CppTools/Payload.h"

using EmbeddedData = MWR::Payload<4096>;
